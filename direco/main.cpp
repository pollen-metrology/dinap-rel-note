#include "Generator.h"

// cxxopts
#include <cxxopts.hpp>

// plog
#include <plog/Log.h>
#include "plog/Initializers/RollingFileInitializer.h"
#include "plog/Appenders/ConsoleAppender.h"

int main(int argc, char *argv[]) {
  cxxopts::Options options("DinapRelNotes", "A simple release note generator");

  options.add_options()
      ("r,root", "Root for files & references", cxxopts::value<std::string>()->default_value("."))
      ("c,config", "Config file", cxxopts::value<std::string>()->default_value("./config.yml"))
      ("o,out", "Output directory for html files", cxxopts::value<std::string>()->default_value("./output"))
      ("s,silent", "Hide banner")
      ("h,help", "Print usage");

  const auto result = options.parse(argc, argv);

  if (result.count("help")) {
    std::cout << options.help() << std::endl;
    return 1;
  }

  if (!result.count("silent")) {
    std::cout << R"(

██████╗ ██╗   ██████╗ ███████╗    ██████╗ ██████╗
██╔══██╗██║   ██╔══██╗██╔════╝   ██╔════╝██╔═══██╗
██║  ██║██║   ██████╔╝█████╗     ██║     ██║   ██║
██║  ██║██║   ██╔══██╗██╔══╝     ██║     ██║   ██║
██████╔╝██║██╗██║  ██║███████╗██╗╚██████╗╚██████╔╝
╚═════╝ ╚═╝╚═╝╚═╝  ╚═╝╚══════╝╚═╝ ╚═════╝ ╚═════╝

   Dinap Release Compiler v1.0.0 By Bertrand Darbon

)";
  }

  static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
  plog::init(plog::debug, &consoleAppender);

  try {
    Generator generator(result["root"].as<std::string>());
    generator.LoadConfig(result["config"].as<std::string>());
    generator.Build(result["out"].as<std::string>());
  } catch (const std::exception &e) {
    LOG_ERROR << e.what();
    return 1;
  }

  return 0;
}
