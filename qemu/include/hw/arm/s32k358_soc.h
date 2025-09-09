#ifndef HW_ARM_S32K358_SOC_H
#define HW_ARM_S32K358_SOC_H

#include "hw/char/s32k358_lpuart.h"  // LPUART device definition
#include "hw/can/s32_flexcan.h"      // FlexCAN device definition
#include "hw/sysbus.h"               // SysBusDevice base class
#include "qom/object.h"              // QEMU Object Model
#include "hw/arm/armv7m.h"           // ARM Cortex-M7 CPU
#include "hw/or-irq.h"               // IRQ helpers

/* -------------------- Type Declaration -------------------- */
#define TYPE_S32K358_SOC "s32k358-soc"
OBJECT_DECLARE_SIMPLE_TYPE(S32K358State, S32K358_SOC)

/* -------------------- Peripheral Counts -------------------- */
#define NUM_LPUART 8
#define NUM_FLEXCAN 2

/* -------------------- Base Addresses -------------------- */
#define LPUART_BASE_ADDR   0x40328000
#define FLEXCAN0_BASE_ADDR 0x40640000

/* -------------------- Memory Regions -------------------- */
#define SRAM_BASE_ADDR  0x20400000
#define SRAM_SIZE       0x00040000   /* 256 KB SRAM */

#define FLASH_BASE_ADDR 0x00400000   /* Flash start address */
#define FLASH_SIZE      0x00200000   /* 2 MB flash */

/* -------------------- IRQ Lines -------------------- */
#define LPUART0_IRQ 141
#define LPUART1_IRQ 142
#define LPUART2_IRQ 143
#define LPUART3_IRQ 144
#define LPUART4_IRQ 145
#define LPUART5_IRQ 146
#define LPUART6_IRQ 147
#define LPUART7_IRQ 148
#define FLEXCAN0_IRQ 87

/* -------------------- S32K358 SoC State Structure -------------------- */
struct S32K358State {
    SysBusDevice parent_obj;          /* Inherits from SysBusDevice */

    /* CPU */
    ARMv7MState armv7m;               /* Cortex-M7 CPU instance */

    /* LPUART peripherals */
    S32K358LPUARTState lpuart[NUM_LPUART];  /* 8 UART modules */

    /* FlexCAN peripherals */
    FlexCANState *flexcan[NUM_FLEXCAN];     /* 2 CAN nodes */

    /* Logical CAN bus connecting FlexCAN nodes */
    CanBus can_bus;

    /* Memory regions */
    MemoryRegion flash;               /* Flash memory */
    MemoryRegion flash_alias;         /* Alias for flash for convenient MMIO */
    MemoryRegion sram;                /* SRAM memory */

    /* System clock */
    Clock *sysclk;
};

#endif /* HW_ARM_S32K358_SOC_H */



