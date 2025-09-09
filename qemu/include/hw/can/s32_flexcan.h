#ifndef HW_CAN_S32_FLEXCAN_H
#define HW_CAN_S32_FLEXCAN_H

#include "hw/sysbus.h"      // SysBusDevice base class
#include "hw/can/can_bus.h"  // Logical CAN bus simulation
#include "qapi/error.h"      // QEMU error reporting

/* -------------------- Type Declaration -------------------- */
#define TYPE_S32K358_FLEXCAN "s32k358-flexcan"
#define S32K358_FLEXCAN(obj) OBJECT_CHECK(FlexCANState, (obj), TYPE_S32K358_FLEXCAN)

/* -------------------- FlexCAN Device State -------------------- */
typedef struct FlexCANState {
    SysBusDevice parent_obj; /* Inherits from SysBusDevice */

    CanBus *bus;             /* Pointer to the logical CAN bus */
    MemoryRegion mmio;       /* MMIO region mapped to CPU address space */

    /* Simulated registers */
    uint32_t MCR;            /* Module Configuration Register */
    uint32_t CTRL;           /* Control Register */
    uint32_t TFR;            /* Transmit Frame Register */
    uint32_t RFR;            /* Receive Frame Register */
} FlexCANState;

/* -------------------- Bus Communication -------------------- */
/* Called by CanBus when a CAN frame is transmitted.
   The FlexCAN node receives the frame from the bus. */
void flexcan_receive(FlexCANState *s, CanFrame *frame);

#endif /* HW_CAN_S32_FLEXCAN_H */




