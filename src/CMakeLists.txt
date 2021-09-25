add_executable(dinapRelNotes
        main.cpp
        Generator.cpp
        handlers/File.cpp
        handlers/Variable.cpp
        handlers/Image.cpp
        handlers/Utils.cpp
        handlers/References.cpp
        handlers/ContentTable.cpp)
target_link_libraries(dinapRelNotes PRIVATE maddy yaml-cpp fmt base64 cxxopts::cxxopts)
target_compile_features(dinapRelNotes PRIVATE cxx_std_20)
