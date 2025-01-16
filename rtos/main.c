#include <avr/io.h>
#include <avr/interrupt.h>
#include "rtos.h"
#include <avr/eeprom.h>
#include "event.h"
#include "timers.h"


void menu_led_f(void);



task_handle_t menu_led;


#ifndef RUN_TESTS
void INI();
void					(*rtos_initialize_avr_device)(void) = INI;
#endif


timer_handle_t timer;



TASK_my_task_t menu_led_f(void)
{


	TASK_do{	
		timer_start_notify_task(&timer, 100);

		condWait_task_delay(500);
	
	}TASK_loop();
}



void INI()
{	
	task_setup(&menu_led, menu_led_f);
	task_start(&menu_led);
}





