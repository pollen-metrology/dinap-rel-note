//
// Created by bertrand on 25/09/2021.
//

#pragma once

#include <filesystem>
#include <any>
#include <memory>
#include <list>

struct GeneratorImpl;

class Generator {
public:
  explicit Generator(const std::filesystem::path &root);

  void LoadConfig(const std::filesystem::path &configFile);

  void Build(const std::filesystem::path &outDir);

  std::list<std::filesystem::path> GetUsedFiles();

  std::filesystem::path GetOutputFile();

private:
  std::shared_ptr<GeneratorImpl> mImpl;
};



