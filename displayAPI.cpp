#include "displayAPI.hpp"
#include "font.hpp"
#include "hardware/gpio.h"
#include <stdio.h>



// SPI Defines
#define SPI_PORT spi0
#define PIN_MISO 0
#define PIN_CS   17
#define PIN_SCK  6
#define PIN_MOSI 7
#define PIN_RST  21
#define PIN_DC   20
#define PIN_BL   0


ST7789VW::ST7789VW(spi_inst_t* spi, DisplayProperties props, uint cs_pin, uint dc_pin, uint rst_pin, uint bl_pin)
    : _spi(spi), _props(props), _cs_pin(cs_pin), _dc_pin(dc_pin), _rst_pin(rst_pin), _bl_pin(bl_pin) {}

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

void ST7789VW::setWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
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

void ST7789VW::drawPixel(uint16_t x, uint16_t y, uint16_t color) {
    setWindow(x, y, 1, 1);
    uint8_t pixel_data[] = {(uint8_t)(color >> 8), (uint8_t)color};
    sendData(pixel_data, sizeof(pixel_data));
}

void ST7789VW::fill(uint16_t color) {
    setWindow(0, 0, _props.width, _props.height);
    uint32_t num_pixels = _props.width * _props.height;
    uint8_t pixel_data[] = {(uint8_t)(color >> 8), (uint8_t)color};

    for (uint32_t i = 0; i < num_pixels; ++i) {
        sendData(pixel_data, sizeof(pixel_data));
    }
}

void ST7789VW::drawChar(uint16_t x, uint16_t y, char c, uint16_t color) {
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

void ST7789VW::drawText(uint16_t x, uint16_t y, const char* text, uint16_t color) {
    int i = 0;
    while (text[i]) {
        drawChar(x + (i * 8), y, text[i], color);
        i++;
    }
}

void ST7789VW::sendCommand(ST7789VW_CMD cmd) {
    uint8_t cmd_val = static_cast<uint8_t>(cmd);
    gpio_put(_cs_pin, 0);
    gpio_put(_dc_pin, 0);
    spi_write_blocking(_spi, &cmd_val, 1);
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

bool ST7789VW::write_string(uint16_t x, uint16_t y, const char* text, uint16_t color, bool word_wrap)
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
    return true;
}

void ST7789VW::set_rotation(void){}

int main() {
    stdio_init_all();

    spi_init(SPI_PORT, 1000 * 1000);
    // gpio_set_function(PIN_MISO, GPIO_FUNC_SPI); //there is no MISO data
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    DisplayProperties props = {134, 240, 240, 320, 50, 40};
    ST7789VW display(SPI_PORT, props, PIN_CS, PIN_DC, PIN_RST, PIN_BL);
    display.init();

    while (1) {
        // display.fill(0x0000);
        display.clear_screen();
        sleep_ms(100);
        display.write_string(0, 0, "Hello, World! This is a very long string that should wrap around to the next line.", 0xFFFF, true);
        sleep_ms(2000);
    }

    return 0;
}