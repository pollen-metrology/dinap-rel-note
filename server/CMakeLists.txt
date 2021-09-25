add_executable(direserv main.cpp Server.cpp Url.cpp Builder.cpp WebsocketCtrl.cpp)
target_link_libraries(direserv PRIVATE diregen yaml-cpp cxxopts::cxxopts drogon libfswatch plog)
target_compile_features(direserv PRIVATE cxx_std_20)

install(TARGETS direserv DESTINATION bin)
