/*=============================================================================
   Copyright (c) 2015-2016, The Linux Foundation. All rights reserved.
    
   Redistribution and use in source and binary forms, with or without
   modification, are permitted (subject to the limitations in the
   disclaimer below) provided that the following conditions are met:
    
      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
    
      * Redistributions in binary form must reproduce the above
        copyright notice, this list of conditions and the following
        disclaimer in the documentation and/or other materials provided
        with the distribution.
    
      * Neither the name of The Linux Foundation nor the names of its
        contributors may be used to endorse or promote products derived
        from this software without specific prior written permission.
    
   NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
   GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
   HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
   IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
                                                      
   qurt_interrupt_asm.S

   GENERAL DESCRIPTION 
     Default interrupt handler for FreeRTOS

               Interrupt Controller ASM FR

=============================================================================*/ 

/*==============================================================================
 FUNCTION qurt_interrupt_irq_handler

 DESCRIPTION
    Interrupt handler for FreeRTOS 
==============================================================================*/
#if defined (__GNUC__)

 .extern cpu_profiler_timer_isr
 
   //.section IntCont, "x"
   //.thumb
   .global  cpu_profiler_interrupt_irq_handler
   .type    cpu_profiler_interrupt_irq_handler, %function
   .align 3
   //.eabi_attribute 25, 1
   
cpu_profiler_interrupt_irq_handler:
       PUSH {R0-R3, LR}
       MRS R0, IPSR    // get the exception number
       LDR R1, =0x10   // subtract 16 since this is for external interrupts
       SUB R0, R0, R1 // get the number expected
       MOV R1, LR	   // need LR to figure out whether this interrupt pre-empted process (user) or master (interrupt) context blah
       MRS R2, MSP	   // need to save master context stack pointer
       MRS R3, PSP	   // need to save process context stack pointer
       BL cpu_profiler_timer_isr // branch to default irq handler
       POP {R0-R3, PC}
       .end

#endif /*__GNUC__ */

#if defined(__ARMCC_VERSION)

   IMPORT cpu_profiler_timer_isr
 
   AREA IntCont, CODE, READONLY
   EXPORT  cpu_profiler_interrupt_irq_handler
   PRESERVE8
cpu_profiler_interrupt_irq_handler
   PUSH {R0-R3, LR}
   MRS R0, IPSR    ; get the exception number
   LDR R1, = 0x10  ; subtract 16 since this is for external interrupts
   SUBS R0, R0, R1     ; get the number expected
   MOV R1, LR	   ; need LR to figure out whether this interrupt pre-empted process (user) or master (interrupt) context
   MRS R2, MSP	   ; need to save master context stack pointer
   MRS R3, PSP	   ; need to save process context stack pointer
   BL cpu_profiler_timer_isr ; branch to default irq handler
   POP {R0-R3, PC}
   END 
   
#endif /*__ARMCC_VERSION */
