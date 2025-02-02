/*
 * task_test.c
 *
 * Created: 04.12.2024 15:27:38
 *  Author: tom
 */ 
#ifdef RUN_TESTS
#include <avr/io.h>
#include <string.h>
#include <avr/interrupt.h>
#include "test.h"
#include "rtos.h"


extern task_handle_t *__task_ready_g;


static task_handle_t *task_list;



/********************* context and local variables save ********************/
static task_dynamic_handle_t *task_local_var;
static uint32_t local_task_var;


__attribute__((noinline)) void test_task_save_context_variables(void)
{
	struct my_data{
		union{
			struct{
				uint8_t val1;
				uint8_t val2;
				uint8_t val3;
				uint8_t val4;
			};
			uint32_t val1_4;
		};
	};

	TASK_init_dynamic_variables_pointer(my_data, sv);
	sv->val1_4 = local_task_var;
	task_update_pc_addr_before_call(task_this);
	local_task_var = sv->val1_4;
}

/********************* new task ********************/
static task_handle_t *task_new_dynamic;
static void test_task_new_task_destructor(task_handle_t *xtask);

static void test_task_create_new(void)
{
	task_new_dynamic = condWait_task_new(test_task_create_new, test_task_new_task_destructor);

	if(task_new_dynamic){
		task_start(task_new_dynamic);
		task_stop();
	}
}

static void test_task_new_task_destructor(task_handle_t *xtask)
{
	if(xtask == task_new_dynamic)
		task_new_dynamic = NULL;
}

/***************** interrupt *******************/
static void test_task_wait_for_irq(void)
{
	condWait_task_wait_irq(_IrqINT1);
}

/***************** delay *******************/
#define TEST_TASK_SLEEP_TIME 10

static void test_task_delay(void)
{
	condWait_task_delay(TEST_TASK_SLEEP_TIME);
}

/***************** join *******************/
#define TEST_TASK_JOIN_JOIN_CHILD								0
#define TEST_TASK_JOIN_STOP_CHILD								1
#define TEST_TASK_JOIN_DELETE_CHILD								2
#define TEST_TASK_JOIN_SLEEP_INFINITE_CHILD						3
#define TEST_TASK_JOIN_SLEEP_INFINITE_CHILD_WAKE_UP_PARENT		4

static task_handle_t *task_to_join;
static volatile uint8_t child_step;


__attribute__((noinline)) static void test_task_join_child(void)
{
	if(child_step == TEST_TASK_JOIN_JOIN_CHILD){
		condWait_task_join(task_to_join);
	
	}
	if(child_step == TEST_TASK_JOIN_STOP_CHILD){
		task_stop();
		
	}else if(child_step == TEST_TASK_JOIN_DELETE_CHILD){
		task_delete();
		
	}else if(child_step == TEST_TASK_JOIN_SLEEP_INFINITE_CHILD){
		condWait_task_infinite_sleep();
		
	}else if(child_step == TEST_TASK_JOIN_SLEEP_INFINITE_CHILD_WAKE_UP_PARENT){
		condWait_task_infinite_sleep_wup();
	}
}

static __attribute__((noinline)) void CALL_TASK(task_handle_t *task)
{
	__task_ready_g->state = READY;
	__task_ready_g = task;
	__task_ready_g->state = RUNNING;
	asm volatile(
	"push r28 \n\t"
	"push r29 \n\t"
	"in r28, __SP_L__ \n\t"
	"in r29, __SP_H__ \n\t"
	::);
	((void (*)(void))(task)->PC)();
	asm volatile(
	"pop r29 \n\t"
	"pop r28 \n\t"
	::);
}

void task_test(void)
{	
	#if TEST_NUMBER_OF_TASKS < 5
		#error "for testing purposes, set TEST_NUMBER_OF_TASKS to a value of at least 5"
	#endif
	
	
	uint8_t *temp;
	task_handle_t *task_ptr_1;
	
	/****** POP AND PUSH LIST ******/
	//push front
	for(uint8_t i = 0; i<TEST_NUMBER_OF_TASKS; i++){
		task_list_push_front(&task_list, test_rtos_task_handle(i));
		
		if(i){
			TEST(test_rtos_task_handle(i)->next_task == test_rtos_task_handle(i-1));
			TEST(test_rtos_task_handle(i-1)->prev_task == test_rtos_task_handle(i));
		}
	}
	TEST(task_list == test_rtos_task_handle(TEST_NUMBER_OF_TASKS-1));
	TEST(test_rtos_task_handle(TEST_NUMBER_OF_TASKS-1)->prev_task == NULL);
	TEST(test_rtos_task_handle(0)->next_task == NULL);
	
	//remove by item
	task_list_remove_by_item(&task_list, test_rtos_task_handle(1));
	TEST(test_rtos_task_handle(0)->prev_task == test_rtos_task_handle(2));
	TEST(test_rtos_task_handle(2)->next_task == test_rtos_task_handle(0));
	
	//pop back
	TEST(task_list_pop_back(&task_list) == test_rtos_task_handle(0));
	TEST(task_list_pop_back(&task_list) == test_rtos_task_handle(2));
	TEST(task_list_pop_back(&task_list) == test_rtos_task_handle(3));
	TEST(task_list_pop_back(&task_list) == test_rtos_task_handle(4));
	TEST(task_list == NULL);

	//push back
	for(uint8_t i = 0; i<TEST_NUMBER_OF_TASKS; i++){
		task_list_push_back(&task_list, test_rtos_task_handle(i));
		
		if(i){
			TEST(test_rtos_task_handle(i-1)->next_task == test_rtos_task_handle(i));
			TEST(test_rtos_task_handle(i)->prev_task == test_rtos_task_handle(i-1));
		}
	}
	TEST(task_list == test_rtos_task_handle(0));
	TEST(test_rtos_task_handle(0)->prev_task == NULL);
	TEST(test_rtos_task_handle(TEST_NUMBER_OF_TASKS-1)->next_task == NULL);
	
	//pop front
	for(uint8_t i = 0; i<TEST_NUMBER_OF_TASKS; i++){
		TEST(task_list_pop_front(&task_list) == test_rtos_task_handle(i));
	}
	TEST(task_list == NULL);
	
	/****** TASK OPERATION ******/
	//new task
	temp = heap_malloc(BOARD_heap_single_block_size*BOARD_heap_number_of_blocks); //allocate all free memory
	task_new_dynamic = task_new(test_task_create_new);
	TEST(task_new_dynamic == NULL);		//should be null, no free memory
	heap_free(temp);
	task_new_dynamic = task_new(test_task_create_new);	//now new task should be created
	TEST(task_get_function_address(task_new_dynamic) == (uint16_t)test_task_create_new);
	TEST(task_get_program_counter(task_new_dynamic) == (uint16_t)test_task_create_new);
	TEST(task_get_state(task_new_dynamic) == STOPPED);	
	task_delete(task_new_dynamic);
	
	//create new task in the other task
	task_ptr_1 = test_rtos_task_handle(0);	//get task handle from test tasks array
	task_setup(task_ptr_1, test_task_create_new);
	task_start(task_ptr_1);
	TEST(task_ptr_1->code_addr == (uint16_t)test_task_create_new);
	TEST(task_ptr_1->state == READY);
	temp = heap_malloc(BOARD_heap_single_block_size*BOARD_heap_number_of_blocks); //allocate all free memory
	CALL_TASK(task_ptr_1);		//call task function
	TEST(task_new_dynamic == NULL);
	TEST(task_ptr_1->state == WAIT_SEMA);
	heap_free(temp);	//free up memory
	TEST(task_ptr_1->state == READY);
	CALL_TASK(task_ptr_1);				//one more task function call
	TEST(task_ptr_1->state == STOPPED);	//this task should be deleted
	TEST(task_new_dynamic != NULL);		//new task should be created
	TEST(task_new_dynamic->destructor_f == test_task_new_task_destructor);
	task_delete(task_new_dynamic);		//task destructor should be called
	TEST(task_new_dynamic == NULL);		//should be null if the task was deleted correctly

	//waiting for an interrupt to be reported
	task_ptr_1 = test_rtos_task_handle(0);	//get task handle from test tasks array
	task_setup(task_ptr_1, test_task_wait_for_irq);
	task_start(task_ptr_1);
	CALL_TASK(task_ptr_1);
	TEST(task_ptr_1->state == INTERRUPT);						//the state should be INTERRUPT
	TEST(task_ptr_1->sleep_irq_num == _IrqINT1);
	TEST(task_check_if_any_is_waiting_for_irq() == TRUE);	
	rtos_irq_report(_IrqINT0);								//report an interrupt other than the one expected by the task
	__task_refresh_interrupted();							//normally this function will be called by the scheduler
	TEST(task_ptr_1->state == INTERRUPT);						//the state should still be INTERRUPT
	rtos_irq_report(_IrqINT1);								//report an expected interrupt
	__task_refresh_interrupted();							//normally this function will be called by the scheduler
	TEST(task_ptr_1->state == READY);						//now the task should be READY
	TEST(task_check_if_any_is_waiting_for_irq() == FALSE);
	task_delete(task_ptr_1);

	//delay task
	task_ptr_1 = test_rtos_task_handle(0);	//get task handle from test tasks array
	task_setup(task_ptr_1, test_task_delay);
	task_start(task_ptr_1);
	CALL_TASK(task_ptr_1);
	TEST(task_ptr_1->state == SLEEP_TIMED);					//the state should be SLEEP_TIMED	
	__task_refresh_delayed(TEST_TASK_SLEEP_TIME/2);			//normally this function will be called by the scheduler
	TEST(task_ptr_1->state == SLEEP_TIMED);					//the state should be still SLEEP_TIMED
	TEST(task_ptr_1->sleep_time < TEST_TASK_SLEEP_TIME);
	__task_refresh_delayed(TEST_TASK_SLEEP_TIME/2 + 1);		//normally this function will be called by the scheduler
	TEST(task_ptr_1->state == READY);						//now the task should be READY
	task_delete(task_ptr_1);
	
	//task join
	for(uint8_t i=0; i<5; i++){
		task_setup(test_rtos_task_handle(i), test_task_join_child);
	}
	task_start(test_rtos_task_handle(0));
	
	for(uint8_t i=0; i<3; i++){
		task_to_join = test_rtos_task_handle(i+1);
		CALL_TASK(test_rtos_task_handle(i));
		TEST(task_check_relationship(test_rtos_task_handle(i), test_rtos_task_handle(i+1)));
	}
	//try to join task which is currently joined by another task
	CALL_TASK(test_rtos_task_handle(4));
	TEST(task_check_relationship(test_rtos_task_handle(4), test_rtos_task_handle(3)) == FALSE);	//should be no relationship
	TEST(test_rtos_task_handle(4)->state == SLEEP_TIMED);	//task should be delayed 
	
	//stop task 3
	child_step = TEST_TASK_JOIN_STOP_CHILD;
	CALL_TASK(test_rtos_task_handle(3));	//now task 3 should be stopped and task 2 should be woken up
	TEST(test_rtos_task_handle(3)->state == STOPPED);
	TEST(test_rtos_task_handle(2)->state == READY);
	TEST(task_check_relationship(test_rtos_task_handle(2), test_rtos_task_handle(3)) == FALSE);	//should be no relationship
	
	//refresh task 4 which was trying to join task 3
	__task_refresh_delayed(1);			//normally this function will be called by the scheduler
	child_step = TEST_TASK_JOIN_JOIN_CHILD;
	TEST(test_rtos_task_handle(4)->state == READY);
	CALL_TASK(test_rtos_task_handle(4));
	TEST(task_check_relationship(test_rtos_task_handle(4), test_rtos_task_handle(3)));	//task 4 should be parent of task 3
	
	//sleep infinite without waking up parent task
	child_step = TEST_TASK_JOIN_SLEEP_INFINITE_CHILD;
	CALL_TASK(test_rtos_task_handle(3));	//now task 3 should sleep infinite and task 4 should not be woken up
	TEST(test_rtos_task_handle(3)->state == SLEEP_INFINITE);
	TEST(test_rtos_task_handle(4)->state == JOIN);
	TEST(task_check_relationship(test_rtos_task_handle(4), test_rtos_task_handle(3)));	//should be relationship
	task_delete(test_rtos_task_handle(4));
	
	//sleep infinite with waking up parent task
	child_step = TEST_TASK_JOIN_SLEEP_INFINITE_CHILD_WAKE_UP_PARENT;
	CALL_TASK(test_rtos_task_handle(2));	//now task 2 should sleep infinite and task 1 should be woken up
	TEST(test_rtos_task_handle(2)->state == SLEEP_INFINITE);
	TEST(test_rtos_task_handle(1)->state == READY);
	TEST(task_check_relationship(test_rtos_task_handle(1), test_rtos_task_handle(2)));	//should be relationship
	
	//delete task
	child_step = TEST_TASK_JOIN_DELETE_CHILD;
	CALL_TASK(test_rtos_task_handle(1));	//now task 1 and task 2 should be deleted, task 0 should be woken up
	TEST(test_rtos_task_handle(2)->state == STOPPED);
	TEST(test_rtos_task_handle(1)->state == STOPPED);
	TEST(test_rtos_task_handle(0)->state == READY);
	TEST(task_check_relationship(test_rtos_task_handle(1), test_rtos_task_handle(2)) == FALSE);	//should be no relationship
	TEST(task_check_relationship(test_rtos_task_handle(0), test_rtos_task_handle(1)) == FALSE);	//should be no relationship
	
	/****** SAVE TASK CONTEXT AND LOCAL VARIABLES ******/
	task_local_var = task_new(test_task_save_context_variables);
	TEST(task_get_program_counter((task_handle_t *)task_local_var) == (uint16_t)test_task_save_context_variables);
	task_start((task_handle_t *)task_local_var);
	local_task_var = 0x11223344;	//set the original value of the local variable
	CALL_TASK((task_handle_t *)task_local_var);
	TEST(local_task_var == 0x11223344);	//check if value is correct
	local_task_var = 0xffca1020;	//change value for the local variable
	CALL_TASK((task_handle_t *)task_local_var);	//call task 
	TEST(local_task_var == 0x11223344);	//variable should be restored to the original value
	task_delete((task_handle_t *)task_local_var);
	
	
	for(uint8_t i=0; i<TEST_NUMBER_OF_TASKS; i++)
		test_rtos_remove_task_from_scheduler(i);
}

#endif

