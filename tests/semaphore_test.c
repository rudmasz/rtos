/*
 * semaphore_test.c
 *
 * Created: 22.11.2024 12:43:08
 *  Author: tom
 */ 
#ifdef RUN_TESTS
#include <avr/io.h>
#include <string.h>
#include <avr/interrupt.h>
#include "test.h"
#include "rtos.h"

	
semaphore_t sem;

static void test_task_semaphore(void)
{
	condWait_semaphore_wait(&sem);
}

static void test_task_mutex(void)
{
	condWait_mutex_lock(&sem);
}


void semaphore_test(void)
{
	#define MAX_SEMAPHORE_CNT	4
	#if TEST_NUMBER_OF_TASKS < 4
		#error "for testing purposes, set TEST_NUMBER_OF_TASKS to a value of at least 4"
	#endif
	
	/****** SEMAPHORE INIT ******/
	semaphore_init(&sem, MAX_SEMAPHORE_CNT);
	
	/****** GET SEMAPHORE ******/
	//MAX_SEMAPHORE_CNT this is the number of resources that should be available
	for(uint8_t i=0; i<MAX_SEMAPHORE_CNT; i++)
		TEST(semaphore_wait(&sem) == TRUE);
	
	//no more available, should return FALSE
	TEST(semaphore_wait(&sem) == FALSE);
	
	/****** GET SEMAPHORE WITH TASK ******/
	//task should be added to waiting group, because there is no available resources
	for(uint8_t i = 0; i < 4; i++)	{
		test_rtos_add_task_to_scheduler(i, test_task_semaphore);
		test_rtos_task_call(i, FALSE);
		TEST(test_rtos_task_handle(i)->state == WAIT_SEMA);
		TEST(test_rtos_task_handle(i)->sleep_sema == &sem);
	}
	TEST(semaphore_is_pending_list_empty(&sem) == FALSE);
	
	//waiting list removal test
	semaphore_remove_from_pending_list(test_rtos_task_handle(1), &sem);	//remove one from the middle
	TEST(test_rtos_task_handle(0)->next_task == test_rtos_task_handle(2));	//check waiting list connection
	
	semaphore_remove_from_pending_list(test_rtos_task_handle(0), &sem);	//remove one from the beginning
	TEST(sem.head_pending_tasks_list == test_rtos_task_handle(2));	//check waiting list connection
	
	semaphore_remove_from_pending_list(test_rtos_task_handle(3), &sem);	//remove one from the end
	TEST(test_rtos_task_handle(2)->next_task == NULL);	//check waiting list connection
	
	//release one semaphore, then the task with id 2 should run automatically
	semaphore_signal(&sem);
	TEST(test_rtos_task_handle(2)->state == READY);
	TEST(test_rtos_task_handle(2)->sleep_sema == NULL);
	TEST(semaphore_is_pending_list_empty(&sem) == TRUE);
	
	for(uint8_t i = 0; i < 4; i++)
		test_rtos_remove_task_from_scheduler(i);
	
	/****** MUTEX ******/
	//initialize as mutex
	mutex_init(&sem);
	
	//mutex is unlocked, the task 0 should be able to get it
	test_rtos_add_task_to_scheduler(0, test_task_mutex);
	test_rtos_task_call(0, FALSE);
	
	TEST(test_rtos_task_handle(0)->head_mutexes_list == &sem);
	TEST(sem.owner == test_rtos_task_handle(0));
	TEST(mutex_check_access(&sem) == TRUE);
	
	//now mutex is locked, the task 1 should be added to waiting list
	test_rtos_add_task_to_scheduler(1, test_task_mutex);
	test_rtos_task_call(1, FALSE);
	
	TEST(test_rtos_task_handle(1)->head_mutexes_list == NULL);
	TEST(sem.owner != test_rtos_task_handle(1));
	TEST(mutex_check_access(&sem) == FALSE);
	TEST(test_rtos_task_handle(1)->state == WAIT_SEMA);
	TEST(test_rtos_task_handle(1)->sleep_sema == &sem);
	
	//task 1 shouldn't be able to unlock the mutex
	TEST(mutex_unlock(&sem) == -2);
	
	//task 0 should be able to unlock the mutex
	test_rtos_set_as_current_running_task(0);
	TEST(mutex_unlock(&sem) == 0);
	
	//task 0 has freed the mutex, so task 1 should automatically become the new owner of the mutex
	test_rtos_set_as_current_running_task(1);
	TEST(mutex_check_access(&sem) == TRUE);
	TEST(test_rtos_task_handle(1)->state == RUNNING);
	TEST(test_rtos_task_handle(1)->sleep_sema == NULL);
	
	//check removing from pending list
	test_rtos_task_call(0, TRUE);	//reset the pc as for semaphores the task context is saved with task_update_pc_addr_after_call()
	TEST(mutex_is_pending_list_empty(&sem) == FALSE);
	mutex_remove_from_pending_list(test_rtos_task_handle(0), &sem);
	TEST(mutex_is_pending_list_empty(&sem) == TRUE);

	test_rtos_remove_task_from_scheduler(0);
	test_rtos_remove_task_from_scheduler(1);
}

#endif

