//
// Created by bertrand on 25/09/2021.
//

#pragma once

#include <memory>
#include <filesystem>

struct ServerImpl;

class Server {
public:
  Server() = delete;
  explicit Server(const std::filesystem::path& configFile);

  void Run();

private:
  std::shared_ptr <ServerImpl> mImpl;
};



