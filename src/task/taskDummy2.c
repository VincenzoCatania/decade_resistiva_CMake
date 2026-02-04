#include "taskDummy2.h"
#include <stdlib.h>
#include <stdio.h>

static TaskDummy2Mem* self = NULL;

// Modify here!
static void idle();
static void pause();
static void running();

static void taskDummy2();
static void WaitForNextPhase();

// Modify here!
static const TaskTransition TaskDummy2Table[TASK_DUMMY2_NUM_STATE] =
{
        {S01_IDLE_DUMMY2,   &idle},
        {S02_PAUSE_DUMMY2,  &pause},
        {S03_RUNNING_DUMMY2,&running},
        {WAIT_FOR_NEXT_STATE_DUMMY2, &WaitForNextPhase}
};

// Modify here!
static void idle()
{
    self->lastStateOfMyTask = self->currentStateOfMyTask;
    self->currentStateOfMyTask = WAIT_FOR_NEXT_STATE_DUMMY2;
}

// Modify here!
static void pause()
{
    self->lastStateOfMyTask = self->currentStateOfMyTask;
    self->currentStateOfMyTask = WAIT_FOR_NEXT_STATE_DUMMY2;
}

// Modify here!
static void running()
{
    self->lastStateOfMyTask = self->currentStateOfMyTask;
    self->currentStateOfMyTask = WAIT_FOR_NEXT_STATE_DUMMY2;
}

static void WaitForNextPhase() // Modify here
{
    switch (self->lastStateOfMyTask)
    {
        case S01_IDLE_DUMMY2 :
            self->lastStateOfMyTask = self->currentStateOfMyTask;
            self->currentStateOfMyTask = S02_PAUSE_DUMMY2;
            break;

        case S02_PAUSE_DUMMY2 :
            self->lastStateOfMyTask = self->currentStateOfMyTask;
            self->currentStateOfMyTask = S03_RUNNING_DUMMY2;
            break;

        case S03_RUNNING_DUMMY2 :
            self->lastStateOfMyTask = self->currentStateOfMyTask;
            self->currentStateOfMyTask = S01_IDLE_DUMMY2;
            break;
    }
}

static void InitEntry()
{
    // peripheralInit(); // modify here!
    // driverInit();     // modify here!

    self->lastStateOfMyTask = S01_IDLE_DUMMY2;
    self->currentStateOfMyTask = S01_IDLE_DUMMY2;

    self->myInterface.myState = TASK_RUN;
}

static void executeRun(void* selfptr) // modify here!
{
    TaskDummy2Table[self->currentStateOfMyTask].fn();
}

static void FinalExit()
{
    // tbd deallocate heap mem
}

static void taskDummy2()
{
    if (self->myInterface.myState == TASK_INIT)
    {
        InitEntry(self);
    }

    executeRun(self);
}

TaskDummy2Mem* dummyTask2Init() // modify here!
{
    self = (TaskDummy2Mem *) malloc( sizeof(TaskDummy2Mem) );
    memset(self, 0U, sizeof(TaskDummy2Mem));

    self->myInterface.myState = TASK_INIT;
    self->myInterface.myCallback = &taskDummy2;

    return self;
}
