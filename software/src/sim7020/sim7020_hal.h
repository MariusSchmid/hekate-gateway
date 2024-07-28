#ifndef B2753E96_8A4A_435E_967D_AC895D2ABA3B
#define B2753E96_8A4A_435E_967D_AC895D2ABA3B
#ifndef A16C374F_FD54_42E6_BBFB_74BDDABD7DAF
#define A16C374F_FD54_42E6_BBFB_74BDDABD7DAF
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief initialize uart
 * @param
 * @return true if successful
 */
bool sim7020_hal_uart_init(void);

bool sim7020_hal_send_cmd_check_recv(char *cmd, char *expected_result, uint32_t timeout);

bool sim7020_hal_send_cmd_get_recv(char *cmd, char *expected_result, uint32_t timeout, char *uart_response, uint16_t uart_response_buffer_size);

bool sim7020_hal_send(uint8_t *message, uint32_t size);

bool sim7020_hal_enable_sim_module(void);

bool sim7020_hal_sim_gpio_init(void);

bool sim7020_hal_init(void);

bool sim7020_hal_sleep_ms(uint32_t ms);


#endif /* A16C374F_FD54_42E6_BBFB_74BDDABD7DAF */

#endif /* B2753E96_8A4A_435E_967D_AC895D2ABA3B */
