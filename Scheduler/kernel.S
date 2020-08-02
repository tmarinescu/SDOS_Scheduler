.data
orgStack: .word 0

.thumb
.syntax unified
.section .text, "ax"
.balign 4
.eabi_attribute Tag_ABI_align_preserved, 1

.global SDOS_Scheduler
.global SDOS_Tick
.global SysTick_Handler
.global SDOS_Setup
.extern SDOS_StackPtr

.thumb_func
SysTick_Handler:		@R0-R3, R12, LR, PC, xPSR already pushed by interrupt
	CPSID IF

	PUSH {R4-R11}		@Store unsaved registers

	LDR R0, =SDOS_StackPtr
	LDR R1, [R0]
	STR SP, [R1]		@Store current stack location

	LDR R1, =orgStack	
	LDR SP, [R1]		@Switch back to main stack

	PUSH {R0, LR}		@Store R0 for easy access again

	BL SDOS_Tick		@Handle tick things like HAL
	BL SDOS_Scheduler	@Run the scheduler
	
	POP {R0, LR}		@Get back the pointer

	LDR R1, [R0]		@SDOS_StackPtr pointer changed inside scheduler
	LDR SP, [R1]		@Restore stack 

	POP {R4-R11}		@Restore registers

	CPSIE IF
	BX LR

SDOS_Setup:
	CPSID IF

	LDR R0, =orgStack
	STR SP, [R0]			@Store the original stack

	LDR R0, =SDOS_StackPtr	@Double pointer
	LDR R1, [R0]
	LDR SP, [R1]			@Switch stack to the thread one

	POP {R0-R12}			@Stack starts at -16, pop the first 13 registers
	ADD SP, SP, #4			@Skip one place since LR is found in the PC index
	POP {LR}
	ADD SP, SP, #4			@Move the place to the xPSR value

	PUSH {R0-R1}			@In case our setup uses these registers
	LDR R0, =0xE000E018
	MOV R1, #0
	STR R1, [R0]			@Reset SysTick to zero
	POP {R0-R1}

	CPSIE IF
	BX LR

.align
.end

