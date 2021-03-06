# -*- mode: cmake -*-
# vi: set ft=cmake :

cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(particles)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_CXX_FLAGS "-D_GLIBCXX_USE_CXX11_ABI=1")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread ")  # -Wshadow (too noisy)
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-deprecated-register -Wno-unused-function")
endif()
message("CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})

# list(APPEND CMAKE_PREFIX_PATH $ENV{DRAKE_BUILD_DIR})
# list(APPEND CMAKE_PREFIX_PATH $ENV{DRAKE_BUILD_DIR}/lib/cmake)
# list(APPEND CMAKE_MODULE_PATH "../../cmake/modules")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        find_package(Eigen3 3.3.5 QUIET CONFIG)
ENDIF()

find_package(GTest MODULE REQUIRED)
find_package(GFlags MODULE REQUIRED COMPONENTS shared)
find_package(drake CONFIG REQUIRED)
find_package(Torch REQUIRED)

#### PYTHON ENVIRONMENTS START ####

find_package( Python2 2.7.15 REQUIRED Interpreter Development)
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(PYTHON_LIBRARIES ${Python2_LIBRARIES} )
else()
        set(PYTHON_LIBRARIES "/usr/lib/x86_64-linux-gnu/libpython2.7.so")
endif()

set(PYTHON_INCLUDE_DIRS "${Python2_INCLUDE_DIRS}"       # kitchen sink
        "/usr/include/python2.7"
        "/usr/local/lib/python2.7/dist-packages/numpy/core/include"
        "${Python2_LIBRARY_DIRS}/python2.7/dist-packages/numpy/core/include"
        "${Python2_LIBRARY_DIRS}/python2.7/site-packages/numpy/core/include")

set(PYTHONPATH
"${drake_DIR}/../../python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages:$ENV{PYTHONPATH}")

message("Python2_LIBRARIES: " ${Python2_LIBRARIES})
#### PYTHON ENVIRONMENTS END ####

# Send over the SDF file so the particle example can be run "in place."
configure_file(models/particle.sdf "${CMAKE_CURRENT_BINARY_DIR}" COPYONLY)

add_library(particles particle.cc particle.h utilities.cc utilities.h)
target_link_libraries(particles drake::drake)

add_executable(uniformly_accelerated_particle uniformly_accelerated_particle.cc)
target_link_libraries(uniformly_accelerated_particle particles
  drake::drake drake::drake-common-text-logging-gflags gflags_shared
)

# TODO(EricCousineau-TRI, jamiesnape): Rewrite these tests to not use GTest.
# this currently fails on a missing header
# if(GTEST_FOUND)
#   add_executable(particle_test particle_test.cc)
#   target_include_directories(particle_test PRIVATE
#     "${CMAKE_CURRENT_SOURCE_DIR}"
#   )
#   target_link_libraries(particle_test
#     particles drake::drake GTest::GTest GTest::Main
#   )
#   add_test(NAME particle_test COMMAND particle_test)

#   add_executable(utilities_test utilities_test.cc)
#   target_include_directories(utilities_test PRIVATE
#     "${CMAKE_CURRENT_SOURCE_DIR}"
#   )
#   target_link_libraries(utilities_test
#     particles drake::drake GTest::GTest GTest::Main
#   )
#   add_test(NAME utilities_test COMMAND utilities_test)
# endif()
