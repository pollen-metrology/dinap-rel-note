add_library(diregen STATIC
        Generator.cpp
        handlers/File.cpp
        handlers/Variable.cpp
        handlers/Image.cpp
        handlers/Utils.cpp
        handlers/References.cpp
        handlers/ContentTable.cpp)

target_include_directories(diregen PUBLIC ./public)
target_link_libraries(diregen PRIVATE maddy yaml-cpp fmt base64 plog)

add_executable(direco main.cpp)
target_link_libraries(direco PRIVATE cxxopts::cxxopts plog diregen)
target_compile_features(direco PRIVATE cxx_std_20)

install(TARGETS direco DESTINATION bin)
