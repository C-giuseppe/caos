#ifndef HW_S32K358_LPUART_H
#define HW_S32K358_LPUART_H

#include "hw/sysbus.h"       // QEMU SysBusDevice
#include "chardev/char-fe.h" // Character device frontend
#include "qom/object.h"      // QEMU Object Model

/* -------------------- LPUART Register Offsets -------------------- */
#define LPUART_BAUD    0x10  /* Baud Rate Register */
#define LPUART_STAT    0x14  /* Status Register */
#define LPUART_CTRL    0x18  /* Control Register */
#define LPUART_DATA    0x1C  /* Data Register */

/* -------------------- Status Register Bits -------------------- */
#define LPUART_STAT_TDRE    (1 << 23) /* Transmit Data Register Empty */
#define LPUART_STAT_RDRF    (1 << 21) /* Receive Data Register Full */

/* -------------------- Control Register Bits -------------------- */
#define LPUART_CTRL_TE      (1 << 19) /* Transmitter Enable */
#define LPUART_CTRL_RE      (1 << 18) /* Receiver Enable */

/* -------------------- Type Declaration -------------------- */
#define TYPE_S32K358_LPUART "s32k358-lpuart"
OBJECT_DECLARE_SIMPLE_TYPE(S32K358LPUARTState, S32K358_LPUART)

/* -------------------- LPUART Device State -------------------- */
struct S32K358LPUARTState {
    SysBusDevice parent_obj;  /* Inherits from SysBusDevice */

    MemoryRegion mmio;        /* MMIO region mapped to CPU address space */
    qemu_irq irq;             /* IRQ line for transmit/receive interrupts */

    uint32_t baud;            /* BAUD register: controls baud rate */
    uint32_t stat;            /* STAT register: transmit/receive status flags */
    uint32_t ctrl;            /* CTRL register: enables transmitter/receiver */
    uint32_t data;            /* DATA register: transmit/receive data byte */

    CharBackend chr;          /* Character backend for UART communication */
};

#endif /* HW_S32K358_LPUART_H */
