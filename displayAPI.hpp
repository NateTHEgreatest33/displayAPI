#ifndef DISPLAY_API_HPP
#define DISPLAY_API_HPP

#include "pico/stdlib.h"
#include "hardware/spi.h"

class ST7789VW {
public:
    ST7789VW(spi_inst_t* spi, uint16_t width, uint16_t height, uint16_t x_offset, uint16_t y_offset, uint cs_pin, uint dc_pin, uint rst_pin, uint bl_pin);

    void init();
    void setWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    void drawPixel(uint16_t x, uint16_t y, uint16_t color);
    void fill(uint16_t color);
    void drawChar(uint16_t x, uint16_t y, char c, uint16_t color);
    void drawText(uint16_t x, uint16_t y, const char* text, uint16_t color);
    void clear_screen(void);

private:
    void sendCommand(uint8_t cmd);
    void sendData(const uint8_t* data, size_t len);
    void reset();

    spi_inst_t* _spi;
    uint _cs_pin;
    uint _dc_pin;
    uint _rst_pin;
    uint _bl_pin;

    uint16_t _width;
    uint16_t _height;
    uint16_t _x_offset;
    uint16_t _y_offset;
};

#endif // DISPLAY_API_HPP