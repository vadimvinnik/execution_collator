cmake_minimum_required(VERSION 3.8)

project(demo)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ../include) 

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(demo Threads::Threads)

