#include "Main.h"
#include "Sch51.h"
#include "LED_flash.h"

extern sTask SCH_tasks_G[SCH_MAX_TASKS];
extern tByte Error_code_G;

void main(void)
   {
   // Set up the scheduler
   SCH_Init_T2();

   // Prepare for the 'Flash_LED' task
   LED_Flash_Init();

   // Add the 'Flash LED' task (on for ~1000 ms, off for ~1000 ms)
   // - timings are in ticks (50 ms tick interval)
   // (Max interval / delay is 65535 ticks)
   SCH_Add_Task(LED_Flash_Update, 0, 20);

   // Start the scheduler
   SCH_Start();

   while(1)
      {
      SCH_Dispatch_Tasks();
      }
   }

void SCH_Init_T2(void)
{
  tByte i;

   for (i = 0; i < SCH_MAX_TASKS; i++)
      {
      SCH_Delete_Task(i);
      }

   // Reset the global error variable
   // - SCH_Delete_Task() will generate an error code,
   //   (because the task array is empty)
   Error_code_G = 0;
// Now set up Timer 2
   // 16-bit timer function with automatic reload

   // Crystal is assumed to be 11.0592 MHz
   // Assume 12 osc cycles per timer increment
   // -> timer resolution is 0.000001085069444444 seconds (1.085)
   // The required Timer 2 overflow is 0.050 seconds (50 ms)
   // - this takes 46080 timer ticks
   // Reload value is 65536 - 46080 = 19456 (dec) = 0x4C00
   //FC66,1ms
   //
  T2CON = 0x04;
  T2MOD = 0x00;
  TH2 = 0x4C;
  TL2 = 0X00;
  RCAP2H = 0X4C;
  RCAP2L = 0X00;
  ET2 = 1;
  TR2 = 1;
}
void SCH_Start(void)
   {
   EA = 1;
   }

/*------------------------------------------------------------------*-

  SCH_Update

  This is the scheduler ISR.  It is called at a rate
  determined by the timer settings in SCH_Init().
  This version is triggered by Timer 2 interrupts:
  timer is automatically reloaded.

-*------------------------------------------------------------------*/
void SCH_Update(void) interrupt INTERRUPT_Timer_2_Overflow
   {
   tByte Index;

   TF2 = 0; // Clear flag

   // NOTE: calculations are in *TICKS* (not milliseconds)
   for (Index = 0; Index < SCH_MAX_TASKS; Index++)
      {
      // Check if there is a task at this location
      if (SCH_tasks_G[Index].pTask)
         {
         if (SCH_tasks_G[Index].Delay == 0)
            {
            // The task is due to run
            SCH_tasks_G[Index].RunMe = 1;  // Set the run flag

            if (SCH_tasks_G[Index].Period)
               {
               // Schedule periodic tasks to run again
               SCH_tasks_G[Index].Delay = SCH_tasks_G[Index].Period;
               }
            }
         else
            {
            // Not yet ready to run: just decrement the delay
            SCH_tasks_G[Index].Delay -= 1;
            }
         }
      }
   }
