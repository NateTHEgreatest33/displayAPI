#ifndef DISPLAY_API_HPP
#define DISPLAY_API_HPP
/*********************************************************************
*
*   HEADER:
*       header file for displayAPI
*
*   Copyright 2023 Nate Lenze
*
*********************************************************************/
/*--------------------------------------------------------------------
                              INCLUDES
--------------------------------------------------------------------*/
#include "pico/stdlib.h"
#include "hardware/spi.h"

/*--------------------------------------------------------------------
                          GLOBAL NAMESPACES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                            TYPES/ENUMS
--------------------------------------------------------------------*/
enum class ST7789VW_CMD : uint8_t {
    SWRESET = 0x01,
    SLPOUT = 0x11,
    NORON = 0x13,
    INVOFF = 0x20,
    INVON = 0x21,
    DISPON = 0x29,
    CASET = 0x2A,
    RASET = 0x2B,
    RAMWR = 0x2C,
    COLMOD = 0x3A,
    MADCTL = 0x36,
};

typedef struct {
    uint16_t width;
    uint16_t height;
    uint16_t map_width;
    uint16_t map_height;
    uint16_t x_offset;
    uint16_t y_offset;
} DisplayProperties;

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

/*--------------------------------------------------------------------
                               CLASSES
--------------------------------------------------------------------*/
class ST7789VW {
    public:
        ST7789VW(spi_inst_t* spi, DisplayProperties props, uint cs_pin, uint dc_pin, uint rst_pin, uint bl_pin);
        ~ST7789VW( void );

        void init();
        void fill(uint16_t color);
        void clear_screen(void);
        bool write_string_pos(uint16_t x, uint16_t y, const char* text, uint16_t color, bool word_wrap = false);
        bool write_string(const char* text, uint16_t color, bool newline = false, bool word_wrap = false);

        enum class Rotation {
            ROTATION_0,
            ROTATION_90,
            ROTATION_180,
            ROTATION_270
        };
        void set_rotation(Rotation rotation);

    private:
        void setWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
        void drawPixel(uint16_t x, uint16_t y, uint16_t color);
        void drawChar(uint16_t x, uint16_t y, char c, uint16_t color);
        void drawText(uint16_t x, uint16_t y, const char* text, uint16_t color);
        void sendCommand(ST7789VW_CMD cmd);
        void sendData(const uint8_t* data, size_t len);
        void reset();

        spi_inst_t* _spi;
        uint _cs_pin;
        uint _dc_pin;
        uint _rst_pin;
        uint _bl_pin;

        DisplayProperties _props;
        DisplayProperties _default_props;
        Rotation _rotation;
        uint16_t _last_x;
        uint16_t _last_y;
};

#endif // DISPLAY_API_HPP