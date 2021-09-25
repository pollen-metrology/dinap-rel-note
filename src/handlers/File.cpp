//
// Created by bertrand on 25/09/2021.
//

#include "File.h"
#include "Utils.h"

// logging
#include <plog/Log.h>

// C++
#include <iostream>
#include <fstream>

void File::Handle(std::string &str, const std::filesystem::path &root) {
  auto refPos = str.find("{{file:", 0);
  while (refPos != std::string::npos) {
    auto endRefPos = str.find("}}", refPos + 7);
    std::string refName = str.substr(refPos + 7, (endRefPos - refPos - 7));

    std::filesystem::path filePath = root / refName;

    if (!std::filesystem::exists(filePath)) {
      LOG_ERROR << "File not found: " << filePath.string();
      refPos = str.find("{{file:", endRefPos);
      continue;
    }

    std::fstream fs;
    fs.open(filePath, std::fstream::in);
    std::stringstream ssi;
    ssi << fs.rdbuf();
    fs.close();

    utils::Replace(str, "{{file:" + refName + "}}", ssi.str());

    refPos = str.find("{{file:", endRefPos);
  }
}
