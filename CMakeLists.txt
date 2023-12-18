cmake_minimum_required(VERSION 3.21)
project(bind-test)

set(INTERFACE_IP "10.1.0.100" CACHE STRING "Address of interface to bind to")
set(INTERFACE_NAME "enx0050b6eb5220" CACHE STRING "Name of interface to bind to")
set(MULTICAST_ADDR "224.2.127.254" CACHE STRING "Multicast IP to use")

find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads)

add_executable(bind-test)
target_sources(bind-test
  PRIVATE
    binding_functions.hpp
    binding_functions.cpp

    components.hpp
    components.cpp

    logging.hpp
    logging.cpp

    server_multicast.cpp
    client_multicast.cpp

    # server_unicast.cpp
    # client_unicast.cpp

    main.cpp
)
target_include_directories(bind-test
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
)
target_compile_definitions(bind-test
  PUBLIC
    -DINTERFACE_IP=\"${INTERFACE_IP}\"
    -DINTERFACE_NAME=\"${INTERFACE_NAME}\"
    -DMULTICAST_ADDR=\"${MULTICAST_ADDR}\"
)
target_compile_options(bind-test
  PRIVATE
    -Wall
)
target_link_libraries(bind-test
  PRIVATE
    Boost::system
    Threads::Threads
    $<$<PLATFORM_ID:QNX>:socket>
)
target_compile_features(bind-test PRIVATE cxx_std_17)

install(TARGETS bind-test)
