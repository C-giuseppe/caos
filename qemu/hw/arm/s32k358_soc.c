#include "qemu/osdep.h"
#include "exec/address-spaces.h"      // Access to system memory regions
#include "hw/arm/s32k358_soc.h"       // S32K358 SoC structure and device type
#include "hw/qdev-clock.h"            // Clock API for QEMU devices
#include "chardev/char.h"             // QEMU character devices (UARTs)
#include "qemu/error-report.h"        // Error handling
#include "hw/arm/armv7m.h"            // ARMv7M CPU device definitions
#include "qapi/error.h"               // QAPI error handling
#include "sysemu/sysemu.h"            // System emulation utilities
#include "hw/misc/unimp.h"            // Placeholder for unimplemented hardware
#include "qemu/log.h"                 // Logging
#include "hw/can/can_bus.h"           // CAN bus infrastructure
#include "hw/can/s32_flexcan.h"       // FlexCAN devices

// Initialize the S32K358 SoC instance
static void s32k358_soc_initfn(Object *obj) {
    qemu_log("Initializing S32K358 SoC\n");

    S32K358State *s = S32K358_SOC(obj);

    /* Initialize ARM Cortex-M7 core as a child device */
    object_initialize_child(obj, "armv7m", &s->armv7m, TYPE_ARMV7M);

    /* Initialize 8 LPUART modules as child devices */
    for (int i = 0; i < NUM_LPUART; i++) {
        object_initialize_child(obj, "lpuart[*]", &s->lpuart[i], TYPE_S32K358_LPUART);
    }

    /* Initialize system clock input */
    s->sysclk = qdev_init_clock_in(DEVICE(s), "sysclk", NULL, NULL, 0);

    /* Initialize CAN bus data structure */
    can_bus_init(&s->can_bus);
}

// Realize (instantiate and map) the S32K358 SoC
static void s32k358_soc_realize(DeviceState *dev_soc, Error **errp) {
    qemu_log("Realizing S32K358 SoC\n");

    S32K358State *s = S32K358_SOC(dev_soc);
    MemoryRegion *system_memory = get_system_memory();
    DeviceState *dev;
    SysBusDevice *busdev;
    Error *err = NULL;

    /* Ensure system clock is connected */
    if (!clock_has_source(s->sysclk)) {
        error_setg(errp, "sysclk clock must be wired up by the board code");
        return;
    }

    /* Flash memory setup */
    memory_region_init_rom(&s->flash, OBJECT(dev_soc), "S32K358.flash", FLASH_SIZE, errp);
    if (err) {
        error_propagate(errp, err);
        return;
    }

    /* Alias for flash memory for convenience in MMIO */
    memory_region_init_alias(&s->flash_alias, OBJECT(dev_soc),
                             "S32K358.flash.alias", &s->flash, 0,
                             FLASH_SIZE);

    /* Add flash memory to system memory map */
    memory_region_add_subregion(system_memory, FLASH_BASE_ADDR, &s->flash);
    memory_region_add_subregion(system_memory, 0, &s->flash_alias);

    /* SRAM memory setup */
    memory_region_init_ram(&s->sram, NULL, "S32K358.sram", SRAM_SIZE, errp);
    if (err) {
        error_propagate(errp, err);
        return;
    }
    memory_region_add_subregion(system_memory, SRAM_BASE_ADDR, &s->sram);

    /* Realize ARM Cortex-M7 CPU */
    dev = DEVICE(&s->armv7m);
    qdev_prop_set_uint32(dev, "num-irq", 240);        // 240 IRQ lines
    qdev_prop_set_uint8(dev, "num-prio-bits", 4);     // 4 priority bits
    qdev_prop_set_string(dev, "cpu-type", ARM_CPU_TYPE_NAME("cortex-m7"));
    qdev_prop_set_bit(dev, "enable-bitband", true);   // Enable bit-banding
    qdev_connect_clock_in(dev, "cpuclk", s->sysclk);  // Connect CPU clock
    object_property_set_link(OBJECT(&s->armv7m), "memory", OBJECT(system_memory), &error_abort);

    if (!sysbus_realize(SYS_BUS_DEVICE(&s->armv7m), errp)) {
        return;
    }
    qemu_log("Realized ARMv7M CPU\n");

    /* Realize all LPUART instances and map them to MMIO */
    const int lpuart_irq[NUM_LPUART] = {
        LPUART0_IRQ, LPUART1_IRQ, LPUART2_IRQ, LPUART3_IRQ,
        LPUART4_IRQ, LPUART5_IRQ, LPUART6_IRQ, LPUART7_IRQ
    };

    for (int i = 0; i < NUM_LPUART; i++) {
        dev = DEVICE(&s->lpuart[i]);

        /* Connect chardev for UART I/O */
        qdev_prop_set_chr(dev, "chardev", serial_hd(i));

        if (!sysbus_realize(SYS_BUS_DEVICE(&s->lpuart[i]), errp)) {
            return;
        }

        busdev = SYS_BUS_DEVICE(dev);
        sysbus_mmio_map(busdev, 0, LPUART_BASE_ADDR + (i * 0x4000)); // MMIO base
        sysbus_connect_irq(busdev, 0, qdev_get_gpio_in(DEVICE(&s->armv7m), lpuart_irq[i]));
    }

    qemu_log("Realized UART\n");

    /* Initialize CAN bus again (just to ensure it's ready) */
    can_bus_init(&s->can_bus);

    /* Realize FlexCAN devices and map them to CAN bus */
    for (int i = 0; i < 2; i++) {
        DeviceState *dev_flex;
        SysBusDevice *sbdev;
        hwaddr base = 0x40024000 + i * 0x1000; // FlexCAN MMIO base addresses

        /* Create FlexCAN device */
        dev_flex = qdev_new(TYPE_S32K358_FLEXCAN);
        s->flexcan[i] = S32K358_FLEXCAN(dev_flex);

        /* Connect FlexCAN to the logical CAN bus */
        s->flexcan[i]->bus = &s->can_bus;

        sbdev = SYS_BUS_DEVICE(dev_flex);

        /* Realize device: initializes MMIO and registers node on CAN bus */
        sysbus_realize_and_unref(sbdev, errp);

        /* Map MMIO to correct address */
        sysbus_mmio_map(sbdev, 0, base);
    }
}

// Class initialization: sets realize callback
static void s32k358_soc_class_init(ObjectClass *klass, void *data) {
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = s32k358_soc_realize;
}

// Type information for S32K358 SoC
static const TypeInfo s32k358_soc_info = {
    .name = TYPE_S32K358_SOC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(S32K358State),
    .instance_init = s32k358_soc_initfn,    // Instance initialization
    .class_init = s32k358_soc_class_init,   // Class initialization
};

// Register S32K358 SoC type with QEMU
static void s32k358_soc_register_types(void) {
    type_register_static(&s32k358_soc_info);
}

// Initialize type registration at QEMU startup
type_init(s32k358_soc_register_types);


