#ifndef AT_TRANS_H
#define AT_TRANS_H
#include "uart.h"
extern uint8_t is_first_flag;

#define ESP_OK 0
#define ESP_FAIL -1

#define AT_RET_SUFFIX '\r\n\r\n'
#define AT_RECV_SUFFIX '\r'

#define ATF_WAIT_TX_OVER() {while (READ_PERI_REG(UART_STATUS(0)) & (UART_TXFIFO_CNT << UART_TXFIFO_CNT_S))\
{WRITE_PERI_REG(0X60000914, 0X73);}}

#define ATF_WAIT_TX_CLEAR() {SET_PERI_REG_MASK(UART_CONF0(0), UART_TXFIFO_RST);\
    						 CLEAR_PERI_REG_MASK(UART_CONF0(0), UART_TXFIFO_RST);}

#define atf_ret_ok(s)   at_port_print("+ok"s"\r\n\r\n")
#define atf_ret_err(s)  at_port_print("+ERR"s"\r\n\r\n")

#define ESP_LOG(s) os_printf("[%s %d] "s"\r\n", __func__,__LINE__)
//function
int at_query_wifi_mode(uint8_t id);
int at_setup_wifi_mode(uint8_t id, char *pPara);
int at_ret_wifi_mode(uint8_t id, char *pPara);

int atf_query_uart(uint8_t id);
int atf_setup_uart(uint8_t id, char *pPara);
int atf_ret_uart(uint8_t id, char *pPara);

int atf_ret_mac(uint8_t id, char *pPara);
int atf_query_mac(uint8_t id);
int atf_setup_mac(uint8_t id, char *pPara);

int atf_query_link_status(uint8_t id);

int atf_query_qjoine(uint8_t id);
int atf_ret_qjoine(uint8_t id, char *pPara);

int atf_query_enter_through(uint8_t id);
int atf_ret_enter_through(uint8_t id, char *pPara);

int atf_query_exit_through(uint8_t id);

int at_ret_net_info(uint8_t id, char *pPara);
int at_setup_net_info(uint8_t id, char *pPara);
//end

void change_uart_reg(void);
void ICACHE_FLASH_ATTR at_uart_tx_byte_filter_call(const uint8*data,uint32 length);
void ICACHE_FLASH_ATTR at_uart_rx_intr_call(uint8* data,int32 len);
void ICACHE_FLASH_ATTR at_uart_tx_complete_call(void);

#endif
