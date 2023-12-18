 cmake_minimum_required(VERSION 3.23)
project(bind-test)

set(CLANG_TIDY_CHECKS)
list(APPEND
  "modernize-*"
  "readability-delete-null-pointer"
  "readability-duplicate-include"
  "readability-convert-member-functions-to-static"
  "readability-implicit-bool-conversion"
  "readability-make-member-function-const"
  "readability-misleading-indentation"
  "readability-non-const-parameter"
  "readability-qualified-auto"
  "readability-redundant-control-flow"
  "readability-simplify-boolean-expr"
  "readability-redundant-preprocessor"
  "readability-string-compare"
  "readability-static-accessed-through-instance"
  "readability-const-return-type"
  "readability-container-contains"
  "readability-container-data-pointer"
  "readability-container-size-empty"
  "readability-avoid-const-params-in-decls"
  "performance-unnecessary-copy-initialization"
  "performance-unnecessary-value-param"
  "performance-move-const-arg"
  "performance-for-range-copy"
  "performance-trivially-destructible"
  "performance-inefficient-string-concatenation"
  "performance-inefficient-vector-operation"
  "performance-implicit-conversion-in-loop"
  "performance-faster-string-find"
  "misc-new-delete-overloads"
  "misc-misplaced-const"
  "misc-misleading-identifier"
  "misc-throw-by-value-catch-by-reference"
)
list(JOIN CMAKE_TIDY_CHECKS "," CMAKE_TIDY_CHECKS)
set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks='${CMAKE_TIDY_CHECKS}")

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
)
target_compile_features(bind-test PRIVATE cxx_std_20)
