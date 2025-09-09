/*
 * FreeRTOS V202212.00 — startup ridotto senza PIT, con SysTick
 */

#include "lpuart.h"

/* FreeRTOS interrupt handlers. */
extern void vPortSVCHandler( void );
extern void xPortPendSVHandler( void );
extern void xPortSysTickHandler( void );

/* (Opzionale) tuo handler CAN se in futuro userai IRQ */
void FLEXCAN0_Handler(void) __attribute__((weak, alias("Default_Handler")));

/* Exception handlers. */
static void HardFault_Handler( void ) __attribute__( ( naked ) );
static void Default_Handler( void ) __attribute__( ( naked ) );
void Reset_Handler( void ) __attribute__( ( naked ) );

extern int main( void );
extern uint32_t _estack;

/* Vector table.
   NOTA: gli indici IRQ variano tra famiglie. Qui lasciamo tutti Default_Handler.
   Quando deciderai di usare le IRQ CAN, metti l’entry giusta per FLEXCANx. */
const uint32_t* isr_vector[] __attribute__((section(".isr_vector"), used)) =
{
    (uint32_t*)&_estack,                 // [0] SP
    (uint32_t*)&Reset_Handler,           // [1] Reset
    (uint32_t*)&Default_Handler,         // [2] NMI
    (uint32_t*)&HardFault_Handler,       // [3] HardFault
    (uint32_t*)&Default_Handler,         // [4] MemManage
    (uint32_t*)&Default_Handler,         // [5] BusFault
    (uint32_t*)&Default_Handler,         // [6] UsageFault
    0, 0, 0, 0,                          // [7..10] Reserved
    (uint32_t*)&vPortSVCHandler,         // [11] SVCall
    (uint32_t*)&Default_Handler,         // [12] DebugMon
    0,                                   // [13] Reserved
    (uint32_t*)&xPortPendSVHandler,      // [14] PendSV
    (uint32_t*)&xPortSysTickHandler,     // [15] SysTick

    /* IRQs: tutto Default_Handler per ora */
    [16 ... 240] = (uint32_t*)&Default_Handler
    /* Quando userai l’IRQ CAN, sostituisci l’indice corretto con FLEXCAN0_Handler */
    /* Esempio (indice fittizio!):
       [IRQ_BASE_FOR_FLEXCAN0] = (uint32_t*)&FLEXCAN0_Handler,
    */
};

void Reset_Handler( void )
{
    main();
}

/* … identici ai tuoi … */
volatile uint32_t r0,r1,r2,r3,r12,lr,pc,psr;

__attribute__( ( used ) )
void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{
    r0 = pulFaultStackAddress[ 0 ];
    r1 = pulFaultStackAddress[ 1 ];
    r2 = pulFaultStackAddress[ 2 ];
    r3 = pulFaultStackAddress[ 3 ];
    r12 = pulFaultStackAddress[ 4 ];
    lr = pulFaultStackAddress[ 5 ];
    pc = pulFaultStackAddress[ 6 ];
    psr = pulFaultStackAddress[ 7 ];
    my_printf("Calling prvGetRegistersFromStack() from fault handler");
    for( ;; );
}

void Default_Handler( void )
{
    __asm volatile
    (
        ".align 8                                \n"
        " ldr r3, =0xe000ed04                    \n"
        " ldr r2, [r3, #0]                       \n"
        " uxtb r2, r2                            \n"
        "Infinite_Loop:                          \n"
        " b  Infinite_Loop                       \n"
        " .ltorg                                 \n"
    );
}

void HardFault_Handler( void )
{
    __asm volatile
    (
        ".align 8                                                   \n"
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, =prvGetRegistersFromStack                         \n"
        " bx r2                                                     \n"
        " .ltorg                                                    \n"
    );
}
