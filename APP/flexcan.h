#ifndef FLEXCAN_H
#define FLEXCAN_H

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "semphr.h"

/* Maximum number of FlexCAN nodes on a single logical CAN bus */
#define MAX_CAN_NODES 2

/* Size of the circular receive buffer per node */
#define RX_BUFFER_SIZE 16

/*
 * CAN frame structure
 * - id: CAN identifier (11-bit or 29-bit, simplified to 32-bit)
 * - data: payload (max 8 bytes)
 * - dlc: data length code (number of valid bytes in data)
 */
typedef struct CanFrame {
    uint32_t id;
    uint8_t data[8];
    uint8_t dlc;
} CanFrame;

/* Forward declaration of FlexCANState for the bus */
struct FlexCANState;

/*
 * Logical CAN bus structure
 * - Maintains a list of FlexCAN nodes connected to this bus
 * - Used to broadcast frames to all nodes except the sender
 */
typedef struct CanBus {
    struct FlexCANState *nodes[MAX_CAN_NODES];
    int num_nodes;  /* Current number of connected nodes */
} CanBus;

/*
 * FlexCAN peripheral state
 * - Represents a single FlexCAN controller
 * - Holds pointer to logical CAN bus and optional receive callback
 * - Implements a circular RX buffer for incoming CAN frames
 * - Mutex protects concurrent access to the RX buffer
 */
typedef struct FlexCANState {
    CanBus *bus;    /* Pointer to the shared logical CAN bus */

    /* Callback function called on frame reception (optional) */
    void (*receive_callback)(struct FlexCANState *s, CanFrame *frame);

    /* Circular RX buffer */
    CanFrame rx_buffer[RX_BUFFER_SIZE];
    uint8_t rx_head;   /* Head index for writing */
    uint8_t rx_tail;   /* Tail index for reading */
    SemaphoreHandle_t rx_mutex;  /* Mutex for thread-safe buffer access */
} FlexCANState;

/* Function prototypes */

/* Initialize a FlexCAN peripheral instance */
void flexcan_init(FlexCANState *s);

/* Initialize a logical CAN bus */
void can_bus_init(CanBus *bus);

/* Add a FlexCAN node to a CAN bus */
void can_bus_add_node(CanBus *bus, FlexCANState *node);

/* Transmit a CAN frame on the bus to all nodes except the sender */
void can_bus_transmit(CanBus *bus, FlexCANState *sender, CanFrame *frame);

/* Read a CAN frame from the FlexCAN node's RX buffer */
bool flexcan_receive(FlexCANState *node, CanFrame *frame);

#endif /* FLEXCAN_H */


