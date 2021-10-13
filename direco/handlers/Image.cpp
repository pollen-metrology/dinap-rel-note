//
// Created by bertrand on 25/09/2021.
//

#include "Image.h"
#include "Utils.h"

// Base64 encode
#include <base64.h>

// logging
#include <plog/Log.h>

// C++
#include <iostream>
#include <fstream>

void Image::Handle(std::string &str, const std::filesystem::path& root) {
  auto refPos = str.find(R"(<img src=")", 0);
  while (refPos != std::string::npos) {
    auto endRefPos = str.find('"', refPos + 10);
    std::string imgUri = str.substr(refPos + 10, (endRefPos - refPos - 10));
    if (imgUri.starts_with("data:")) {
      refPos = str.find(R"(<img src=")", endRefPos);
      continue;
    }

    std::filesystem::path imPath = root / imgUri;

    if (!std::filesystem::exists(imPath)) {
      LOG_ERROR << "Image not found: " << imPath.string() << std::endl;
      refPos = str.find(R"(<img src=")", endRefPos);
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
      LOG_ERROR << "Failed to encode image" << std::endl;
      refPos = str.find(R"(<img src=")", endRefPos);
      continue;
    }

    LOG_INFO << "Using image " << imPath.string();
    utils::Replace(str, imgUri, "data:image/jpeg;base64," + encoded);

    refPos = str.find(R"(<img src=")", endRefPos);
  }
}
