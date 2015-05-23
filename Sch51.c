/**
* @Filename:
* @Created:
* @Updated:
* @Author: Kelly Hwong
* @Description: Cooperative scheduler
*/
#include "Main.h"
#include "Port.h"
#include "Sch51.h"

/** Public variables */
//The array of tasks
sTask SCH_tasks_G[SCH_MAX_TASKS];

//Used to display the error code
tByte Error_code_G = 0;

/** Private function prototypes */
static void SCH_Go_To_Sleep(void);

/** Private variables*/
//Keeps tracks of time since last error was recorded
static tWord Error_tick_count_G;
//The code of the last error
static tByte Last_error_code_G;

/** SCH_Dispatch_Tasks
* the dispatch funtion. When a task is due to run,
* SCH_Dispatch_Tasks() will run it.
* This function must be called repeatedly in the
* main loop.
*/
void SCH_Dispatch_Tasks(void)
{
    tByte Index;

    //Dispatches the next tast(if one is ready)
    for(Index = 0; Index < SCH_MAX_TASKS; Index++)
    {
        if(SCH_tasks_G[Index].RunMe > 0)
        {
            //Run the task
            (*SCH_tasks_G[Index].pTask)();
            //Reset the RunMe flag
            SCH_tasks_G[Index].RunMe -= 1;

            //Periodic tasks will automatically run again
            //But if this is a one-time task, remove it
            //from the tasks array
            if(SCH_tasks_G[Index].Period == 0)
            {
                SCH_Delete_Task(Index);
            }
        }
    }
    //Report system status
    SCH_Report_Status();
    //The scheduler enters idle mode
    SCH_Go_To_Sleep();
}

/*------------------------------------------------------------------*-

  SCH_Add_Task()

  Causes a task (function) to be executed at regular intervals
  or after a user-defined delay

  Fn_P   - The name of the function which is to be scheduled.
           NOTE: All scheduled functions must be 'void, void' -
           that is, they must take no parameters, and have
           a void return type.

  DELAY  - The interval (TICKS) before the task is first executed

  PERIOD - If 'PERIOD' is 0, the function is only called once,
           at the time determined by 'DELAY'.  If PERIOD is non-zero,
           then the function is called repeatedly at an interval
           determined by the value of PERIOD (see below for examples
           which should help clarify this).


  RETURN VALUE:

  Returns the position in the task array at which the task has been
  added.  If the return value is SCH_MAX_TASKS then the task could
  not be added to the array (there was insufficient space).  If the
  return value is < SCH_MAX_TASKS, then the task was added
  successfully.

  Note: this return value may be required, if a task is
  to be subsequently deleted - see SCH_Delete_Task().

  EXAMPLES:

  Task_ID = SCH_Add_Task(Do_X,1000,0);
  Causes the function Do_X() to be executed once after 1000 sch ticks.

  Task_ID = SCH_Add_Task(Do_X,0,1000);
  Causes the function Do_X() to be executed regularly, every 1000 sch ticks.

  Task_ID = SCH_Add_Task(Do_X,300,1000);
  Causes the function Do_X() to be executed regularly, every 1000 ticks.
  Task will be first executed at T = 300 ticks, then 1300, 2300, etc.

-*------------------------------------------------------------------*/
tByte SCH_Add_Task(void (code * pFunction)(), 
                   const tWord DELAY, 
                   const tWord PERIOD)    
{
    tByte Index = 0;
   while ((SCH_tasks_G[Index].pTask != 0) && (Index < SCH_MAX_TASKS))
    {
        Index++;
    }
    if(SCH_MAX_TASKS==Index)
    {
        //Tast list is full
        //Set the global error variable
        Error_code_G = ERROR_SCH_TOO_MANY_TASKS;

        //Also return an error code
        return SCH_MAX_TASKS;
    }
    //If we're here, there must be a space in the task array
    SCH_tasks_G[Index].pTask = pFunction;
    SCH_tasks_G[Index].Delay = DELAY;
    SCH_tasks_G[Index].Period = PERIOD;
    SCH_tasks_G[Index].RunMe = 0;

    //return position of task, in case to delete it later
    //if we have to
    return Index;
}

/*------------------------------------------------------------------*-

  SCH_Delete_Task()

  Removes a task from the scheduler.  Note that this does
  *not* delete the associated function from memory:
  it simply means that it is no longer called by the scheduler.

  TASK_INDEX - The task index.  Provided by SCH_Add_Task().

  RETURN VALUE:  RETURN_ERROR or RETURN_NORMAL

-*------------------------------------------------------------------*/
bit SCH_Delete_Task(const tByte TASK_INDEX)
{
    bit Return_code;
    if(0==SCH_tasks_G[TASK_INDEX].pTask)
    {
        //Set the global error variable
        Error_code_G = ERROR_SCH_CANNOT_DELETE_TASK;
        //Also return a error code
        Return_code = RETURN_ERROR;
    }
    else
    {
        Return_code = RETURN_NORMAL;
    }
    SCH_tasks_G[TASK_INDEX].pTask = 0x0000;
    SCH_tasks_G[TASK_INDEX].Delay = 0;
    SCH_tasks_G[TASK_INDEX].Period = 0;
    SCH_tasks_G[TASK_INDEX].RunMe = 0;

    return Return_code;
}

/*------------------------------------------------------------------*-

  SCH_Report_Status()

  Simple function to display error codes.

  This version displays code on a port with attached LEDs:
  adapt, if required, to report errors over serial link, etc.

  Errors are only displayed for a limited period
  (60000 ticks = 1 minute at 1ms tick interval).
  After this the the error code is reset to 0.

  This code may be easily adapted to display the last
  error 'for ever': this may be appropriate in your
  application.

  See Chapter 10 for further information.

-*------------------------------------------------------------------*/
void SCH_Report_Status(void)
{
#ifdef SCH_REPORT_ERRORS
   // ONLY APPLIES IF WE ARE REPORTING ERRORS
   // Check for a new error code
   if (Error_code_G != Last_error_code_G)
      {
      // Negative logic on LEDs assumed
      Error_port = 255 - Error_code_G;

      Last_error_code_G = Error_code_G;

      if (Error_code_G != 0)
         {
         Error_tick_count_G = 60000;
         }
      else
         {
         Error_tick_count_G = 0;
         }
      }
   else
      {
      if (Error_tick_count_G != 0)
         {
         if (--Error_tick_count_G == 0)
            {
            Error_code_G = 0; // Reset error code
            }
         }
      }
#endif
}

void SCH_Go_To_Sleep()
{
   PCON |= 0x01;    // Enter idle mode (generic 8051 version)
   //51的话0x01是休眠模式，0x02是掉电模式

   // Entering idle mode requires TWO consecutive instructions
   // on 80c515 / 80c505 - to avoid accidental triggering
   //PCON |= 0x01;    // Enter idle mode (#1)
   //PCON |= 0x20;    // Enter idle mode (#2)
}
