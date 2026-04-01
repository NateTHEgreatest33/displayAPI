cmake_minimum_required(VERSION 3.13)
project(displayAPI C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

add_library( displayAPI
  displayAPI.cpp
  displayAPI.hpp

  font.cpp
  font.hpp
)

# Add pico_stdlib library and hardware libraries
target_link_libraries(displayAPI 
  pico_stdlib 
  hardware_spi 
  hardware_gpio 

  # Using Pico W
  pico_cyw43_arch_none
)

target_include_directories(displayAPI PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")