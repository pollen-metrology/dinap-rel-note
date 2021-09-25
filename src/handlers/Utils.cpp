//
// Created by bertrand on 25/09/2021.
//

#include "Utils.h"

void utils::Replace(std::string &str, const std::string &token, const std::string &value) {
  auto tokenPos = str.find(token);
  while (tokenPos != std::string::npos) {
    str.replace(tokenPos, token.length(), value);
    tokenPos = str.find(token);
  }
}