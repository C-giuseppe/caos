#include "flexcan.h"
#include "lpuart.h"
#include <string.h>
#include "semphr.h"

/*
 * Initialize a FlexCAN peripheral instance
 * - Clears the bus pointer
 * - Clears receive callback
 * - Initializes the circular RX buffer and mutex
 */
void flexcan_init(FlexCANState *s) {
    s->bus = NULL;
    s->receive_callback = NULL;

    /* Circular RX buffer initialization */
    s->rx_head = 0;
    s->rx_tail = 0;
    s->rx_mutex = xSemaphoreCreateMutex();  /* FreeRTOS mutex to protect buffer access */
}

/*
 * Initialize a logical CAN bus
 * - Resets node count
 * - Clears node pointers
 */
void can_bus_init(CanBus *bus) {
    bus->num_nodes = 0;
    memset(bus->nodes, 0, sizeof(bus->nodes));
}

/*
 * Add a new FlexCAN node to the CAN bus
 * - Limits the number of nodes to MAX_CAN_NODES
 */
void can_bus_add_node(CanBus *bus, FlexCANState *node) {
    if (bus->num_nodes < MAX_CAN_NODES) {
        bus->nodes[bus->num_nodes++] = node;
    }
}

/*
 * Transmit a CAN frame on the logical bus
 * - Frame is delivered to all nodes except the sender
 * - Each receiver inserts the frame into its circular RX buffer
 * - Optional callback is called after insertion
 */
void can_bus_transmit(CanBus *bus, FlexCANState *sender, CanFrame *frame) {
    for (int i = 0; i < bus->num_nodes; i++) {
        if (bus->nodes[i] != sender && bus->nodes[i] != NULL) {
            FlexCANState *receiver = bus->nodes[i];

            /* Insert frame into the circular buffer (thread-safe) */
            if (xSemaphoreTake(receiver->rx_mutex, portMAX_DELAY) == pdTRUE) {
                receiver->rx_buffer[receiver->rx_head] = *frame;
                receiver->rx_head = (receiver->rx_head + 1) % RX_BUFFER_SIZE;
                xSemaphoreGive(receiver->rx_mutex);
            }

            /* Call receive callback if defined */
            if (receiver->receive_callback)
                receiver->receive_callback(receiver, frame);
        }
    }
}

/*
 * Read a CAN frame from a FlexCAN node's RX buffer
 * - Returns true if a frame was available
 * - Uses mutex to ensure thread-safe access
 */
bool flexcan_receive(FlexCANState *node, CanFrame *frame) {
    bool has_frame = false;
    if (xSemaphoreTake(node->rx_mutex, portMAX_DELAY) == pdTRUE) {
        if (node->rx_head != node->rx_tail) {
            *frame = node->rx_buffer[node->rx_tail];
            node->rx_tail = (node->rx_tail + 1) % RX_BUFFER_SIZE;
            has_frame = true;
        }
        xSemaphoreGive(node->rx_mutex);
    }
    return has_frame;
}
