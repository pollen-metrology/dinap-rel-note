//
// Created by bertrand on 25/09/2021.
//

#include "Server.h"
#include "Builder.h"
#include "Generator.h"
#include "WebsocketCtrl.h"
#include "Url.h"

// Yaml parser
#include <yaml-cpp/yaml.h>

// drogon
#include <drogon/drogon.h>

//c++
#include <filesystem>
#include <fstream>

using namespace drogon;
namespace fs = std::filesystem;

struct ServerImpl {
  YAML::Node config;
};

Server::Server(const fs::path &configFile) :
    mImpl(std::make_shared<ServerImpl>(ServerImpl{
        .config = YAML::LoadFile(configFile.string())
    })) {

}

fs::path CheckPath(const std::string &path) {
  if (!fs::exists(path)) {
    LOG_ERROR << "Directory " << path << " not found";
    throw std::runtime_error("Directory not found");
  }
  return path;
}

void Server::Run() {
  fs::path configDir = CheckPath(mImpl->config["configs"].as<std::string>());
  fs::path rootDir = CheckPath(mImpl->config["root"].as<std::string>());
  fs::path outputDir = CheckPath(mImpl->config["output"].as<std::string>());

  Builder builder(configDir, rootDir, outputDir);
  builder.InitLogger();
  builder.Run();

  const auto ip = mImpl->config["ip"].as<std::string>("127.0.0.1");
  const auto port = mImpl->config["port"].as<int>(80);
  const auto serverUrl = "http://" + ip + (port == 80 ? "" : ":" + std::to_string(port));

  LOG_INFO << "Starting server on " << serverUrl;

  for (const auto &item: fs::directory_iterator(outputDir)) {
    LOG_INFO << "Release note available on " << serverUrl << "/" << url::encode(item.path().filename());
  }

  auto wsCtrl = std::make_shared<WebsocketCtrl>(builder);

  app()
      .setDocumentRoot(outputDir)
      .setLogLevel(trantor::Logger::kWarn)
      .registerController(wsCtrl)
      .registerHandler("/{}", [&outputDir](const HttpRequestPtr& request, std::function<void(const HttpResponsePtr&)>&& callback) {
        std::string file = request->path();
        file.erase(file.begin());
        std::filesystem::path filePath = outputDir / file;

        if (!std::filesystem::exists(filePath)) {
          LOG_ERROR << "File not found: " << filePath.string();
          auto resp = HttpResponse::newHttpResponse();
          resp->setStatusCode(drogon::HttpStatusCode::k404NotFound);
          callback(resp);
          return;
        }
        std::fstream fs;
        fs.open(filePath, std::fstream::in);
        std::stringstream ssi;
        ssi << fs.rdbuf();
        fs.close();
        ssi << WebsocketCtrl::websocket;
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k200OK);
        resp->setBody(ssi.str());
        callback(resp);
      })
      .addListener(mImpl->config["ip"].as<std::string>(), mImpl->config["port"].as<int>(80))
      .setThreadNum(16)
      .run();
}
