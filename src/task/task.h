#ifndef TASK_H
#define TASK_H
#include "GeneralInclude.h"


typedef enum
{
	TASK_INIT = 0U,
	TASK_RUN,
	TASK_STOP,
	TASK_NUM
}TaskState;

typedef struct
{
	TaskState myState;
	pFunc myCallback;
}TaskCommonInterface;

typedef enum // modify here!
{
	S01_IDLE_DUMMY1=0U,
	S02_PAUSE=1U,
	S03_RUNNING=2U,
	WAIT_FOR_NEXT_STATE_DUMMY1 = 3U,
	TASK_DUMMY1_NUM_STATE
}StateTaskDummy1;

typedef enum // modify here!
{
	S01_IDLE_DUMMY2=0U,
	S02_PAUSE_DUMMY2=1U,
	S03_RUNNING_DUMMY2=2U,
	WAIT_FOR_NEXT_STATE_DUMMY2 = 3U,
	TASK_DUMMY2_NUM_STATE
}StateTaskDummy2;

typedef enum // modify here!
{
	TASK_0_DUMMY1 = 0U,
	TASK_1_DUMMY2 = 1U,
	TASK_COUNT
}TaskEnum;

typedef struct
{
	uint8 state;
	pFunc fn;
}TaskTransition;

// modify here!
typedef struct
{
	TaskCommonInterface myInterface;
	StateTaskDummy1 currentStateOfMyTask;
	StateTaskDummy1 lastStateOfMyTask;

}TaskDummy1Mem;

// modify here!
typedef struct
{
	TaskCommonInterface myInterface;
	StateTaskDummy2 currentStateOfMyTask;
	StateTaskDummy2 lastStateOfMyTask;
	uint32 snapSystick;
}TaskDummy2Mem;

#endif // TASK_H
