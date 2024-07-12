#include "loragw_usb.h"
#include "sx1261_usb.h"
#include "sx1250_usb.h"
int lgw_usb_open(const char *com_path, void **com_target_ptr) { return 0; }

int lgw_usb_close(void *com_target) { return 0; }

int lgw_usb_w(void *com_target, uint8_t spi_mux_target, uint16_t address, uint8_t data) { return 0; }

int lgw_usb_r(void *com_target, uint8_t spi_mux_target, uint16_t address, uint8_t *data) { return 0; }

int lgw_usb_wb(void *com_target, uint8_t spi_mux_target, uint16_t address, const uint8_t *data, uint16_t size) { return 0; }

int lgw_usb_rb(void *com_target, uint8_t spi_mux_target, uint16_t address, uint8_t *data, uint16_t size) { return 0; }

int lgw_usb_rmw(void *com_target, uint16_t address, uint8_t offs, uint8_t leng, uint8_t data) { return 0; }

int lgw_usb_set_write_mode(lgw_com_write_mode_t write_mode) { return 0; }

int lgw_usb_flush(void *com_target) { return 0; }

uint16_t lgw_usb_chunk_size(void) { return 0; }

int lgw_usb_get_temperature(void *com_target, float *temperature) { return 0; }

int sx1261_usb_w(void *com_target, sx1261_op_code_t op_code, uint8_t *data, uint16_t size) { return 0; }

int sx1261_usb_r(void *com_target, sx1261_op_code_t op_code, uint8_t *data, uint16_t size) { return 0; }

int sx1261_usb_set_write_mode(lgw_com_write_mode_t write_mode) { return 0; }

int sx1261_usb_flush(void *com_target) { return 0; }

int sx1250_usb_w(void *com_target, uint8_t spi_mux_target, sx1250_op_code_t op_code, uint8_t *data, uint16_t size) { return 0; }

int sx1250_usb_r(void *com_target, uint8_t spi_mux_target, sx1250_op_code_t op_code, uint8_t *data, uint16_t size) { return 0; }