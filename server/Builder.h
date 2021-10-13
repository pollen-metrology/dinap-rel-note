//
// Created by bertrand on 25/09/2021.
//

#pragma once

#include <filesystem>
#include <memory>
#include <functional>

struct BuilderImpl;

class Builder {
public:
  Builder(std::filesystem::path config, std::filesystem::path root, std::filesystem::path output);

  virtual ~Builder();

  void Run();

  void Watch();

  void InitLogger();

  void OnBuild(std::function<void(const std::filesystem::path&)> cb);

private:
  std::shared_ptr<BuilderImpl> mImpl;
};



