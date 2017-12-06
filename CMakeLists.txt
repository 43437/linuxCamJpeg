cmake_minimum_required(VERSION 2.6)
project(linuxcamjpeg)


include_directories("/home/acmax/app/ffmpeg-dev/include/")

link_directories("/home/acmax/app/ffmpeg-dev/lib/")

add_executable(linuxcamjpeg main.cpp)

target_link_libraries(linuxcamjpeg "avformat" "avcodec" "avutil" "swresample" "swscale" "avdevice")