cmake_minimum_required(VERSION 2.6)
project(linuxcamjpeg)

add_executable(linuxcamjpeg main.cpp)

install(TARGETS linuxcamjpeg RUNTIME DESTINATION bin)
