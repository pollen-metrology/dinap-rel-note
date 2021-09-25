//
// Created by bertrand on 25/09/2021.
//

#pragma once

#include <list>
#include <filesystem>

struct Item {
  std::string title;
  std::filesystem::path path;
};

struct Section {
  std::string title;
  std::list<Item> items;
};
