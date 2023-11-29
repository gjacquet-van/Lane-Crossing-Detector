cmake_minimum_required(VERSION 2.8)
project( ImageProc )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(bin/LaneChangeDetector  src/LaneChangeDetector.cpp)
target_link_libraries(bin/LaneChangeDetector ${OpenCV_LIBS} )








