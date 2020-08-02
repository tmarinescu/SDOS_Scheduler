#ifndef _KERNEL_H_
#define _KERNEL_H_

extern "C"
{
	void SDOS_Tick(void);
	void SDOS_Setup(void);
	void SDOS_Scheduler(void);
}

class Task
{
public:
	void(*volatile Function)(void);
	Task();
	~Task();
};

class Thread
{
public:
	volatile uint32_t* Stack;
	volatile Task* AttachedTask;
	Thread();
	~Thread();
};

#endif