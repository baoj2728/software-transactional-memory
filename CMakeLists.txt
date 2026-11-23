cmake_minimum_required(VERSION 3.16)
project(software_transactional_memory LANGUAGES CXX)

find_package(Threads REQUIRED)

add_library(stm STATIC src/context.cpp)
target_compile_features(stm PUBLIC cxx_std_20)
target_include_directories(stm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(stm PUBLIC Threads::Threads)
