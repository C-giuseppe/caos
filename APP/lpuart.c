#include "lpuart.h"

/*
 * Simple blocking printf function for LPUART
 * - Sends a null-terminated string over the UART
 * - Waits until the transmit buffer is empty before sending each character
 */
void my_printf(const char *str) {
    while (*str) {
        /* Wait until the Transmit Data Register Empty (TDRE) flag is set */
        while (!(LPUART_STAT & LPUART_STAT_TDRE)) {
            /* Busy wait */
        }

        /* Write the next character to the DATA register */
        LPUART_DATA = *str++;
    }
}

/*
 * Initialize the LPUART peripheral
 * - Set a default baud rate
 * - Enable transmitter (TE) and receiver (RE)
 */
void lpuart_init(void){
    LPUART_BAUD = 0x1A0;                 /* Example baud rate configuration */
    LPUART_CTRL |= (LPUART_CTRL_TE | LPUART_CTRL_RE); /* Enable TX and RX */
}

