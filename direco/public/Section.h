//
// Created by bertrand on 25/09/2021.
//

#pragma once

#include <list>
#include <filesystem>
#include <optional>

struct Item {
  std::string title;
  std::optional<std::filesystem::path> path;
};

struct Section {
  std::string title;
  std::list<Item> items;
};
