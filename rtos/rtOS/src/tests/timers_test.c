/*
 * timers_test.c
 *
 * Created: 09.01.2025 15:57:47
 *  Author: tom
 */ 
#ifdef RUN_TESTS
#include <avr/io.h>
#include <string.h>
#include <avr/interrupt.h>
#include "test.h"
#include "rtos.h"

static timer_handle_t t1, t2, t3;
static uint8_t t2_timer_reset, t3_timer_reset;

//notify function
void timer_t2_notify(void)
{
	t2_timer_reset = TRUE;
	timer_start_notify_function(&t2, 0x2E624, timer_t2_notify);
}

//notify task
__attribute__((noinline))void task_timer(void)
{
	if(t3_timer_reset == FALSE){
		t3_timer_reset = TRUE;
		timer_start_notify_task(&t3, 0x3E623);
		condWait_task_infinite_sleep();
	}else{
		timer_start_notify_task(&t3, 200);
		task_delete();
	}
}


void timers_test(void)
{
	uint32_t time1, time2, time3;
	//check time to clock ticks change
	TEST(__timer_ms_to_ticks_16bits(1) == 1);
	TEST(__timer_ms_to_ticks_16bits(8) == 8);
	TEST(__timer_ms_to_ticks_16bits(32) == 32);
	TEST(__timer_ms_to_ticks_16bits(128) == 127);
	TEST(__timer_ms_to_ticks_16bits(512) == 508);
	TEST(__timer_ms_to_ticks_16bits(2048) == 2034);
	TEST(__timer_ms_to_ticks_16bits(6993) == 6944);
	TEST(__timer_ms_to_ticks_16bits(58542) == 58131);
	
	//check timer counter settings
	timer_start(&t1, 58542);
	TEST(t1.TCNT == 58131);
	
	timer_start(&t1, 2162687832);
	TEST(t1.TCNT == 2147483647);
	
	timer_start(&t1, 1000000000);
	TEST(t1.TCNT == 992969698);
	
	timer_start(&t1, 463129181);
	TEST(t1.TCNT == 459873244);
	
	time1 = 0x1E625;
	time2 = 0x2E624;
	time3 = 0x3E623;
	
	timer_start(&t1, time1);
	timer_start_notify_function(&t2, time2, timer_t2_notify);
	test_rtos_add_task_to_scheduler(0, task_timer);
	test_rtos_task_call(0, FALSE);
	
	time1 = t1.TCNT;
	time2 = t2.TCNT;
	time3 = t3.TCNT;
	
	for(uint8_t i=0; i<6; i++){
		__timer_refresh_timers(13000);
		time1 -= 13000;
		time2 -= 13000;
		time3 -= 13000;
	}
	TEST(timer_get_time(&t1) == time1);
	TEST(timer_get_time(&t2) == time2);
	TEST(timer_get_time(&t3) == time3);
	
	__timer_refresh_timers(65535);
	time2 -= 65535;
	time3 -= 65535;
	TEST(timer_get_time(&t1) == 0);
	TEST(timer_get_time(&t2) == time2);
	TEST(timer_get_time(&t3) == time3);

	__timer_refresh_timers(65535);	
	time3 -= 65535;
	TEST(t2_timer_reset == TRUE);
	TEST(timer_get_time(&t2) != 0);
	TEST(timer_get_time(&t3) == time3);
	
	__timer_refresh_timers(65535);	
	TEST(task_get_state(test_rtos_task_handle(0)) == READY);
	test_rtos_task_call(0, TRUE);
	TEST(timer_get_time(&t3) == 0);
	test_rtos_remove_task_from_scheduler(0);
	
	
	t2_timer_reset = FALSE;
	timer_stop_Notify(&t2);
	TEST(t2_timer_reset == TRUE);
	timer_stop(&t2);
	TEST(timer_get_time(&t2) == 0);
	
}




#endif