FROM ubuntu
WORKDIR /usr/local/bin/dinap/
RUN apt-get update
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get install -y git cmake gcc python3-pip uuid-dev --fix-missing 
RUN python3 -m pip install conan
#RUN git clone --recurse-submodules https://github.com/BertrandD/dinap-rel-note.git
RUN git clone --recurse-submodules https://github.com/pollen-metrology/dinap-rel-note.git
RUN cd dinap-rel-note && mkdir cmake-build && cd cmake-build && conan install ../external/drogon/conanfile.txt -s compiler.libcxx=libstdc++11 --build=missing && cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_paths.cmake && cmake --build . --target install -- -j8 && cd .. && rm -rf cmake-build
RUN mkdir /direserv
COPY config.yml /direserv
WORKDIR /direserv
ENTRYPOINT ["direserv"]