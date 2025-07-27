#ifndef DISPLAY_API_HPP
#define DISPLAY_API_HPP

#include "pico/stdlib.h"
#include "hardware/spi.h"

class ST7789VW {
public:
    ST7789VW(spi_inst_t* spi, uint cs_pin, uint dc_pin, uint rst_pin, uint bl_pin);

    void init();
    void setWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    void drawPixel(uint16_t x, uint16_t y, uint16_t color);
    void fill(uint16_t color);
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

    static const uint16_t DISPLAY_WIDTH = 240;
    static const uint16_t DISPLAY_HEIGHT = 320;
};

#endif // DISPLAY_API_HPP