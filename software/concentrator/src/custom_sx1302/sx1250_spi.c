/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Functions used to handle LoRa concentrator SX1250 radios.

License: Revised BSD License, see LICENSE.TXT file include in the project
*/

/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

#include <stdint.h> /* C99 types */
#include <stdio.h>  /* printf fprintf */
#include <unistd.h> /* lseek, close */
#include <fcntl.h>  /* open */
#include <string.h> /* memset */

// #include <sys/ioctl.h>
// #include <linux/spi/spidev.h>

#include "loragw_spi.h"
#include "loragw_aux.h"
#include "sx1250_spi.h"

#include "concentrator_spi.h"
#include "spi_custom_init.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#if DEBUG_RAD == 1
#define DEBUG_MSG(str) fprintf(stdout, str)
#define DEBUG_PRINTF(fmt, args...) fprintf(stdout, "%s:%d: " fmt, __FUNCTION__, __LINE__, args)
#define CHECK_NULL(a)                                                                        \
    if (a == NULL)                                                                           \
    {                                                                                        \
        fprintf(stderr, "%s:%d: ERROR: NULL POINTER AS ARGUMENT\n", __FUNCTION__, __LINE__); \
        return LGW_SPI_ERROR;                                                                \
    }
#else
#define DEBUG_MSG(str)
#define DEBUG_PRINTF(fmt, args...)
#define CHECK_NULL(a)         \
    if (a == NULL)            \
    {                         \
        return LGW_SPI_ERROR; \
    }
#endif

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */

#define WAIT_BUSY_SX1250_MS 1

/* -------------------------------------------------------------------------- */

static SPI_handle_t this_spi = 0;

/* --- PUBLIC FUNCTIONS DEFINITION ------------------------------------------ */

int sx1250_spi_init(struct SPI_handle_s *spi_handle)
{
    this_spi = spi_handle;
    return 0;
}

int sx1250_spi_w(void *com_target, uint8_t spi_mux_target, sx1250_op_code_t op_code, uint8_t *data, uint16_t size)
{
    int cmd_size = 2; /* header + op_code */
    CHECK_NULL(com_target);
    uint8_t out_buf[cmd_size + size];
    uint8_t command_size;
    uint8_t in_buf[ARRAY_SIZE(out_buf)];

    SPI_handle_t spi_handle = (SPI_handle_t ) com_target;

    /* prepare frame to be sent */
    out_buf[0] = spi_mux_target;
    out_buf[1] = (uint8_t)op_code;
    for (int i = 0; i < (int)size; i++)
    {
        out_buf[cmd_size + i] = data[i];
    }
    command_size = cmd_size + size;

    if (!(concentrator_spi_write_read(spi_handle, out_buf, in_buf, command_size)))
    {
        return LGW_SPI_ERROR;
    }
    return LGW_SPI_SUCCESS;
#if 0
    int com_device;
    int cmd_size = 2; /* header + op_code */
    uint8_t out_buf[cmd_size + size];
    uint8_t command_size;
    struct spi_ioc_transfer k;
    int a, i;

    /* wait BUSY */
    wait_ms(WAIT_BUSY_SX1250_MS);

    /* check input variables */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    com_device = *(int *)com_target;

    /* prepare frame to be sent */
    out_buf[0] = spi_mux_target;
    out_buf[1] = (uint8_t)op_code;
    for(i = 0; i < (int)size; i++) {
        out_buf[cmd_size + i] = data[i];
    }
    command_size = cmd_size + size;

    /* I/O transaction */
    memset(&k, 0, sizeof(k)); /* clear k */
    k.tx_buf = (unsigned long) out_buf;
    k.len = command_size;
    k.speed_hz = SPI_SPEED;
    k.cs_change = 0;
    k.bits_per_word = 8;
    a = ioctl(com_device, SPI_IOC_MESSAGE(1), &k);

    /* determine return code */
    if (a != (int)k.len) {
        DEBUG_MSG("ERROR: SPI WRITE FAILURE\n");
        return LGW_SPI_ERROR;
    } else {
        DEBUG_MSG("Note: SPI write success\n");
        return LGW_SPI_SUCCESS;
    }
    return LGW_SPI_SUCCESS;
#endif
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int sx1250_spi_r(void *com_target, uint8_t spi_mux_target, sx1250_op_code_t op_code, uint8_t *data, uint16_t size)
{
    int cmd_size = 2; /* header + op_code + NOP */
    uint8_t out_buf[cmd_size + size];
    uint8_t command_size;
    uint8_t in_buf[ARRAY_SIZE(out_buf)];

    /* wait BUSY */
    wait_ms(WAIT_BUSY_SX1250_MS);

    /* check input variables */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    SPI_handle_t spi_handle = (SPI_handle_t ) com_target;

    /* prepare frame to be sent */
    out_buf[0] = spi_mux_target;
    out_buf[1] = (uint8_t)op_code;
    for (int i = 0; i < (int)size; i++)
    {
        out_buf[cmd_size + i] = data[i];
    }
    command_size = cmd_size + size;

    if (!(concentrator_spi_write_read(spi_handle, out_buf, in_buf, command_size)))
    {
        return LGW_SPI_ERROR;
    }
    memcpy(data, in_buf + cmd_size, size);
    return LGW_SPI_SUCCESS;
#if 0
    int com_device;
    int cmd_size = 2; /* header + op_code + NOP */
    uint8_t out_buf[cmd_size + size];
    uint8_t command_size;
    uint8_t in_buf[ARRAY_SIZE(out_buf)];
    struct spi_ioc_transfer k;
    int a, i;

    /* wait BUSY */
    wait_ms(WAIT_BUSY_SX1250_MS);

    /* check input variables */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    com_device = *(int *)com_target;

    /* prepare frame to be sent */
    out_buf[0] = spi_mux_target;
    out_buf[1] = (uint8_t)op_code;
    for(i = 0; i < (int)size; i++) {
        out_buf[cmd_size + i] = data[i];
    }
    command_size = cmd_size + size;

    /* I/O transaction */
    memset(&k, 0, sizeof(k)); /* clear k */
    k.tx_buf = (unsigned long) out_buf;
    k.rx_buf = (unsigned long) in_buf;
    k.len = command_size;
    k.cs_change = 0;
    a = ioctl(com_device, SPI_IOC_MESSAGE(1), &k);

    /* determine return code */
    if (a != (int)k.len) {
        DEBUG_MSG("ERROR: SPI READ FAILURE\n");
        return LGW_SPI_ERROR;
    } else {
        DEBUG_MSG("Note: SPI read success\n");
        //*data = in_buf[command_size - 1];
        memcpy(data, in_buf + cmd_size, size);
        return LGW_SPI_SUCCESS;
    }
    return LGW_SPI_SUCCESS;
#endif
}

/* --- EOF ------------------------------------------------------------------ */
