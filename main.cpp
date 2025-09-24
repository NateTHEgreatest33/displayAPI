/*********************************************************************
*
*   NAME:
*       main.cpp for testing displayAPI
*
*   Copyright 2025 Nate Lenze
*
*********************************************************************/
/*--------------------------------------------------------------------
                              INCLUDES
--------------------------------------------------------------------*/
#include "hardware/spi.h"
#include "pico/stdlib.h"

#include "displayAPI.hpp"

/*--------------------------------------------------------------------
                          GLOBAL NAMESPACES
--------------------------------------------------------------------*/
/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/
#define SPI_PORT spi0
#define PIN_MISO 0
#define PIN_CS   17
#define PIN_SCK  6
#define PIN_MOSI 7
#define PIN_RST  21
#define PIN_DC   20
#define PIN_BL   0

/*--------------------------------------------------------------------
                            TYPES/ENUMS
--------------------------------------------------------------------*/

/*********************************************************************
*
*   PROCEDURE NAME:
*       main
*
*   DESCRIPTION:
*       Main function
*
*********************************************************************/
int main()
    {
    stdio_init_all();

    spi_init(SPI_PORT, 1000 * 1000);
    // gpio_set_function(PIN_MISO, GPIO_FUNC_SPI); //there is no MISO data
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    while(1)
        {

        DisplayProperties props = {135, 240, 240, 320, 52, 40}; //52,40 confirmed correct //53,40 works
        ST7789VW display(SPI_PORT, props, PIN_CS, PIN_DC, PIN_RST, PIN_BL);
        display.init();

        //block data tx
        int rot_var = 0;
        while(rot_var == 0 ){
        display.toggleDisplay(false);
        }


        display.set_rotation( ST7789VW::Rotation::ROTATION_90 );
        display.fill( 0x0000 );
        display.write_string("Hello, World! This is a very long string that should wrap around to the next line.", 0xFFFF, true, true);// this works
        display.write_string("Hello, World2! This is a very long string that should wrap around to the next line.", 0xFFFF, true, true);
        sleep_ms(1000);


        display.toggleDisplay(false);
        display.clear_screen();
        display.fill(0xF800);
        display.toggleDisplay(true);
        sleep_ms(1000);
        display.toggleDisplay(false);
        display.fill(0x0000);
        display.toggleDisplay(true);

        }


    return 0;
    }