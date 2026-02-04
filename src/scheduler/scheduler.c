#include "scheduler.h"

static SchedulerMem* self = NULL;

static void scheduler();
static TaskCommonInterface* getInterface(void* taskptr);

void schedulerInit()
{
	self = (SchedulerMem*)malloc(sizeof(SchedulerMem));

	for(int i = 0; i < TASK_NUM; ++i)
	{
		self->taskList[i] = NULL;
	}
	self->running = FALSE;
}

void schedulerAddTask(void* taskMem, uint8 taskIndex)
{
	self->taskList[taskIndex] = taskMem;
}

static TaskCommonInterface* getInterface(void* taskptr)
{
	TaskCommonInterface* retval = NULL;

	if(taskptr != NULL)
	{
		retval = taskptr;
	}

	return retval;
}

static void scheduler()
{
	uint8 taskit = 0;

	while (self->running)
	{
		if(self->taskList[taskit] != NULL)
		{
			if (getInterface(self->taskList[taskit])->myState == TASK_INIT ||
					getInterface(self->taskList[taskit])->myState == TASK_RUN)
			{
				getInterface(self->taskList[taskit])->myCallback();
			}
		}
		++taskit;
		if(taskit == TASK_COUNT)
		{
			taskit = 0;
		}

	}
}

void schedulerStart()
{
	self->running = ON;
	scheduler(self);
}
