#include "qemu/osdep.h"          // QEMU OS-dependent utilities
#include "hw/can/s32_flexcan.h"  // FlexCAN definitions (FlexCANState, flexcan_receive)
#include "qemu/log.h"            // QEMU logging utilities (qemu_log)

// Minimal MMIO read function for FlexCAN
// Currently, this is a stub that ignores the address and size and always returns 0.
static uint64_t flexcan_read(void *opaque, hwaddr addr, unsigned size)
{
    FlexCANState *s = opaque;  // Cast opaque pointer to FlexCANState
    (void)addr; (void)size;    // Suppress unused variable warnings
    return 0;                   // Stub: always return 0
}

// Minimal MMIO write function for FlexCAN
// Currently a stub: ignores address, value, and size
static void flexcan_write(void *opaque, hwaddr addr, uint64_t val, unsigned size)
{
    FlexCANState *s = opaque;
    (void)addr; (void)val; (void)size;
}

// Simulated reception of a CAN frame
// Logs the frame ID and DLC (Data Length Code) for demonstration purposes
void flexcan_receive(FlexCANState *s, CanFrame *frame)
{
    qemu_log("FlexCAN received frame ID=0x%03X DLC=%u\n", frame->id, frame->dlc);
}

// Memory-mapped I/O operations for FlexCAN
// Associates read/write functions with MMIO region
static const MemoryRegionOps flexcan_ops = {
    .read = flexcan_read,                 // Function to read from MMIO
    .write = flexcan_write,               // Function to write to MMIO
    .endianness = DEVICE_LITTLE_ENDIAN,   // FlexCAN uses little-endian format
    .impl = {
        .min_access_size = 4,            // Minimum allowed access size (bytes)
        .max_access_size = 4,            // Maximum allowed access size (bytes)
    },
};

// Realize (initialize) the FlexCAN device
// Called when the device is created in QEMU
static void flexcan_realize(DeviceState *dev, Error **errp)
{
    FlexCANState *s = S32K358_FLEXCAN(dev);  // Cast DeviceState to FlexCANState

    // Initialize MMIO region for the FlexCAN peripheral
    memory_region_init_io(&s->mmio, OBJECT(s), &flexcan_ops, s,
                          "flexcan-mmio", 0x1000);  // 4 KB MMIO

    // Connect the MMIO region to the SysBusDevice
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->mmio);

    // Register the FlexCAN node on a logical CAN bus, if a bus is assigned
    if (s->bus) {
        can_bus_add_node(s->bus, s);
    }
}

// Initialize the device class for FlexCAN
// Associates the realize function with the device class
static void flexcan_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = flexcan_realize;  // Set the realization callback
}

// Type information for the FlexCAN device
static const TypeInfo flexcan_info = {
    .name = TYPE_S32K358_FLEXCAN,         // Unique type name
    .parent = TYPE_SYS_BUS_DEVICE,        // Inherits from SysBusDevice
    .instance_size = sizeof(FlexCANState),// Size of the device state structure
    .class_init = flexcan_class_init,     // Class initialization callback
};

// Register the FlexCAN type with QEMU
static void flexcan_register_types(void)
{
    type_register_static(&flexcan_info);  // Registers the type as static
}

// Initialize the FlexCAN type at QEMU startup
type_init(flexcan_register_types);

