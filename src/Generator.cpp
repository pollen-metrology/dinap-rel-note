//
// Created by bertrand on 25/09/2021.
//

#include "Generator.h"

// Markdown parser
#include <maddy/parser.h>

#include <yaml-cpp/yaml.h>
#include <base64.h>

// C++
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <fmt/format.h>

struct Item {
  std::string title;
  std::filesystem::path path;
};
struct Section {
  std::string title;
  std::list<Item> items;
};

struct GeneratorImpl {
  std::unordered_map<std::string, std::string> vars;
  std::vector<Section> sections;
  std::filesystem::path root;
  std::filesystem::path header;
  std::filesystem::path footer;
  std::filesystem::path outputFile = "output.html";
  bool contentTable = true;

  std::stringstream BuildContentTable();

  std::stringstream HandleImages(std::stringstream ss) const;

  void HandleImages(std::string &stri) const;

  void HandleFiles(std::string &stri) const;

  std::stringstream HandleFiles(std::stringstream ss) const;
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
    throw std::runtime_error("Config file not found");
  }

  std::cout << "Preparing release note from " << configFile.generic_string() << std::endl;

  YAML::Node config = YAML::LoadFile(configFile.generic_string());

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
      std::cout << "WARN: output file already exists" << std::endl;
    }
  }

  for (const auto &item: config["vars"]) {
    auto key = item.first.as<std::string>();
    auto value = item.second.as<std::string>();
    std::cout << "\t" << key << "=" << value << std::endl;
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
      auto itemTitle = item["title"].as<std::string>();
      const auto itemFile = item["file"].as<std::string>();
      if (!std::filesystem::exists(mImpl->root / itemFile)) {
        std::cerr << "Error: " << mImpl->root / itemFile << " not found" << std::endl;
        continue;
      }

      std::cout << "Adding item " << item << " to release note" << std::endl;
      sect.items.emplace_back(Item{
          .title = std::move(itemTitle),
          .path = mImpl->root / itemFile
      });
    }
    mImpl->sections.emplace_back(std::move(sect));
  }
}

void Replace(std::string &str, const std::string &token, const std::string &value) {
  auto tokenPos = str.find(token);
  while (tokenPos != std::string::npos) {
    str.replace(tokenPos, token.length(), value);
    tokenPos = str.find(token);
  }
}

std::stringstream Replace(std::fstream &fs, const std::unordered_map<std::string, std::string> &replace) {
  std::stringstream ss;
  ss << fs.rdbuf();
  std::string s = ss.str();
  for (const auto &item: replace) {
    Replace(s, "{{" + item.first + "}}", item.second);
  }
  ss.str(s);
  return ss;
}

void HandleRefs(std::stringstream &ss, unsigned int &offset) {
  std::string str = ss.str();
  auto refPos = str.find("$ref:", 0);
  while (refPos != std::string::npos) {
    auto endRefPos = str.find('$', refPos + 5);
    std::string refName = str.substr(refPos + 5, (endRefPos - refPos - 5));

    Replace(str, "$ref:" + refName + "$", std::to_string(offset++));

    refPos = str.find("$ref:", endRefPos);
  }

  ss.str(str);
}

void GeneratorImpl::HandleImages(std::string &stri) const {
  auto refPos = stri.find("{{img:", 0);
  while (refPos != std::string::npos) {
    auto endRefPos = stri.find("}}", refPos + 6);
    std::string refName = stri.substr(refPos + 6, (endRefPos - refPos - 6));

    std::filesystem::path imPath = root / refName;

    if (!std::filesystem::exists(imPath)) {
      std::cerr << "Image not found: " << imPath.string() << std::endl;
      refPos = stri.find("{{img:", endRefPos);
      continue;
    }

    std::fstream fs;
    fs.open(imPath, std::fstream::in);
    std::stringstream ssi;
    ssi << fs.rdbuf();
    fs.close();
    std::string input = ssi.str();
    std::string encoded;
    if (!Base64::Encode(input, &encoded)) {
      std::cerr << "Failed to encode image" << std::endl;
      refPos = stri.find("{{img:", endRefPos);
      continue;
    }

    Replace(stri, "{{img:" + refName + "}}", "data:image/jpeg;base64," + encoded);

    refPos = stri.find("{{img:", endRefPos);
  }
}

void GeneratorImpl::HandleFiles(std::string &stri) const {
  auto refPos = stri.find("{{file:", 0);
  while (refPos != std::string::npos) {
    auto endRefPos = stri.find("}}", refPos + 7);
    std::string refName = stri.substr(refPos + 7, (endRefPos - refPos - 7));

    std::filesystem::path filePath = root / refName;

    if (!std::filesystem::exists(filePath)) {
      std::cerr << "File not found: " << filePath.string() << std::endl;
      refPos = stri.find("{{file:", endRefPos);
      continue;
    }

    std::fstream fs;
    fs.open(filePath, std::fstream::in);
    std::stringstream ssi;
    ssi << fs.rdbuf();
    fs.close();

    Replace(stri, "{{file:" + refName + "}}", ssi.str());

    refPos = stri.find("{{file:", endRefPos);
  }
}

std::stringstream GeneratorImpl::HandleImages(std::stringstream ss) const {
  std::string str = ss.str();
  HandleImages(str);
  ss.str(str);
  return ss;
}

std::stringstream GeneratorImpl::HandleFiles(std::stringstream ss) const {
  std::string str = ss.str();
  HandleFiles(str);
  ss.str(str);
  return ss;
}

std::stringstream GeneratorImpl::BuildContentTable() {
  std::stringstream ss;
  ss << R"(<div class="tableOfContent">)";
  for (const auto &section: sections) {
    ss << fmt::format(R"(<a href="#{}" class="sectionTitle">{}</a>)", section.title, section.title);
    for (const auto &doc: section.items) {
      ss << fmt::format(R"(<a href="#{}" class="docTitle">{}</a>)", doc.title, doc.title);
    }
  }
  ss << R"(</div>)";
  return ss;
}

void Generator::Build(const std::filesystem::path &outDir) {
  if (!std::filesystem::exists(outDir)) {
    std::cout << "INFO: Creating directory " << outDir.string() << std::endl;
    std::filesystem::create_directories(outDir);
  }
  if (!std::filesystem::is_directory(outDir)) {
    throw std::runtime_error("Output directory is not a valid directory");
  }

  std::cout << "Building to " << (outDir / mImpl->outputFile).string() << std::endl;
  std::fstream fso, fs;
  fso.open(outDir / mImpl->outputFile, std::fstream::out);

  fs.open(mImpl->header, std::fstream::in);
  fso << mImpl->HandleFiles(mImpl->HandleImages(Replace(fs, mImpl->vars))).rdbuf();
  if (mImpl->contentTable)
    fso << mImpl->BuildContentTable().rdbuf();
  fs.close();

  unsigned int offset = 1;
  std::shared_ptr<maddy::Parser> parser = std::make_shared<maddy::Parser>(config);
  for (const auto &section: mImpl->sections) {
    fso << fmt::format(R"(<h1 id="{}" class="sectionId">{}</h1>)", section.title, section.title) << std::endl;
    for (const auto &doc: section.items) {
      fso << fmt::format(R"(<h2 id="{}" class="docId">{}</h2>)", doc.title, doc.title) << std::endl;
      fs.open(doc.path, std::fstream::in);
      auto ss = Replace(fs, mImpl->vars);
      HandleRefs(ss, offset);
      std::string html;
      if (doc.path.extension() == ".md") {
        html = parser->Parse(ss);
      } else {
        html = ss.str();
      }
      mImpl->HandleImages(html);
      mImpl->HandleFiles(html);
      fs.close();
      fso << html;
    }
  }

  fs.open(mImpl->footer, std::fstream::in);
  fso << fs.rdbuf();
  fs.close();

  fso.close();
}
