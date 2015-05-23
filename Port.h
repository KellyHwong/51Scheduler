/**
* @Filename:
* @Created:
* @Updated:
* @Author: Kelly Hwong
* @Description: Cooperative scheduler
*/
#ifndef _PORT_H
#define _PORT_H

#define SCH_REPORT_ERRORS

//不懂为什么这样做
#ifdef SCH_REPORT_ERRORS
#define Error_port P2
#endif

//claim port to be used
sbit LED_pin = P1^3;

#endif
