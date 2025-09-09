#ifndef HW_CAN_CAN_BUS_H
#define HW_CAN_CAN_BUS_H

#include <stdint.h>
#include <stdbool.h>

/* Forward declaration of FlexCANState
   Each CAN node (FlexCAN controller) will use this type */
typedef struct FlexCANState FlexCANState;

/* -------------------- CAN Frame -------------------- */
/* Represents a single CAN message/frame */
typedef struct CanFrame {
    uint32_t id;      // 11-bit or 29-bit CAN ID
    uint8_t data[8];  // Up to 8 bytes of payload
    uint8_t dlc;      // Data Length Code: number of bytes in 'data' (0-8)
} CanFrame;

/* -------------------- Logical CAN Bus -------------------- */
/* Represents a simple "virtual" CAN bus connecting multiple FlexCAN nodes */
typedef struct CanBus {
    void *nodes[16];  // Array of pointers to nodes participating on this bus
    int num_nodes;    // Number of nodes currently attached
} CanBus;

/* -------------------- Bus Functions -------------------- */

/* Initialize the CAN bus.
   Sets node count to zero and clears node array. */
void can_bus_init(CanBus *bus);

/* Add a node to the CAN bus.
   'node' is a pointer to a FlexCANState (or compatible structure). */
void can_bus_add_node(CanBus *bus, void *node);

/* Transmit a CAN frame on the bus.
   All nodes except the sender will receive the frame. */
void can_bus_transmit(CanBus *bus, void *sender, CanFrame *frame);

#endif /* HW_CAN_CAN_BUS_H */

