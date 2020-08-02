#include <stm32g4xx_hal.h>
#include <stm32_hal_legacy.h>
#include  "Scheduler.hpp"

#define SHPR3 (*((volatile uint32_t*)0xE000ED20))

extern "C"
{
	volatile uint32_t **SDOS_StackPtr;
	uint32_t __attribute__((section(".ccmram"))) StackRAM[64 * 4];
}

extern "C" void SDOS_Tick(void)
{
	HAL_IncTick();
}

Thread::Thread()
{
	AttachedTask = 0;
}

Thread::~Thread()
{
	
}

Task::Task()
{
	Function = 0;
}

Task::~Task()
{
	
}

Thread Threads[4];
Task Tasks[4];

void Thread1(void)
{
	while (true)
	{
		if (Threads[0].AttachedTask != 0)
		{
			(*Threads[0].AttachedTask->Function)();
		}
	}
}

void Thread2(void)
{
	while (true)
	{
		if (Threads[1].AttachedTask != 0)
		{
			(*Threads[1].AttachedTask->Function)();
		}
	}
}

void Thread3(void)
{
	while (true)
	{
		if (Threads[2].AttachedTask != 0)
		{
			(*Threads[2].AttachedTask->Function)();
		}
	}
}

void Thread4(void)
{
	while (true)
	{
		if (Threads[3].AttachedTask != 0)
		{
			(*Threads[3].AttachedTask->Function)();
		}
	}
}

uint32_t StackOffset = 0;
void SetStack(uint32_t thread, uint32_t stack)
{
	for (uint32_t i = StackOffset; i < stack + StackOffset; i++)
	{
		StackRAM[0] = 0;
	}
	
	Threads[thread].Stack = &StackRAM[StackOffset + stack - 16]; //SP
	if (thread == 0)
		StackRAM[StackOffset + stack - 2] = (uint32_t)&Thread1; //PC
	else if (thread == 1)
		StackRAM[StackOffset + stack - 2] = (uint32_t)&Thread2; //PC
	else if (thread == 2)
		StackRAM[StackOffset + stack - 2] = (uint32_t)&Thread3; //PC
	else
		StackRAM[StackOffset + stack - 2] = (uint32_t)&Thread4; //PC
	StackRAM[StackOffset + stack - 1] = 0x01000000; //xPSR
	StackOffset += stack;
}

uint32_t TaskOffset = 0;
void AddTask(void(*task)(void))
{
	Tasks[TaskOffset].Function = task;
	TaskOffset++;
}

volatile bool SetHigh = false;

void Task1()
{
	SetHigh = !SetHigh;
	HAL_Delay(500);
}

void Task2()
{
	if (SetHigh == true)
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
}

void Task3()
{
	__NOP();
}

void Task4()
{
	__NOP();
}

uint32_t curThread = 0;
void SDOS_Scheduler(void)
{
	curThread++;
	if (curThread >= 4)
		curThread = 0;
	
	SDOS_StackPtr = &Threads[curThread].Stack;
}

int main(void)
{
	HAL_Init();
	__disable_irq();
	uint32_t MillisPrescaler = SystemCoreClock / 1000;
	
	__GPIOB_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.Pin = GPIO_PIN_8;

	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

	SetStack(0, 64);
	SetStack(1, 64);
	SetStack(2, 64);
	SetStack(3, 64);
	AddTask(&Task1);
	AddTask(&Task2);
	AddTask(&Task3);
	AddTask(&Task4);
	
	SysTick->CTRL = 0;
	SysTick->VAL = 0;
		
	//Check the programming manual, (SHPR3 PRI_15)
	SHPR3 = (SHPR3 & 0x00FFFFFF) | 0xE0000000;
	SysTick->LOAD = (1 * MillisPrescaler) - 1;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
	
	SDOS_StackPtr = &Threads[0].Stack;
	
	Threads[0].AttachedTask = &Tasks[0];
	Threads[1].AttachedTask = &Tasks[1];
	Threads[2].AttachedTask = &Tasks[2];
	Threads[3].AttachedTask = &Tasks[3];
	
	SDOS_Setup();
	
	for (;;)
	{
		__NOP();
	}
}
