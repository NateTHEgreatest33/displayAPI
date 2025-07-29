/*********************************************************************
*
*   NAME:
*       displayAPI.cpp
*
*   DESCRIPTION:
*       API for ST7789VW display
*
*   Copyright 2023 Nate Lenze
*
*********************************************************************/

/*--------------------------------------------------------------------
                              INCLUDES
--------------------------------------------------------------------*/
#include "displayAPI.hpp"
#include "font.hpp"
#include "hardware/gpio.h"
#include <stdio.h>

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
                                TYPES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                           MEMORY CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              VARIABLES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                                MACROS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              PROCEDURES
--------------------------------------------------------------------*/
/*********************************************************************
*
*   PROCEDURE NAME:
*       ST7789VW::ST7789VW (constructor)
*
*   DESCRIPTION:
*       ST7789VW class constructor
*
*********************************************************************/
ST7789VW::ST7789VW(spi_inst_t* spi, DisplayProperties props, uint cs_pin, uint dc_pin, uint rst_pin, uint bl_pin)
    : _spi(spi), _props(props), _default_props(props), _cs_pin(cs_pin), _dc_pin(dc_pin), _rst_pin(rst_pin), _bl_pin(bl_pin), _last_x(0), _last_y(0)
    {
    }

/*********************************************************************
*
*   PROCEDURE NAME:
*       ST7789VW::~ST7789VW (deconstructor)
*
*   DESCRIPTION:
*       ST7789VW class deconstructor
*
*********************************************************************/
ST7789VW::~ST7789VW( void )
    {
    }

/*********************************************************************
*
*   PROCEDURE NAME:
*       ST7789VW::init
*
*   DESCRIPTION:
*       Initializes the display
*
*********************************************************************/
void ST7789VW::init()
    {
    gpio_init(_cs_pin);
    gpio_set_dir(_cs_pin, GPIO_OUT);
    gpio_put(_cs_pin, 1);

    gpio_init(_dc_pin);
    gpio_set_dir(_dc_pin, GPIO_OUT);

    gpio_init(_rst_pin);
    gpio_set_dir(_rst_pin, GPIO_OUT);

    if( _bl_pin != 0 )
        {
        gpio_init(_bl_pin);
        gpio_set_dir(_bl_pin, GPIO_OUT);
        gpio_put(_bl_pin, 1);
        }


    reset();

    sendCommand(ST7789VW_CMD::SWRESET);
    sleep_ms(150);

    sendCommand(ST7789VW_CMD::SLPOUT);
    sleep_ms(50);

    sendCommand(ST7789VW_CMD::COLMOD);
    uint8_t colmod_data[] = {0x55}; // 16-bit/pixel
    sendData(colmod_data, sizeof(colmod_data));

    sendCommand(ST7789VW_CMD::MADCTL);
    uint8_t madctl_data[] = {0x00};
    sendData(madctl_data, sizeof(madctl_data));

    sendCommand(ST7789VW_CMD::NORON);
    sendCommand(ST7789VW_CMD::INVON);
    sendCommand(ST7789VW_CMD::DISPON);
    sleep_ms(50);
    }

/*********************************************************************
*
*   PROCEDURE NAME:
*       ST7789VW::setWindow
*
*   DESCRIPTION:
*       Sets the drawing window
*
*********************************************************************/
void ST7789VW::setWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
    {
    uint16_t x_start = x + _props.x_offset;
    uint16_t y_start = y + _props.y_offset;
    uint16_t x_end = x + width - 1 + _props.x_offset;
    uint16_t y_end = y + height - 1 + _props.y_offset;

    sendCommand(ST7789VW_CMD::CASET);
    uint8_t caset_data[] = {(uint8_t)(x_start >> 8), (uint8_t)x_start, (uint8_t)(x_end >> 8), (uint8_t)x_end};
    sendData(caset_data, sizeof(caset_data));

    sendCommand(ST7789VW_CMD::RASET);
    uint8_t raset_data[] = {(uint8_t)(y_start >> 8), (uint8_t)y_start, (uint8_t)(y_end >> 8), (uint8_t)y_end};
    sendData(raset_data, sizeof(raset_data));

    sendCommand(ST7789VW_CMD::RAMWR);
    }

/*********************************************************************
*
*   PROCEDURE NAME:
*       ST7789VW::drawPixel
*
*   DESCRIPTION:
*       Draws a single pixel
*
*********************************************************************/
void ST7789VW::drawPixel(uint16_t x, uint16_t y, uint16_t color)
    {
    setWindow(x, y, 1, 1);
    uint8_t pixel_data[] = {(uint8_t)(color >> 8), (uint8_t)color};
    sendData(pixel_data, sizeof(pixel_data));
    }

/*********************************************************************
*
*   PROCEDURE NAME:
*       ST7789VW::fill
*
*   DESCRIPTION:
*       Fills the screen with a color
*
*********************************************************************/
void ST7789VW::fill(uint16_t color)
    {
    setWindow(0, 0, _props.width, _props.height);
    uint32_t num_pixels = _props.width * _props.height;
    uint8_t pixel_data[] = {(uint8_t)(color >> 8), (uint8_t)color};

    for (uint32_t i = 0; i < num_pixels; ++i) {
        sendData(pixel_data, sizeof(pixel_data));
    }
    }

/*********************************************************************
*
*   PROCEDURE NAME:
*       ST7789VW::drawChar
*
*   DESCRIPTION:
*       Draws a single character
*
*********************************************************************/
void ST7789VW::drawChar(uint16_t x, uint16_t y, char c, uint16_t color)
    {
    if (x > _props.width - 8 || y > _props.height - 8) {
        return;
    }

    for (int i = 0; i < 8; i++) {
        uint8_t line = font[(int)c][i];
        for (int j = 0; j < 8; j++) {
            if ((line >> (7 - j)) & 1) {
                drawPixel(x + j, y + i, color);
            }
        }
    }
    }

/*********************************************************************
*
*   PROCEDURE NAME:
*       ST7789VW::drawText
*
*   DESCRIPTION:
*       Draws a string of text
*
*********************************************************************/
void ST7789VW::drawText(uint16_t x, uint16_t y, const char* text, uint16_t color)
    {
    int i = 0;
    while (text[i]) {
        drawChar(x + (i * 8), y, text[i], color);
        i++;
    }
    }

/*********************************************************************
*
*   PROCEDURE NAME:
*       ST7789VW::sendCommand
*
*   DESCRIPTION:
*       Sends a command to the display
*
*********************************************************************/
void ST7789VW::sendCommand(ST7789VW_CMD cmd)
    {
    uint8_t cmd_val = static_cast<uint8_t>(cmd);
    gpio_put(_cs_pin, 0);
    gpio_put(_dc_pin, 0);
    spi_write_blocking(_spi, &cmd_val, 1);
    gpio_put(_cs_pin, 1);
    }

/*********************************************************************
*
*   PROCEDURE NAME:
*       ST7789VW::sendData
*
*   DESCRIPTION:
*       Sends data to the display
*
*********************************************************************/
void ST7789VW::sendData(const uint8_t* data, size_t len)
    {
    gpio_put(_cs_pin, 0);
    gpio_put(_dc_pin, 1);
    spi_write_blocking(_spi, data, len);
    gpio_put(_cs_pin, 1);
    }

/*********************************************************************
*
*   PROCEDURE NAME:
*       ST7789VW::reset
*
*   DESCRIPTION:
*       Resets the display
*
*********************************************************************/
void ST7789VW::reset()
    {
    gpio_put(_rst_pin, 0);
    sleep_ms(10);
    gpio_put(_rst_pin, 1);
    sleep_ms(120);
    }

/*********************************************************************
*
*   PROCEDURE NAME:
*       ST7789VW::clear_screen
*
*   DESCRIPTION:
*       Clears the screen
*
*********************************************************************/
void ST7789VW::clear_screen()
    {
    fill(0x0000); //black color
    }

/*********************************************************************
*
*   PROCEDURE NAME:
*       ST7789VW::write_string_pos
*
*   DESCRIPTION:
*       Writes a string at a specific position
*
*********************************************************************/
bool ST7789VW::write_string_pos(uint16_t x, uint16_t y, const char* text, uint16_t color, bool word_wrap)
    {
    uint16_t current_x = x;
    uint16_t current_y = y;
    int i = 0;
    while (text[i]) {
        if (word_wrap) {
            const char* word_start = &text[i];
            int word_len = 0;
            while (text[i] && text[i] != ' ') {
                word_len++;
                i++;
            }

            if (current_x + (word_len * 8) > _props.width) {
                current_x = x;
                current_y += 8;
            }

            for (int j = 0; j < word_len; j++) {
                drawChar(current_x, current_y, word_start[j], color);
                current_x += 8;
            }

            if (text[i] == ' ') {
                drawChar(current_x, current_y, ' ', color);
                current_x += 8;
                i++;
            }

        } else {
            if (current_x + 8 > _props.width) {
                current_x = x;
                current_y += 8;
            }
            drawChar(current_x, current_y, text[i], color);
            current_x += 8;
            i++;
        }
    }
    _last_x = current_x;
    _last_y = current_y;
    return true;
    }

/*********************************************************************
*
*   PROCEDURE NAME:
*       ST7789VW::write_string
*
*   DESCRIPTION:
*       Writes a string
*
*********************************************************************/
bool ST7789VW::write_string(const char* text, uint16_t color, bool newline, bool word_wrap)
    {
    uint16_t start_x = _last_x;
    uint16_t start_y = _last_y;

    if (newline) {
        start_x = 0;
        start_y += 8;
    }

    return write_string_pos(start_x, start_y, text, color, word_wrap);
    }

/*********************************************************************
*
*   PROCEDURE NAME:
*       ST7789VW::set_rotation
*
*   DESCRIPTION:
*       Sets the display rotation
*
*********************************************************************/
void ST7789VW::set_rotation(Rotation rotation)
    {
    _rotation = rotation;
    uint8_t madctl_data;

    switch (rotation) {
        case Rotation::ROTATION_0:
            madctl_data = 0x00;
            _props.width = _default_props.width;
            _props.height =_default_props.height;
            _props.x_offset = _default_props.x_offset;
            _props.y_offset = _default_props.y_offset;
            break;
        case Rotation::ROTATION_90:
            madctl_data = 0x60;
            _props.width = _default_props.height;
            _props.height = _default_props.width;
            _props.x_offset = _default_props.y_offset;
            _props.y_offset = (_default_props.x_offset + 1);
            break;
        case Rotation::ROTATION_180:
            madctl_data = 0xC0;
            _props.width = _default_props.width;
            _props.height = _default_props.height;
            _props.x_offset = (_default_props.x_offset + 1);
            _props.y_offset = _default_props.y_offset;
            break;
        case Rotation::ROTATION_270:
            madctl_data = 0xA0;
            _props.width = _default_props.height;
            _props.height = _default_props.width;
            _props.x_offset = _default_props.y_offset;
            _props.y_offset = _default_props.x_offset;
            break;
    }

    sendCommand(ST7789VW_CMD::MADCTL);
    sendData(&madctl_data, 1);
    }

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



//testing stuff
int rot_var = (int)ST7789VW::Rotation::ROTATION_0;

while(1)
    {
    {
    DisplayProperties props1 = {240, 320, 240, 320, 0, 0};
    ST7789VW display1(SPI_PORT, props1, PIN_CS, PIN_DC, PIN_RST, PIN_BL);
    display1.init();
    sleep_ms(1000);
    display1.set_rotation( ST7789VW::Rotation::ROTATION_0 );
    display1.fill( 0xFFFF ); //fill entire map white
    sleep_ms(100);
    }


    {
    DisplayProperties props = {135, 240, 240, 320, 52, 40}; //52,40 confirmed correct //53,40 works
    ST7789VW display(SPI_PORT, props, PIN_CS, PIN_DC, PIN_RST, PIN_BL);
    display.init();
    display.set_rotation( ST7789VW::Rotation::ROTATION_90 );
    display.fill( 0x0000 );
    display.write_string("Hello, World! This is a very long string that should wrap around to the next line.", 0xFFFF, true, true);// this works
    display.write_string("Hello, World2! This is a very long string that should wrap around to the next line.", 0xFFFF, true, true);
    sleep_ms(100);
    }

//rotation 90 (53,40)
//rotation 180 (53,40)
//rotation 270 (52,40)
//rotation 0    (52,40)



    // {
    // DisplayProperties props = {135, 240, 240, 320, 52, 40}; //52,40 confirmed correct
    // ST7789VW display(SPI_PORT, props, PIN_CS, PIN_DC, PIN_RST, PIN_BL);
    // display.init();
    // display.set_rotation( ST7789VW::Rotation::ROTATION_180 );
    // display.fill( 0x0000 );
    // }

    // {
    // DisplayProperties props = {135, 240, 240, 320, 52, 40}; //52,40 confirmed correct
    // ST7789VW display(SPI_PORT, props, PIN_CS, PIN_DC, PIN_RST, PIN_BL);
    // display.init();
    // display.set_rotation( ST7789VW::Rotation::ROTATION_270 );
    // display.fill( 0x0000 );
    // }


    }



    // DisplayProperties props = {135, 240, 240, 320, 52, 40}; //52, 40 is confirmed correct
    // ST7789VW display(SPI_PORT, props, PIN_CS, PIN_DC, PIN_RST, PIN_BL);
    // display.init();
    // display.set_rotation(ST7789VW::Rotation::ROTATION_0);
    // display.fill( 0x0000 );
    // while (1) {
    //     // display.fill(0x0000);
    //     display.clear_screen();
    //     sleep_ms(100);
    //     // display.write_string(0, 2, "Hello, World! This is a very long string that should wrap around to the next line.", 0xFFFF, true);// this works
    //     display.write_string(0, 0, "Hello, World! This is a very long string that should wrap around to the next line.", 0xFFFF, false);// this works
    //     sleep_ms(2000);

    //     display.set_rotation(ST7789VW::Rotation::ROTATION_90);
    //     display.clear_screen();
    //     sleep_ms(100);
    //     // display.write_string(0, 2, "Hello, World! This is a very long string that should wrap around to the next line.", 0xFFFF, true);// this works
    //     display.write_string(0, 0, "Hello, World! This is a very long string that should wrap around to the next line.", 0xFFFF, false);// this works
    //     sleep_ms(2000); //doesnt

    //     display.set_rotation(ST7789VW::Rotation::ROTATION_180);
    //     display.clear_screen();
    //     sleep_ms(100);
    //     // display.write_string(0, 2, "Hello, World! This is a very long string that should wrap around to the next line.", 0xFFFF, true);// this works
    //     display.write_string(0, 0, "Hello, World! This is a very long string that should wrap around to the next line.", 0xFFFF, false);// this works
    //     sleep_ms(2000);

    //     display.set_rotation(ST7789VW::Rotation::ROTATION_270);
    //     display.clear_screen();
    //     sleep_ms(100);
    //     // display.write_string(0, 2, "Hello, World! This is a very long string that should wrap around to the next line.", 0xFFFF, true);// this works
    //     display.write_string(0, 0, "Hello, World! This is a very long string that should wrap around to the next line.", 0xFFFF, false);// this works
    //     sleep_ms(2000);


    // }

    return 0;
    }