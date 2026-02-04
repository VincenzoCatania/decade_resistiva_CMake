#include "generalInclude.h"
#include "scheduler.h"
#include "task.h"

// modify all main!

static TaskDummy1Mem* dummyTask1 = NULL;
static TaskDummy2Mem* dummyTask2 = NULL;

int main(void)
{

	sysinit();
	
	schedulerInit();

	dummyTask1 = dummyTask1Init();
	dummyTask2 = dummyTask2Init();

	schedulerAddTask(dummyTask1, TASK_0_DUMMY1);
	schedulerAddTask(dummyTask2, TASK_1_DUMMY2);

	schedulerStart();

}


