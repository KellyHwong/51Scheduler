/**
* @Filename:
* @Created:
* @Updated:
* @Author: Kelly Hwong
* @Description: Cooperative scheduler
*/
#ifndef _SCH51_H
#define _SCH51_H

#include "Main.h"

//Store in DATA area
//Total 7 bytes for each task
typedef data struct
{
    //Pointer to the tast(must be 'void (void)')
    void (code * pTask)(void);
    //Delay(ticks) until the function will be run
    tWord Delay;
    //Interval(ticks) between subsequent runs
    tWord Period;
    //Incremented(by scheduler) when task is due to execute
    tByte RunMe;
} sTask;

//Function prototypes
void SCH_Dispatch_Tasks(void);
tByte SCH_Add_Task(void (code*)(void), const tWord, const tWord);
bit SCH_Delete_Task(const tByte);
void SCH_Report_Status(void);

//The maximum number of tasks at any one time
//during the execution of the program

//WARNING! Must be adjusted for each new project
#define SCH_MAX_TASKS   (5)

#endif
