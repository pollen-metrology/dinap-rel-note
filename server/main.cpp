#include "Server.h"

// cxxopts
#include <cxxopts.hpp>

// yaml parser
#include <yaml-cpp/yaml.h>

// c++
#include <filesystem>

int main(int argc, char *argv[]) {
  cxxopts::Options options("DinapRelNotes", "A simple release note generator");

  options.add_options()
      ("c,config", "Server config file", cxxopts::value<std::string>()->default_value("./config.yml"))
      ("s,silent", "Hide banner")
      ("b,build-only", "Build only (do not open http server)")
      ("h,help", "Print usage");

  const auto result = options.parse(argc, argv);

  if (result.count("help")) {
    std::cout << options.help() << std::endl;
    return 1;
  }

  if (!result.count("silent")) {
    std::cout << R"(

██████╗ ██╗   ██████╗ ███████╗   ███████╗███████╗██████╗ ██╗   ██╗
██╔══██╗██║   ██╔══██╗██╔════╝   ██╔════╝██╔════╝██╔══██╗██║   ██║
██║  ██║██║   ██████╔╝█████╗     ███████╗█████╗  ██████╔╝██║   ██║
██║  ██║██║   ██╔══██╗██╔══╝     ╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝
██████╔╝██║██╗██║  ██║███████╗██╗███████║███████╗██║  ██║ ╚████╔╝
╚═════╝ ╚═╝╚═╝╚═╝  ╚═╝╚══════╝╚═╝╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝

   Dinap Release Compiler Server v1.0.0 By Bertrand Darbon

)";
  }
  if (result.count("build-only") == 1) {
    std::cout << "Mode Build-Only" << std::endl;
  }

  Server server(result["config"].as<std::string>());

  server.Run(result.count("build-only") == 1);

  return 0;
}
