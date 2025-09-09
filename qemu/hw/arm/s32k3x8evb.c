#include "qemu/osdep.h"           // QEMU OS-dependent utilities
#include "qapi/error.h"           // QEMU error handling API
#include "hw/boards.h"            // QEMU board/machine definitions
#include "hw/qdev-properties.h"   // QEMU device property handling
#include "hw/qdev-clock.h"        // QEMU clock API
#include "qemu/error-report.h"    // Error reporting functions
#include "hw/arm/boot.h"          // ARM boot/firmware loading functions
#include "hw/arm/s32k358_soc.h"   // S32K358 SoC device definitions
#include "qemu/log.h"             // QEMU logging utilities

/* Define the main system clock frequency */
// According to S32K358 documentation, the default system clock (FIRC) runs at 48 MHz.
#define SYSCLK_FRQ 48000000ULL /* 48 MHz */

// Initialize the S32K3X8EVB board (MachineState)
// Sets up system clocks, creates SoC, and optionally loads firmware
static void s32k3x8evb_init(MachineState *machine)
{
    DeviceState *soc_dev; // Pointer to the SoC device
    Clock *sysclk;        // Pointer to the system clock object

    /* Create a fixed-frequency system clock */
    sysclk = clock_new(OBJECT(machine), "SYSCLK"); // Create a new clock object
    clock_set_hz(sysclk, SYSCLK_FRQ);              // Set its frequency to 48 MHz

    /* Create and initialize the SoC device */
    soc_dev = qdev_new(TYPE_S32K358_SOC);                 // Instantiate the SoC device
    object_property_add_child(OBJECT(machine), "soc", OBJECT(soc_dev)); // Attach to machine
    qdev_connect_clock_in(soc_dev, "sysclk", sysclk);    // Connect the system clock to the SoC
    qemu_log("SYSCLK connected with frequency: %u Hz\n", clock_get_hz(sysclk));

    /* Realize the SysBus device (initialize hardware emulation) */
    sysbus_realize_and_unref(SYS_BUS_DEVICE(soc_dev), &error_fatal); 
    qemu_log("Realized S32K358 SoC\n");

    /* Load firmware image into flash memory if specified */
    if (machine->kernel_filename) {
        // Load ARM Cortex-M7 firmware into the flash region
        armv7m_load_kernel(ARM_CPU(first_cpu), machine->kernel_filename, 0, FLASH_SIZE); 
    }
}

// Set up the MachineClass structure for S32K3X8EVB
// Defines machine description, initialization function, and valid CPU types
static void s32k3x8evb_machine_init(MachineClass *mc)
{
    // List of valid CPU types for this machine
    static const char *const valid_cpu_types[] = {
        ARM_CPU_TYPE_NAME("cortex-m7"), /* S32K358 uses Cortex-M7 */
        NULL
    };

    mc->desc = "S32K3X8EVB-Q289 Machine (Cortex-M7)"; // Human-readable description
    mc->init = s32k3x8evb_init;                         // Initialization callback
    mc->valid_cpu_types = valid_cpu_types;             // CPU types supported
}

// Macro to register the machine type with QEMU
DEFINE_MACHINE("s32k3x8evb", s32k3x8evb_machine_init);



