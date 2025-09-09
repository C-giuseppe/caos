#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "lpuart.h"
#include "flexcan.h"
#include <stdio.h>

/* Task periodicity in milliseconds */
#define UART_TASK_DELAY_MS 500
#define CAN_TASK_DELAY_MS 1000

/* Global objects */
CanBus can_bus;
FlexCANState flexcan0;
FlexCANState flexcan1;
SemaphoreHandle_t print_mutex;

/* 
 * Thread-safe printing via UART
 * - Ensures only one task prints at a time
 */
void my_printf_safe(const char *str) {
    if (xSemaphoreTake(print_mutex, portMAX_DELAY) == pdTRUE) {
        my_printf(str);
        xSemaphoreGive(print_mutex);
    }
}

/* 
 * CAN receive callback for FlexCAN1
 * - Called whenever a frame is received
 * - Prints frame ID and data
 */
void can_receive_callback(FlexCANState *s, CanFrame *frame) {
    (void)*s;  /* Unused */
    char buf[64];

    sprintf(buf, "Received CAN1 frame: ID=%03lX Data=", frame->id);
    my_printf_safe(buf);

    for (int i = 0; i < frame->dlc; i++) {
        sprintf(buf, "%02X ", frame->data[i]);
        my_printf_safe(buf);
    }

    my_printf_safe("\r\n");
}

/* UART Task: prints a message periodically */
void uart_task(void *pvParameters) {
    (void)pvParameters;
    while (1) {
        my_printf("UART Task running...\r\n");
        vTaskDelay(pdMS_TO_TICKS(UART_TASK_DELAY_MS));
    }
}

/* CAN0 Task: sends frames periodically */
void can0_task(void *pvParameters) {
    (void)pvParameters;
    uint8_t counter = 0;

    while (1) {
        CanFrame frame;
        frame.id = 0x123;
        frame.dlc = 2;
        frame.data[0] = counter++;
        frame.data[1] = counter++;

        char buf[32];
        my_printf("CAN0 sent frame: ");
        for (int i = 0; i < frame.dlc; i++) {
            sprintf(buf, "%02X ", frame.data[i]);
            my_printf_safe(buf);
        }
        my_printf("\r\n");

        /* Transmit frame on the logical CAN bus */
        can_bus_transmit(&can_bus, &flexcan0, &frame);

        vTaskDelay(pdMS_TO_TICKS(CAN_TASK_DELAY_MS));
    }
}

/* CAN1 Task: receives frames from its RX buffer */
void can1_task(void *pvParameters) {
    (void)pvParameters;
    uint32_t received_count = 0;

    while (1) {
        CanFrame frame;

        /* Check if a frame is available */
        if (flexcan_receive(&flexcan1, &frame)) {
            received_count++;
            char buf[64];
            sprintf(buf, "CAN1 Task: frame received count=%lu\r\n", received_count);
            my_printf_safe(buf);
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/* Main function */
int main(void) {
    /* Initialize UART and printing mutex */
    lpuart_init();
    print_mutex = xSemaphoreCreateMutex();

    /* Initialize CAN nodes and bus */
    flexcan_init(&flexcan0);
    flexcan_init(&flexcan1);
    can_bus_init(&can_bus);

    /* Connect nodes to the bus */
    flexcan0.bus = &can_bus;
    flexcan1.bus = &can_bus;
    can_bus_add_node(&can_bus, &flexcan0);
    can_bus_add_node(&can_bus, &flexcan1);

    /* Set callback for CAN1 reception */
    flexcan1.receive_callback = can_receive_callback;

    /* Create FreeRTOS tasks */
    xTaskCreate(uart_task, "UART Task", 256, NULL, 1, NULL);
    xTaskCreate(can0_task, "CAN0 Task", 256, NULL, 1, NULL);
    xTaskCreate(can1_task, "CAN1 Task", 256, NULL, 2, NULL);

    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    /* Should never reach here */
    while (1);
}
