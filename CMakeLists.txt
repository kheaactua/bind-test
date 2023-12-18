 cmake_minimum_required(VERSION 3.23)
project(bind-test)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

add_executable(bind-test)
target_sources(bind-test PRIVATE main.cpp)
target_compile_definitions(bind-test
  PUBLIC
    -DINTERFACE_IP=192.168.2.53
    -DMULTICAST_ADDR=224.2.127.254
    -DINTERFACE_NAME="eno1"
)
target_compile_features(bind-test PRIVATE cxx_std_17)
