add_subdirectory(maddy)
add_subdirectory(yaml-cpp)
add_subdirectory(fmt)
add_subdirectory(cxxopts)
add_subdirectory(plog)

add_library(base64 INTERFACE)
target_include_directories(base64 INTERFACE base64/)