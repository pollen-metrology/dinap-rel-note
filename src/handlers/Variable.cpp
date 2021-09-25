//
// Created by bertrand on 25/09/2021.
//

#include "Variable.h"
#include "Utils.h"

void Variable::Handle(std::string &str, const std::unordered_map<std::string, std::string> &vars) {
  for (const auto &var: vars) {
    utils::Replace(str, "{{" + var.first + "}}", var.second);
  }
}
