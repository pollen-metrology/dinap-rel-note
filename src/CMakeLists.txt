add_executable(dinapRelNotes main.cpp Generator.cpp Generator.h)
target_link_libraries(dinapRelNotes PRIVATE maddy yaml-cpp fmt base64 cxxopts::cxxopts)
target_compile_features(dinapRelNotes PRIVATE cxx_std_20)
