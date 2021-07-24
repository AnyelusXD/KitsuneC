cmake_minimum_required(VERSION 3.16)
project(Kitsune_cpp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

add_executable(Kitsune_cpp main.cpp source/utils.cpp include/utils.h source/netStat.cpp include/netStat.h source/featureExtractor.cpp include/featureExtractor.h source/neuralnet.cpp include/neuralnet.h source/kitNET.cpp include/kitNET.h include/cluster.h source/cluster.cpp test/testDense.cpp test/kitsuneExample.cpp test/test.h)