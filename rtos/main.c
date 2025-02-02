#include <avr/io.h>
#include "rtos.h"


task_handle_t menu_led;
timer_handle_t timer;

#ifndef RUN_TESTS
void INI();
void (*rtos_initialize_avr_device)(void) = INI;
#endif


void timer_function(void)
{
	PORTD ^= 0xff;
	timer_start_notify_function(&timer, 1000, timer_function);
}


TASK_my_task_t menu_led_f(void)
{
	static volatile uint8_t led = 0;
	DDRD = 0xff;

	TASK_do{
		PORTD = led++;
		condWait_task_delay(100);
		
		if(led == 0){
			timer_function();
			condWait_task_delay(4999);
			timer_stop(&timer);
		}
	
	}TASK_loop();
}

void INI()
{	
	task_setup(&menu_led, menu_led_f);
	task_start(&menu_led);
}





