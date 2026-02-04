#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "stdlib.h"
#include "task.h"
#include "generalDefines.h"
#include "platformTypes.h"

typedef struct
{
	void* taskList[TASK_COUNT];
	boolean running;
}SchedulerMem;

void schedulerInit();
void schedulerStart();
void schedulerAddTask(void* taskMem, uint8 taskIndex);

#endif // SCHEDULER_H
