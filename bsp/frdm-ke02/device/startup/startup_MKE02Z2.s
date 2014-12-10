/* K64F startup ARM GCC
 * Purpose: startup file for Cortex-M4 devices. Should use with
 *   GCC for ARM Embedded Processors
 * Version: V1.2
 * Date: 15 Nov 2011
 *
 * Copyright (c) 2011, ARM Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the ARM Limited nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ARM LIMITED BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
    .syntax unified
    .arch armv7-m
    
/* Memory Model
   The HEAP starts at the end of the DATA section and grows upward.

   The STACK starts at the end of the RAM and grows downward.

   The HEAP and stack STACK are only checked at compile time:
   (DATA_SIZE + HEAP_SIZE + STACK_SIZE) < RAM_SIZE

   This is just a check for the bare minimum for the Heap+Stack area before
   aborting compilation, it is not the run time limit:
   Heap_Size + Stack_Size = 0x80 + 0x80 = 0x100
 */
    .section .stack
    .align 3
#ifdef __STACK_SIZE
    .equ    Stack_Size, __STACK_SIZE
#else
    .equ    Stack_Size, 0x200
#endif
    .globl    __StackTop
    .globl    __StackLimit
__StackLimit:
    .space    Stack_Size
    .size __StackLimit, . - __StackLimit
__StackTop:
    .size __StackTop, . - __StackTop

    .section .heap
    .align 3
#ifdef __HEAP_SIZE
    .equ    Heap_Size, __HEAP_SIZE
#else
    .equ    Heap_Size, 0x400
#endif
    .globl    __HeapBase
    .globl    __HeapLimit
__HeapBase:
    .space    Heap_Size
    .size __HeapBase, . - __HeapBase
__HeapLimit:
    .size __HeapLimit, . - __HeapLimit

    .section .vector_table,"a",%progbits
    .align 2
    .globl __isr_vector
__isr_vector:
    .long   __StackTop                  /* Top of Stack */
    .long   Reset_Handler               /* Reset Handler */
    .long   NMI_Handler                 /* NMI Handler                  */
    .long   HardFault_Handler           /* Hard Fault Handler           */
    .long   MemManage_Handler           /* MPU Fault Handler            */
    .long   BusFault_Handler            /* Bus Fault Handler            */
    .long   UsageFault_Handler          /* Usage Fault Handler          */
    .long   0                           /* Reserved                     */
    .long   0                           /* Reserved                     */
    .long   0                           /* Reserved                     */
    .long   0                           /* Reserved                     */
    .long   SVC_Handler                 /* SVCall Handler               */
    .long   DebugMon_Handler            /* Debug Monitor Handler        */
    .long   0                           /* Reserved                     */
    .long   PendSV_Handler              /* PendSV Handler               */
    .long   SysTick_Handler             /* SysTick Handler              */

    /* External Interrupts */
	.long	    Reserved16_IRQHandler  /* Reserved interrupt 16*/
	.long     Reserved17_IRQHandler  /* Reserved interrupt 17*/
	.long     Reserved18_IRQHandler  /* Reserved interrupt 18*/
	.long     Reserved19_IRQHandler  /* Reserved interrupt 19*/
	.long     Reserved20_IRQHandler  /* Reserved interrupt 20*/
	.long     FTMRH_IRQHandler  /* FTMRH command complete/read collision interrupt*/
	.long     LVD_LVW_IRQHandler	/* Low Voltage Detect, Low Voltage Warning*/
	.long     IRQ_IRQHandler  /* External interrupt*/
	.long     I2C0_IRQHandler  /* I2C0 interrupt*/
	.long     Reserved25_IRQHandler  /* Reserved interrupt 25*/
	.long     SPI0_IRQHandler  /* SPI0 interrupt*/
	.long     SPI1_IRQHandler  /* SPI1 interrupt*/
	.long     UART0_IRQHandler  /* UART0 status/error interrupt*/
	.long     UART1_IRQHandler  /* UART1 status/error interrupt*/
	.long     UART2_IRQHandler  /* UART2 status/error interrupt*/
	.long     ADC0_IRQHandler  /* ADC0 interrupt*/
	.long     ACMP0_IRQHandler  /* ACMP0 interrupt*/
	.long     FTM0_IRQHandler  /* FTM0 Single interrupt vector for all sources*/
	.long     FTM1_IRQHandler  /* FTM1 Single interrupt vector for all sources*/
	.long     FTM2_IRQHandler  /* FTM2 Single interrupt vector for all sources*/
	.long     RTC_IRQHandler  /* RTC overflow*/
	.long     ACMP1_IRQHandler  /* ACMP1 interrupt*/
	.long     PIT_CH0_IRQHandler	/* PIT CH0 overflow*/
	.long     PIT_CH1_IRQHandler	/* PIT CH1 overflow*/
	.long     KBI0_IRQHandler  /* Keyboard interrupt 0*/
	.long     KBI1_IRQHandler  /* Keyboard interrupt 1*/
	.long     Reserved42_IRQHandler  /* Reserved interrupt 42*/
	.long     ICS_IRQHandler  /* MCG interrupt*/
	.long     Watchdog_IRQHandler  /* WDOG Interrupt*/
	.long     Reserved45_IRQHandler  /* Reserved interrupt 45*/
	.long     Reserved46_IRQHandler  /* Reserved interrupt 46*/
	.long     Reserved47_IRQHandler  /* Reserved interrupt 47*/

            
    
    .size    __isr_vector, . - __isr_vector

    .section .text.Reset_Handler
    .thumb
    .thumb_func
    .align  2
    .globl   Reset_Handler
    .type    Reset_Handler, %function
Reset_Handler:
/*     Loop to copy data from read only memory to RAM. The ranges
 *      of copy from/to are specified by following symbols evaluated in
 *      linker script.
 *      __etext: End of code section, i.e., begin of data sections to copy from.
 *      __data_start__/__data_end__: RAM address range that data should be
 *      copied to. Both must be aligned to 4 bytes boundary.  */

disable_watchdog:
    /* unlock */
    ldr r1, =0x4005200e
    ldr r0, =0xc520
    strh r0, [r1]
    ldr r0, =0xd928
    strh r0, [r1]
    /* disable */
    ldr r1, =0x40052000
    ldr r0, =0x01d2
    strh r0, [r1]

    ldr    r1, =__etext
    ldr    r2, =__data_start__
    ldr    r3, =__data_end__

    subs   r3, r2
    ble    .Lflash_to_ram_loop_end

    movs    r4, 0
.Lflash_to_ram_loop:
    ldr    r0, [r1,r4]
    str    r0, [r2,r4]
    adds   r4, 4
    cmp    r4, r3
    blt    .Lflash_to_ram_loop
.Lflash_to_ram_loop_end:

    ldr   r0, =SystemInit
    blx   r0
    ldr   r0, =init_data_bss
    blx   r0    
    ldr   r0, =main
    bx    r0
    .pool
    .size Reset_Handler, . - Reset_Handler

    .text
/*    Macro to define default handlers. Default handler
 *    will be weak symbol and just dead loops. They can be
 *    overwritten by other handlers */
    .macro    def_default_handler    handler_name
    .align 1
    .thumb_func
    .weak    \handler_name
    .type    \handler_name, %function
\handler_name :
    b    .
    .size    \handler_name, . - \handler_name
    .endm

/* Exception Handlers */

    def_default_handler    NMI_Handler
    def_default_handler    HardFault_Handler
    def_default_handler    MemManage_Handler
    def_default_handler    BusFault_Handler
    def_default_handler    UsageFault_Handler
    def_default_handler    SVC_Handler
    def_default_handler    DebugMon_Handler
    def_default_handler    PendSV_Handler
    def_default_handler    SysTick_Handler
    def_default_handler    Default_Handler
    
    .macro    def_irq_default_handler    handler_name
    .weak     \handler_name
    .set      \handler_name, Default_Handler
    .endm

/* IRQ Handlers */
    def_irq_default_handler     Reserved16_IRQHandler
    def_irq_default_handler     Reserved17_IRQHandler
    def_irq_default_handler     Reserved18_IRQHandler
    def_irq_default_handler     Reserved19_IRQHandler
    def_irq_default_handler     Reserved20_IRQHandler
    def_irq_default_handler     FTMRH_IRQHandler
    def_irq_default_handler     LVD_LVW_IRQHandler
    def_irq_default_handler     IRQ_IRQHandler
    def_irq_default_handler     I2C0_IRQHandler
    def_irq_default_handler     Reserved25_IRQHandler
    def_irq_default_handler     SPI0_IRQHandler
    def_irq_default_handler     SPI1_IRQHandler
    def_irq_default_handler     UART0_IRQHandler
    def_irq_default_handler     UART1_IRQHandler
    def_irq_default_handler     UART2_IRQHandler
    def_irq_default_handler     ADC0_IRQHandler
    def_irq_default_handler     ACMP0_IRQHandler
    def_irq_default_handler     FTM0_IRQHandler
    def_irq_default_handler     FTM1_IRQHandler
    def_irq_default_handler     FTM2_IRQHandler
    def_irq_default_handler     RTC_IRQHandler
    def_irq_default_handler     ACMP1_IRQHandler
    def_irq_default_handler     PIT_CH0_IRQHandler
    def_irq_default_handler     PIT_CH1_IRQHandler
    def_irq_default_handler     KBI0_IRQHandler
    def_irq_default_handler     KBI1_IRQHandler
    def_irq_default_handler     Reserved42_IRQHandler
    def_irq_default_handler     ICS_IRQHandler
    def_irq_default_handler     Watchdog_IRQHandler
    def_irq_default_handler     Reserved45_IRQHandler
    def_irq_default_handler     Reserved46_IRQHandler
    def_irq_default_handler     Reserved47_IRQHandler
    def_irq_default_handler     DefaultISR
    
/* Flash protection region, placed at 0x400 */
    .text
    .thumb
    .align 2
    .section .kinetis_flash_config_field,"a",%progbits
kinetis_flash_config:
    .long 0xffffffff
    .long 0xffffffff
    .long 0xffffffff
    .long 0xfffffffe

    .end
