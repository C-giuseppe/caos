#include "qemu/osdep.h"
#include "hw/char/s32k358_lpuart.h"   // LPUART state definition
#include "hw/irq.h"                    // IRQ API
#include "hw/qdev-properties.h"        // Device properties
#include "hw/qdev-properties-system.h"
#include "qemu/log.h"
#include "qemu/module.h"

/* Debug printing macro */
// LPUART_ERR_DEBUG controls the debug verbosity
#ifndef LPUART_ERR_DEBUG
#define LPUART_ERR_DEBUG 0
#endif

// Debug print macros with optional level filtering
#define DB_PRINT_L(lvl, fmt, args...) do { \
    if (LPUART_ERR_DEBUG >= lvl) { \
        qemu_log("%s: " fmt, __func__, ## args); \
    } \
} while (0)

#define DB_PRINT(fmt, args...) DB_PRINT_L(1, fmt, ## args)

/* -------------------- Character Device Handlers -------------------- */

// Check if the LPUART can receive data (used by chardev frontends)
static int s32k358_lpuart_can_receive(void *opaque) {
    S32K358LPUARTState *s = opaque;
    return !(s->stat & LPUART_STAT_RDRF); // 1 if RX register is empty
}

// Handle data received from chardev backend
static void s32k358_lpuart_receive(void *opaque, const uint8_t *buf, int size) {
    S32K358LPUARTState *s = opaque;

    if (!(s->ctrl & LPUART_CTRL_RE)) {
        DB_PRINT("Receiver not enabled; dropping data.\n");
        return;
    }

    s->data = *buf;                  // Store received byte in DATA register
    s->stat |= LPUART_STAT_RDRF;     // Set "Receive Data Register Full" flag
    qemu_set_irq(s->irq, 1);         // Trigger interrupt to CPU
}

/* -------------------- Memory-mapped register access -------------------- */

// Read handler for LPUART MMIO registers
static uint64_t s32k358_lpuart_read(void *opaque, hwaddr addr, unsigned size) {
    S32K358LPUARTState *s = opaque;

    switch (addr) {
    case LPUART_BAUD:
        return s->baud;
    case LPUART_STAT:
        return s->stat;
    case LPUART_CTRL:
        return s->ctrl;
    case LPUART_DATA:
        s->stat &= ~LPUART_STAT_RDRF; // Clear RX flag after read
        return s->data;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "[lpuart] - Invalid read offset: 0x%" HWADDR_PRIx "\n", addr);
        return 0;
    }
}

// Write handler for LPUART MMIO registers
static void s32k358_lpuart_write(void *opaque, hwaddr addr, uint64_t val, unsigned size) {
    S32K358LPUARTState *s = opaque;

    switch (addr) {
    case LPUART_BAUD:
        s->baud = val;
        break;
    case LPUART_CTRL:
        s->ctrl = val;
        if (s->ctrl & LPUART_CTRL_TE) {
            s->stat |= LPUART_STAT_TDRE; // Transmit Data Register Empty
        }
        break;
    case LPUART_DATA:
        if (!(s->ctrl & LPUART_CTRL_TE)) {
            DB_PRINT("Transmitter not enabled; dropping data.\n");
            return;
        }
        // Send data to all connected chardev frontends
        qemu_chr_fe_write_all(&s->chr, (uint8_t *)&val, 1);
        s->stat |= LPUART_STAT_TDRE; // Set TDRE after sending
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "Invalid write offset: 0x%" HWADDR_PRIx "\n", addr);
    }
}

// MemoryRegionOps structure to define read/write access for MMIO
static const MemoryRegionOps s32k358_lpuart_ops = {
    .read = s32k358_lpuart_read,
    .write = s32k358_lpuart_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

/* -------------------- Device properties -------------------- */
static Property s32k358_lpuart_properties[] = {
    DEFINE_PROP_CHR("chardev", S32K358LPUARTState, chr), // Connect LPUART to QEMU chardev
    DEFINE_PROP_END_OF_LIST(),
};

/* -------------------- Initialization and realization -------------------- */

// Initialize instance: setup IRQ and MMIO
static void s32k358_lpuart_init(Object *obj)
{
    S32K358LPUARTState *s = S32K358_LPUART(obj);

    sysbus_init_irq(SYS_BUS_DEVICE(obj), &s->irq);  // Create IRQ line

    memory_region_init_io(&s->mmio, obj, &s32k358_lpuart_ops, s, "s32k358-lpuart", 0x20);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->mmio); // Map MMIO
}

// Realize instance: configure registers and connect chardev
static void s32k358_lpuart_realize(DeviceState *dev, Error **errp) {
    S32K358LPUARTState *s = S32K358_LPUART(dev);

    // Initialize registers
    s->ctrl = 0;
    s->stat = 0;
    s->baud = 0x1A0; // Example default baud rate
    s->data = 0;

    // Setup chardev handlers for RX/TX
    qemu_chr_fe_set_handlers(&s->chr,
                             s32k358_lpuart_can_receive, // Can receive?
                             s32k358_lpuart_receive,     // Receive callback
                             NULL, NULL,
                             s, NULL, true);
}

/* -------------------- Class and type registration -------------------- */

// Class init: assign realize function and properties
static void s32k358_lpuart_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = s32k358_lpuart_realize;
    device_class_set_props(dc, s32k358_lpuart_properties);
}

// Type info structure
static const TypeInfo s32k358_lpuart_info = {
    .name = TYPE_S32K358_LPUART,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(S32K358LPUARTState),
    .instance_init = s32k358_lpuart_init,
    .class_init = s32k358_lpuart_class_init,
};

// Register the LPUART type with QEMU
static void s32k358_lpuart_register_types(void) {
    type_register_static(&s32k358_lpuart_info);
}

// Initialize type registration at QEMU startup
type_init(s32k358_lpuart_register_types);



