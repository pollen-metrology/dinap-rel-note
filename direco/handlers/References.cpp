//
// Created by bertrand on 25/09/2021.
//

#include "References.h"
#include "Utils.h"

void References::Handle(std::string &str, unsigned int &offset) {
  auto refPos = str.find("$ref:", 0);
  while (refPos != std::string::npos) {
    auto endRefPos = str.find('$', refPos + 5);
    std::string refName = str.substr(refPos + 5, (endRefPos - refPos - 5));

    utils::Replace(str, "$ref:" + refName + "$", std::to_string(offset++));

    refPos = str.find("$ref:", endRefPos);
  }
}
