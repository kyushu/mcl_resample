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


find_package(OpenCV 3.0.0 REQUIRED)
if(OpenCV_FOUND)
    message(STATUS "OpenCV library status:")
    message(STATUS "    version: ${OpenCV_VERSION}")
    message(STATUS "    libraries: ${OpenCV_LIBS}")
    message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

    INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})

    add_executable(resampling_evaluate 4_resampling_evaluate.cpp)
    target_link_libraries(resampling_evaluate ${OpenCV_LIBS})
else()
    message("OpenCV not found, so we won't build the webcam_face_pose_ex example.")
endif()