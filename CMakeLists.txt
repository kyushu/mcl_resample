project(MCL)

cmake_minimum_required(VERSION 3.5)

# add_definitions(-std=c++11)
add_compile_options(-std=c++11)

set(CXX_FLAGS "-Wall")

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(mcl 1_mcl.cpp)
add_executable(resampling 2_resampling.cpp)
add_executable(test_resampling_wheel test_resampling_wheel.cpp)
add_executable(resampling_wheel 3_resampling_wheel.cpp)
add_executable(resampling_evaluate 4_resampling_evaluate.cpp)

# add_executable(gnuplot_test gnuplot_test.cpp)