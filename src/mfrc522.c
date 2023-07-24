#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pico/time.h"
#include "mfrc522.h"

void mfrc522_write_register(mfrc522 *device, mfrc522_register reg, uint8_t val) {    
    uint8_t buf[2] = { (reg << 1) & 0x7e, val };
    
    gpio_put(device->cs, false);
    spi_write_blocking(device->spi, buf, 2);
    gpio_put(device->cs, true);
}

uint8_t mfrc522_read_register(mfrc522 *device, mfrc522_register reg) {
    uint8_t data[1];
    uint8_t buf[1] = { ((reg << 1) & 0x7e) | 0x80 };

    gpio_put(device->cs, false);
    spi_write_blocking(device->spi, buf, 1);
    spi_read_blocking(device->spi, 0, data, 1);
    gpio_put(device->cs, true);

    return *data;
}

void mfrc522_reset(mfrc522 *device) {
    mfrc522_write_register(device, Command, SoftReset);
}

uint8_t mfrc522_to_card(mfrc522_command command) {

}

void mfrc522_init(mfrc522 *device) {
    spi_init(device->spi, 1000000); // this thing operates at 10 kHz
    gpio_set_function(device->mosi, GPIO_FUNC_SPI);
    gpio_set_function(device->miso, GPIO_FUNC_SPI);
    gpio_set_function(device->sck, GPIO_FUNC_SPI);
    gpio_init(device->cs);
    gpio_set_dir(device->cs, GPIO_OUT);
    gpio_put(device->cs, true);
}