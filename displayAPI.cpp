#include "displayAPI.hpp"
#include "hardware/gpio.h"
#include <stdio.h>

// Commands
#define CMD_SWRESET 0x01
#define CMD_SLPOUT 0x11
#define CMD_NORON 0x13
#define CMD_INVOFF 0x20
#define CMD_INVON  0x21 //this inverts collors so that 0xFFFF white and 0x0000 is black
#define CMD_DISPON 0x29
#define CMD_CASET 0x2A
#define CMD_RASET 0x2B
#define CMD_RAMWR 0x2C
#define CMD_COLMOD 0x3A
#define CMD_MADCTL 0x36

// SPI Defines
// #define SPI_PORT spi0
// #define PIN_MISO 4
// #define PIN_CS   5
// #define PIN_SCK  6
// #define PIN_MOSI 7
// #define PIN_RST  8
// #define PIN_DC   9
// #define PIN_BL   10
#define SPI_PORT spi0
#define PIN_MISO 0
#define PIN_CS   17
#define PIN_SCK  6
#define PIN_MOSI 7
#define PIN_RST  21
#define PIN_DC   20
#define PIN_BL   0


// const struct st7789_config lcd_config = {
//     .spi      = spi0_hw, //works
//     // .gpio_din = 19,
//     .gpio_din = 7, //works
//     // .gpio_clk = 18, 
//     .gpio_clk = 6,  //works
//     .gpio_cs  = 17,
//     .gpio_dc  = 20,
//     .gpio_rst = 21,
//     .gpio_bl  = 0, //not used?
// };


ST7789VW::ST7789VW(spi_inst_t* spi, uint cs_pin, uint dc_pin, uint rst_pin, uint bl_pin)
    : _spi(spi), _cs_pin(cs_pin), _dc_pin(dc_pin), _rst_pin(rst_pin), _bl_pin(bl_pin) {}

void ST7789VW::init() {
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

    sendCommand(CMD_SWRESET);
    sleep_ms(150);

    sendCommand(CMD_SLPOUT);
    sleep_ms(50);

    sendCommand(CMD_COLMOD);
    uint8_t colmod_data[] = {0x55}; // 16-bit/pixel
    sendData(colmod_data, sizeof(colmod_data));

    sendCommand(CMD_MADCTL);
    uint8_t madctl_data[] = {0x00};
    sendData(madctl_data, sizeof(madctl_data));

    sendCommand(CMD_NORON);
    sendCommand(CMD_INVON);
    sendCommand(CMD_DISPON);
    sleep_ms(50);
}

void ST7789VW::setWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    uint16_t x_end = x + width - 1;
    uint16_t y_end = y + height - 1;

    sendCommand(CMD_CASET);
    uint8_t caset_data[] = {(uint8_t)(x >> 8), (uint8_t)x, (uint8_t)(x_end >> 8), (uint8_t)x_end};
    sendData(caset_data, sizeof(caset_data));

    sendCommand(CMD_RASET);
    uint8_t raset_data[] = {(uint8_t)(y >> 8), (uint8_t)y, (uint8_t)(y_end >> 8), (uint8_t)y_end};
    sendData(raset_data, sizeof(raset_data));

    sendCommand(CMD_RAMWR);
}

void ST7789VW::drawPixel(uint16_t x, uint16_t y, uint16_t color) {
    setWindow(x, y, 1, 1);
    uint8_t pixel_data[] = {(uint8_t)(color >> 8), (uint8_t)color};
    sendData(pixel_data, sizeof(pixel_data));
}

void ST7789VW::fill(uint16_t color) {
    setWindow(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    uint32_t num_pixels = DISPLAY_WIDTH * DISPLAY_HEIGHT;
    uint8_t pixel_data[] = {(uint8_t)(color >> 8), (uint8_t)color};

    for (uint32_t i = 0; i < num_pixels; ++i) {
        sendData(pixel_data, sizeof(pixel_data));
    }
}


void ST7789VW::sendCommand(uint8_t cmd) {
    gpio_put(_cs_pin, 0);
    gpio_put(_dc_pin, 0);
    spi_write_blocking(_spi, &cmd, 1);
    gpio_put(_cs_pin, 1);
}

void ST7789VW::sendData(const uint8_t* data, size_t len) {
    gpio_put(_cs_pin, 0);
    gpio_put(_dc_pin, 1);
    spi_write_blocking(_spi, data, len);
    gpio_put(_cs_pin, 1);
}

void ST7789VW::reset() {
    gpio_put(_rst_pin, 0);
    sleep_ms(10);
    gpio_put(_rst_pin, 1);
    sleep_ms(120);
}

void ST7789VW::clear_screen() {
    fill(0x0000); //black color

}

int main() {
    stdio_init_all();

    spi_init(SPI_PORT, 1000 * 1000);
    // gpio_set_function(PIN_MISO, GPIO_FUNC_SPI); //there is no MISO data
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    ST7789VW display(SPI_PORT, PIN_CS, PIN_DC, PIN_RST, PIN_BL);
    display.init();

    while (1) {
        display.fill(0xFFFF); // White
        sleep_ms(1000);
        display.fill(0x001F); // Blue
        sleep_ms(1000);
        display.fill(0xF800); // Red
        sleep_ms(1000);
        display.fill(0x07E0); // Green
        sleep_ms(1000);
        display.clear_screen(); //white?
        sleep_ms(1000);
    }

    return 0;
}
