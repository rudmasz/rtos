/*
 * event_test.c
 *
 * Created: 02.10.2024 12:55:38
 *  Author: tom
 */ 
#ifdef RUN_TESTS
#include <avr/io.h>
#include <avr/interrupt.h>
#include "test.h"
#include "rtos.h"
#include "event.h"

#if (TEST_NUMBER_OF_TASKS < 2)
	#error "TEST_NUMBER_OF_TASKS sholud be at least 2"
#endif

event_action_t action;
uint8_t signal_src;
uint8_t signal_mask;



static void test_task_1(void)
{
	condWait_event_action_wait(&action);
}

static void test_task_2(void)
{
	condWait_event_action_wait(&action);
}

static void test_task_signal(void)
{
	condWait_event_wait_signal(&signal_src, signal_mask, 1);
}

void event_test(void)
{
	#define check_add_task_as_action_listener(task_id, task_function)\
		test_rtos_add_task_to_scheduler(task_id, task_function);\
		test_rtos_task_call(task_id, FALSE);\
		TEST(test_rtos_task_handle(task_id)->state == WAIT_SEMA);\
		TEST(test_rtos_task_handle(task_id)->sleep_sema == &action)
		
	#define check_task_was_notified_about_action(task_id)\
		TEST(test_rtos_task_handle(task_id)->state == READY);\
		TEST(test_rtos_task_handle(task_id)->sleep_sema == NULL);\
		test_rtos_remove_task_from_scheduler(task_id)
/*
	#define check_add_task_as_action_listener(task_id, task_function)\
		test_rtos_add_task_to_scheduler(task_id, task_function);\
		test_rtos_set_as_current_running_task(task_id);\
		task_function();\
		TEST(test_rtos_task_handle(task_id)->state == WAIT_SEMA);\
		TEST(test_rtos_task_handle(task_id)->sleep_sema == &action)
	
	#define check_task_was_notified_about_action(task_id)\
		TEST(test_rtos_task_handle(task_id)->state == READY);\
		TEST(test_rtos_task_handle(task_id)->sleep_sema == NULL);\
		test_rtos_remove_task_from_scheduler(task_id)*/
		
/****** EVENT INIT ******/
	event_action_init(&action);
	TEST(action.count == 0);
	TEST(action.max_count == 2);
	
/****** ADD ACTION LISTENERS ******/	
	check_add_task_as_action_listener(0, test_task_1);
	check_add_task_as_action_listener(1, test_task_2);
	
	event_action_notify_listeners(&action);
	
	check_task_was_notified_about_action(0);
	check_task_was_notified_about_action(1);
	
/****** SIGNAL ******/
	test_rtos_add_task_to_scheduler(0, test_task_signal);
	signal_mask = 0x11;
	signal_src = 0x01;
	
	test_rtos_task_call(0, FALSE);
	TEST(test_rtos_task_handle(0)->state == SLEEP_TIMED);
	
	__task_refresh_delayed(1);
	signal_src = 0x33;
	test_rtos_task_call(0, FALSE);
	TEST(test_rtos_task_handle(0)->state == RUNNING);
	test_rtos_remove_task_from_scheduler(0);	

	#undef check_task_was_notified_about_action
	#undef check_add_task_as_action_listener
}

#endif