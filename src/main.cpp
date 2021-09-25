#include "Generator.h"

// cxxopts
#include <cxxopts.hpp>

int main(int argc, char *argv[]) {
  cxxopts::Options options("DinapRelNotes", "A simple release note generator");

  options.add_options()
      ("r,root", "Root for files & references", cxxopts::value<std::string>()->default_value("."))
      ("c,config", "Config file", cxxopts::value<std::string>()->default_value("./config.yml"))
      ("o,out", "Output directory for html files", cxxopts::value<std::string>()->default_value("./output"))
      ("h,help", "Print usage");

  const auto result = options.parse(argc, argv);

  Generator generator(result["root"].as<std::string>());
  generator.LoadConfig(result["config"].as<std::string>());
  generator.Build(result["out"].as<std::string>());

  return 0;
}
