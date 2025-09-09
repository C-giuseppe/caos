#include "qemu/osdep.h"           // QEMU OS-dependent utilities and macros
#include "hw/can/can_bus.h"       // Definitions for CAN bus structures and functions
#include "hw/can/s32_flexcan.h"   // FlexCAN driver header, includes flexcan_receive function
#include <string.h>               // Standard C library for memory operations (memset)
#include "qemu/log.h"             // QEMU logging functions (qemu_log)

// Initialize the CAN bus structure
// This function prepares the bus for use by resetting node count and clearing node references.
void can_bus_init(CanBus *bus) {
    bus->num_nodes = 0;                         // Set initial number of nodes to 0
    memset(bus->nodes, 0, sizeof(bus->nodes));  // Clear all node pointers to NULL
}

// Add a node to the CAN bus
// 'node' is a pointer to a CAN controller (e.g., FlexCANState)
// This function checks if the bus has room for another node (max 16 nodes)
void can_bus_add_node(CanBus *bus, void *node) {
    if (bus->num_nodes < 16) {                  // Check bus limit
        bus->nodes[bus->num_nodes++] = node;    // Add the node and increment the node count
    } else {
        qemu_log("CAN bus node limit reached!\n"); // Log an error if limit exceeded
    }
}

// Transmit a CAN frame from a sender node to all other nodes on the bus
// 'sender' is the node transmitting the frame
// 'frame' is the CAN message to transmit
void can_bus_transmit(CanBus *bus, void *sender, CanFrame *frame) {
    for (int i = 0; i < bus->num_nodes; i++) {          // Iterate through all nodes on the bus
        if (bus->nodes[i] != sender && bus->nodes[i] != NULL) { 
            // Skip the sender itself and any empty node slots

            FlexCANState *node = (FlexCANState *)bus->nodes[i]; // Cast void* to FlexCANState*
            flexcan_receive(node, frame);                       // Deliver the frame to the node
        }
    }
}

