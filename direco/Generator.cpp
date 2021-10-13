//
// Created by bertrand on 25/09/2021.
//

#include "Generator.h"
#include "handlers/File.h"
#include "handlers/Image.h"
#include "handlers/Variable.h"
#include "handlers/References.h"
#include "handlers/ContentTable.h"
#include "Section.h"

// Markdown parser
#include <maddy/parser.h>

// yaml parser
#include <yaml-cpp/yaml.h>

// logging
#include <plog/Log.h>

// C++
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <fmt/format.h>

struct GeneratorImpl {
  std::unordered_map<std::string, std::string> vars;
  std::vector<Section> sections;
  std::filesystem::path root;
  std::filesystem::path header;
  std::filesystem::path footer;
  std::filesystem::path outputFile = "output.html";
  bool contentTable = true;

  std::list<std::filesystem::path> usedFiles;
};


Generator::Generator(const std::filesystem::path &root) {
  if (!std::filesystem::exists(root)) {
    throw std::runtime_error("Root folder does not exists: " + root.string());
  }
  if (!std::filesystem::is_directory(root)) {
    throw std::runtime_error("Given root is not a directory: " + root.string());
  }
  mImpl = std::make_shared<GeneratorImpl>();
  mImpl->root = root;
}

void Generator::LoadConfig(const std::filesystem::path &configFile) {
  if (!std::filesystem::exists(configFile)) {
    throw std::runtime_error("Config file not found: " + configFile.string());
  }

  LOG_INFO << "Preparing release note from " << configFile.generic_string();

  YAML::Node config = YAML::LoadFile(configFile.generic_string());
  mImpl->usedFiles.push_back(configFile);

  if (config["content-table"].IsDefined()
      && (
          config["content-table"].as<std::string>() == "off" ||
          config["content-table"].as<std::string>() == "OFF" ||
          config["content-table"].as<std::string>() == "false" ||
          config["content-table"].as<std::string>() == "FALSE"
      )) {
    mImpl->contentTable = false;
  }

  if (config["output"].IsDefined()) {
    mImpl->outputFile = config["output"].as<std::string>();
    if (std::filesystem::exists(mImpl->root / mImpl->outputFile)) {
      LOG_WARNING << "Output file already exists";
    }
  }

  for (const auto &item: config["vars"]) {
    auto key = item.first.as<std::string>();
    auto value = item.second.as<std::string>();
    LOG_INFO << "\t" << key << "=" << value;
    mImpl->vars[std::move(key)] = std::move(value);
  }

  mImpl->header = mImpl->root / config["templates"]["header"].as<std::string>();
  mImpl->footer = mImpl->root / config["templates"]["footer"].as<std::string>();

  if (!std::filesystem::exists(mImpl->header)) {
    throw std::runtime_error("Header template not found");
  }
  if (!std::filesystem::exists(mImpl->footer)) {
    throw std::runtime_error("Footer template not found");
  }

  const auto sections = config["sections"];
  for (const auto section: sections) {
    auto title = section["title"].as<std::string>();

    const auto items = section["items"];
    Section sect{
        .title = std::move(title)
    };
    for (const auto &item: items) {
      auto itemTitle = item["title"].as<std::string>("");
      auto itemFile = item["file"].as<std::string>("");
      if (!itemFile.empty() && !std::filesystem::exists(mImpl->root / itemFile)) {
        LOG_ERROR << "Error: " << mImpl->root / itemFile << " not found";
        itemFile = "";
      }

      sect.items.emplace_back(Item{
          .title = std::move(itemTitle),
          .path = itemFile.empty() ? std::nullopt : std::make_optional(mImpl->root / itemFile)
      });
    }
    mImpl->sections.emplace_back(std::move(sect));
  }
}

void Generator::Build(const std::filesystem::path &outDir) {
  if (!std::filesystem::exists(outDir)) {
    LOG_INFO << "Creating missing output directory " << outDir.string();
    std::filesystem::create_directories(outDir);
  }
  if (!std::filesystem::is_directory(outDir)) {
    throw std::runtime_error("Output directory is not a valid directory");
  }

  LOG_INFO << "Building to " << (outDir / mImpl->outputFile).string();
  std::fstream fso, fs;
  std::filesystem::create_directories(outDir/mImpl->outputFile.parent_path());
  fso.open(outDir / mImpl->outputFile, std::fstream::out);

  {
    fs.open(mImpl->header, std::fstream::in);
    mImpl->usedFiles.push_back(mImpl->header);
    std::stringstream ss;
    ss << fs.rdbuf();
    fs.close();
    std::string str = ss.str();
    Variable::Handle(str, mImpl->vars);
    Image::Handle(str, mImpl->root);
    File::Handle(str, mImpl->root);
    if (mImpl->contentTable)
      ContentTable::Build(str, mImpl->sections);
    fso << str;
  }

  unsigned int offset = 1;
  std::shared_ptr<maddy::Parser> parser = std::make_shared<maddy::Parser>();
  for (const auto &section: mImpl->sections) {
    if (!section.title.empty())
      fso << fmt::format(R"(<h1 id="{}" class="sectionId">{}</h1>)", section.title, section.title) << std::endl;
    for (const auto &doc: section.items) {
      if (!doc.title.empty())
        fso << fmt::format(R"(<h2 id="{}" class="docId">{}</h2>)", doc.title, doc.title) << std::endl;
      if (doc.path) {
        fs.open(doc.path.value(), std::fstream::in);
        mImpl->usedFiles.push_back(doc.path.value());
        std::stringstream ss;
        ss << fs.rdbuf();
        std::string str = ss.str();
        Variable::Handle(str, mImpl->vars);
        References::Handle(str, offset);
        if (doc.path->extension() == ".md") {
          ss.str(str);
          LOG_DEBUG << "Parsing Markdown file " << doc.path->string();
          str = parser->Parse(ss);
        }
        Image::Handle(str, doc.path->parent_path());
        File::Handle(str, mImpl->root);
        fs.close();
        fso << str;
      }
    }
  }

  fs.open(mImpl->footer, std::fstream::in);
  mImpl->usedFiles.push_back(mImpl->footer);
  fso << fs.rdbuf();
  fs.close();

  fso.close();
}

std::list<std::filesystem::path> Generator::GetUsedFiles() {
  return mImpl->usedFiles;
}

std::filesystem::path Generator::GetOutputFile() {
  return mImpl->outputFile;
}
