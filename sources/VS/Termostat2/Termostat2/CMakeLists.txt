# CMakeList.txt : CMake project for Termostat2, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (Termostat2 "Termostat2.cpp" "Termostat2.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Termostat2 PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
