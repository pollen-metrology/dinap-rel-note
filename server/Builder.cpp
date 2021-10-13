//
// Created by bertrand on 25/09/2021.
//

#include "Builder.h"
#include "Generator.h"

// fswatch
#include <libfswatch/c++/monitor.hpp>
#include <libfswatch/c++/monitor_factory.hpp>

// logging
#include <plog/Log.h>
#include "plog/Initializers/RollingFileInitializer.h"
#include "plog/Appenders/ConsoleAppender.h"

// c++
#include <thread>
#include <algorithm>
#include <list>

namespace fs = std::filesystem;

struct BuilderImpl {
  fs::path config;
  fs::path root;
  fs::path output;
  std::thread thread;
  fsw::monitor *monitor = nullptr;
  std::map<std::string, std::list<fs::path>> concernedConfigs;
  std::list<std::function<void(fs::path)>> cb;
};

Builder::Builder(fs::path config, fs::path root, fs::path output) :
    mImpl(std::make_shared<BuilderImpl>(BuilderImpl{
        .config = std::move(config),
        .root = std::move(root),
        .output = std::move(output)
    })) {

}

void Builder::InitLogger() {
  static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
  plog::init(plog::debug, &consoleAppender);
}


void Builder::Run() {
  for (const auto &releaseConfigFile: fs::recursive_directory_iterator(mImpl->config)) {
    if (releaseConfigFile.path().extension() == ".yml") {
      LOG_INFO << "Compiling" << releaseConfigFile.path();
      Generator generator(mImpl->root);
      generator.LoadConfig(releaseConfigFile.path());
      generator.Build(mImpl->output);
      auto files = generator.GetUsedFiles();
      for (const auto &file: files) {
        mImpl->concernedConfigs[file].emplace_back(releaseConfigFile.path());
        mImpl->concernedConfigs[file].sort();
        mImpl->concernedConfigs[file].unique();
      }
    }
  }
}

void Builder::Watch() {
  auto process_events = [](const std::vector<fsw::event> &events, void *ctx) -> void {
    auto impl = static_cast<BuilderImpl*>(ctx);
    std::list<std::string> files;
    for (const auto &event: events) {
      files.push_back(event.get_path());
    }
    files.sort();
    files.unique();
    for (const auto &file: files) {
      if (impl->concernedConfigs.contains(file)) {
        for (const auto &configFile: impl->concernedConfigs.at(file)) {
          Generator generator(impl->root);
          generator.LoadConfig(configFile);
          generator.Build(impl->output);
          for (const auto &cb: impl->cb) {
            cb(generator.GetOutputFile());
          }
          auto usedFiles = generator.GetUsedFiles();
          for (const auto &usedFile: usedFiles) {
            impl->concernedConfigs[usedFile].emplace_back(configFile);
            impl->concernedConfigs[usedFile].sort();
            impl->concernedConfigs[usedFile].unique();
          }
        }
      }
    }
  };


  std::vector<std::string> paths{{mImpl->root.string()}};
  // Create the default platform monitor
  mImpl->monitor =
      fsw::monitor_factory::create_monitor(fsw_monitor_type::system_default_monitor_type,
                                           paths,
                                           process_events,
                                           mImpl.get());
  mImpl->monitor->set_recursive(true);
  mImpl->monitor->set_latency(0.2);
  mImpl->monitor->set_event_type_filters(
      {
          {Created},
          {Updated},
      });

  mImpl->thread = std::thread([this]() {
    mImpl->monitor->start();
  });
}

Builder::~Builder() {
  if (mImpl->monitor)
  {
    mImpl->monitor->stop();
    mImpl->thread.join();
  }
}

void Builder::OnBuild(std::function<void(const fs::path &)> cb) {
  mImpl->cb.emplace_back(std::move(cb));
}
