#include "taskDummy1.h"
#include <stdlib.h>
#include <stdio.h>
#include "Driver_Analog.h"
#include "board_manager.h"

static TaskDummy1Mem* self = NULL;

// Modify here!
static void idle();
static void pause();
static void running();


static void WaitForNextPhase();
static void taskDummy1();

// Modify here!
static const TaskTransition TaskDummy1Table[TASK_DUMMY1_NUM_STATE]=
{
		{S01_IDLE_DUMMY1, &idle},
		{S02_PAUSE, &pause},
		{S03_RUNNING, &running},
		{WAIT_FOR_NEXT_STATE_DUMMY1, &WaitForNextPhase}
};

// Modify here!
static void idle()
{
	self->lastStateOfMyTask = self->currentStateOfMyTask;
	self->currentStateOfMyTask = WAIT_FOR_NEXT_STATE_DUMMY1;
}

// Modify here!
static void pause()
{
	self->lastStateOfMyTask = self->currentStateOfMyTask;
	self->currentStateOfMyTask = WAIT_FOR_NEXT_STATE_DUMMY1;
}

// Modify here!
static void running()
{
	self->lastStateOfMyTask = self->currentStateOfMyTask;
	self->currentStateOfMyTask = WAIT_FOR_NEXT_STATE_DUMMY1;
}


static void WaitForNextPhase() // Modify here
{

	manage_uart_buffer();
	manage_incoming_messages();
	// static uint32_t counter = 0;
	// counter++;
	// if(counter >= 10000)
	// {
	// 	counter = 0;
	// 	manage_analog_module();
	// }

	switch (self->lastStateOfMyTask)
	{

	case S01_IDLE_DUMMY1 :
		self->lastStateOfMyTask = self->currentStateOfMyTask;
		self->currentStateOfMyTask = S02_PAUSE;
		break;
	case S02_PAUSE :
		self->lastStateOfMyTask = self->currentStateOfMyTask;
		self->currentStateOfMyTask = S03_RUNNING;
		break;
	case S03_RUNNING :
		self->lastStateOfMyTask = self->currentStateOfMyTask;
		self->currentStateOfMyTask = S01_IDLE_DUMMY1;
		break;

	}



}



static void InitEntry()
{

	init_driver_analog();
	Init_485();
	init_spi_device();

	

	self->lastStateOfMyTask = S01_IDLE_DUMMY1;
	self->currentStateOfMyTask = S01_IDLE_DUMMY1;

	self->myInterface.myState = TASK_RUN;

}

static void executeRun(void* selfptr) // modify here!
{
	TaskDummy1Table[self->currentStateOfMyTask].fn();
}


static void FinalExit()
{
	// tbd deallocate heap mem
}


static void taskDummy1()
{

	if(self->myInterface.myState == TASK_INIT)
	{
		InitEntry(self);
	}

	executeRun(self);
}


TaskDummy1Mem* dummyTask1Init() // modify here!
{
	self = (TaskDummy1Mem *) malloc( sizeof(TaskDummy1Mem) );
	memset(self, 0U, sizeof(TaskDummy1Mem));

	self->myInterface.myState = TASK_INIT;
	self->myInterface.myCallback = &taskDummy1;

	return self;
}


