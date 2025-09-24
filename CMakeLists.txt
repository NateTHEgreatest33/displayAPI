cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(SmartHomeCore C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

project(displayAPI C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

pico_sdk_init()

add_executable(displayAPI
    displayAPI.cpp
    font.cpp

    main.cpp
)

# Add pico_stdlib library and hardware libraries
target_link_libraries(displayAPI 
  pico_stdlib 
  hardware_spi 
  hardware_gpio 
  pico_cyw43_arch_none
)

# enable usb output, disable uart output
pico_enable_stdio_usb(displayAPI 1)
pico_enable_stdio_uart(displayAPI 0)


#make uf2
pico_add_extra_outputs(displayAPI)