#ifndef LPUART_H
#define LPUART_H

#include <stdint.h>

/* 
 * Base address for LPUART0 peripheral 
 * (memory-mapped registers)
 */
#define LPUART0_BASE_ADDR   0x40328000

/* 
 * LPUART register definitions using memory-mapped I/O
 * Access is volatile to prevent compiler optimizations
 */
#define LPUART_BAUD         (*(volatile uint32_t *)(LPUART0_BASE_ADDR + 0x10)) /* Baud Rate Register */
#define LPUART_STAT         (*(volatile uint32_t *)(LPUART0_BASE_ADDR + 0x14)) /* Status Register */
#define LPUART_CTRL         (*(volatile uint32_t *)(LPUART0_BASE_ADDR + 0x18)) /* Control Register */
#define LPUART_DATA         (*(volatile uint32_t *)(LPUART0_BASE_ADDR + 0x1C)) /* Data Register */

/* Bit masks for LPUART status/control registers */
#define LPUART_STAT_TDRE    (1 << 23) /* Transmit Data Register Empty */
#define LPUART_STAT_RDRF    (1 << 21) /* Receive Data Register Full */
#define LPUART_CTRL_TE      (1 << 19) /* Transmitter Enable */
#define LPUART_CTRL_RE      (1 << 18) /* Receiver Enable */

/* Functions for basic LPUART operation */

/* Blocking printf-style output using LPUART */
void my_printf(const char *str);

/* Initialize LPUART0: set baud rate and enable TX/RX */
void lpuart_init(void);

#endif /* LPUART_H */




