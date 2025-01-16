/*
 * rtos_test.c
 *
 * Created: 18.09.2024 14:32:03
 *  Author: tom
 */
#ifdef RUN_TESTS
#include <avr/io.h>
#include <avr/interrupt.h>
#include "test.h"
#include "rtos.h"



#define MAX_FAILED_TESTS 30


#define RUN_MODE				0xFF


extern uint8_t __stack[];
extern task_handle_t __idle_task;
extern task_handle_t *__task_ready_g;
extern rtos_peripheral_irq_register_t __rtos_irq_reg;
extern rtos_peripheral_register_t __rtos_peripherals;
extern uint32_t	__rtos_system_time;

void idle_task();

task_handle_t	test_tasks[TEST_NUMBER_OF_TASKS];


uint8_t initialization = TRUE;
uint8_t sleep_mode = RUN_MODE;


struct{
	struct{
		char	*file_name;
		uint16_t line_number;
	}test[MAX_FAILED_TESTS];
	
	uint8_t cnt;
}failed;


void init_tests(void);

void (*rtos_initialize_avr_device)(void) = init_tests;



__attribute__((noinline)) void test_rtos_back_jump(void)
{
	if(initialization == FALSE)
		__rtos_back_jump();	
}

task_handle_t *test_rtos_task_handle(uint8_t task_id)
{
	return (task_id < TEST_NUMBER_OF_TASKS) ? &test_tasks[task_id] : test_tasks;
}

void test_rtos_task_call(uint8_t task_id, uint8_t reset_pc)
{
	if(task_id < TEST_NUMBER_OF_TASKS){
		task_handle_t *task = &test_tasks[task_id];
		
		__task_ready_g->state = READY;
		__task_ready_g = task;
		__task_ready_g->state = RUNNING;
		if(reset_pc)task->PC = task->code_addr;
		asm volatile(
		"push r28 \n\t"
		"push r29 \n\t"
		"in r28, __SP_L__ \n\t"
		"in r29, __SP_H__ \n\t"
		::);
		((void (*)(void))task->PC)();
		asm volatile(
		"pop r29 \n\t"
		"pop r28 \n\t"
		::);
	}
}

void test_rtos_add_task_to_scheduler(uint8_t task_id, void (*task_function)(void))
{
	if(task_id < TEST_NUMBER_OF_TASKS){
		task_handle_t *task = &test_tasks[task_id];
		
		if(task->next_task == NULL){
			task_setup(task, task_function, NULL);
			task_start(task);
		}
	}
}

void test_rtos_remove_task_from_scheduler(uint8_t task_id)
{
	if(task_id < TEST_NUMBER_OF_TASKS){
		task_handle_t *task = &test_tasks[task_id];
	
		if(task->next_task != NULL){
			if(__task_ready_g == task){
				__task_ready_g = task->next_task;
				__task_ready_g->state = RUNNING;
			}
			(task->next_task)->prev_task = task->prev_task;
			(task->prev_task)->next_task = task->next_task;
			task->next_task = NULL;
			task->prev_task = NULL;
			task->state = STOPPED;
		}
	}
}

void test_rtos_set_as_current_running_task(uint8_t task_id)
{
	if( (task_id < TEST_NUMBER_OF_TASKS) && (test_tasks[task_id].next_task != NULL) ){
		__task_ready_g->state = READY;
		__task_ready_g = &test_tasks[task_id];
		__task_ready_g->state = RUNNING;
	}
}

void test_rtos_peripherals_off(void)
{
	__rtos_peripherals = _BV(RTOS_peripheral_system_clock_timer);
}

void test_rtos_irq_reset(void)
{
	__rtos_irq_reg = 0;
}

static void test_idle_task(void)
{
	
}

/**********************************************************************************************//**
 * Idle task test
 *			any_irq_pending == TRUE -> run mode
 *
 *			any_peripheral		tasks_in_scheduler
 *				FALSE				FALSE				-> SLEEP_MODE_EXT_STANDBY
 *				TRUE				FALSE				-> SLEEP_MODE_IDLE
 *				FALSE				TRUE				-> run mode
 *				TRUE				TRUE				-> run mode
 *
 **************************************************************************************************/
void set_sleep_mode(uint8_t mode)
{
	sleep_mode = mode;
}

void check_sleep_mode(void)
{
/*			any_peripheral		tasks_in_scheduler
*				FALSE				FALSE				-> SLEEP_MODE_EXT_STANDBY
*/
	test_rtos_remove_task_from_scheduler(0);
	test_rtos_peripherals_off();
	test_rtos_irq_reset();
	idle_task();
	TEST(sleep_mode == SLEEP_MODE_EXT_STANDBY);

/*			any_peripheral		tasks_in_scheduler
*				TRUE				FALSE				-> SLEEP_MODE_IDLE
*/
	test_rtos_remove_task_from_scheduler(0);
	rtos_peripheral_switch_on(_TWI);
	test_rtos_irq_reset();
	idle_task();
	TEST(sleep_mode == SLEEP_MODE_IDLE);
	
/*			any_peripheral		tasks_in_scheduler
*				FALSE				TRUE				-> run mode
*/
	test_rtos_add_task_to_scheduler(0, test_idle_task);
	test_rtos_peripherals_off();
	test_rtos_irq_reset();
	sleep_mode = RUN_MODE;
	idle_task();
	TEST(sleep_mode == RUN_MODE);

/*			any_peripheral		tasks_in_scheduler
*				TRUE				TRUE				-> run mode
*/
	test_rtos_add_task_to_scheduler(0, test_idle_task);
	rtos_peripheral_switch_on(_AC);
	test_rtos_irq_reset();
	sleep_mode = RUN_MODE;
	idle_task();
	TEST(sleep_mode == RUN_MODE);
	
/*			any_peripheral		tasks_in_scheduler
*				FALSE				FALSE				-> SLEEP_MODE_EXT_STANDBY
*								any_irq_pending == TRUE -> run mode
*/
	test_rtos_remove_task_from_scheduler(0);
	test_rtos_peripherals_off();
	test_rtos_irq_reset();
	rtos_irq_report(_IrqTIMER3_OVF);
	sleep_mode = RUN_MODE;
	idle_task();
	TEST(sleep_mode == RUN_MODE);	
	
	test_rtos_remove_task_from_scheduler(0);
	test_rtos_peripherals_off();
	test_rtos_irq_reset();
}


/**********************************************************************************************//**
 * Macros tests
 **************************************************************************************************/

void check_load_uint_8(void)
{
	uint8_t src, dest;
	uint8_t *src_ptr = &src;
	
	src = 34;
	rtos_load_uint8_t(dest, src);
	TEST(dest == src);
	
	*src_ptr = 55;
	rtos_load_uint8_t_from_ptr(dest, src_ptr);
	TEST(dest == src);
}

void check_load_uint_16(void)
{
	uint16_t src, dest;
	uint16_t *src_ptr = &src;
	
	src = 34589;
	rtos_load_uint16_t(dest, src);
	TEST(dest == src);
	
	*src_ptr = 55221;
	rtos_load_uint16_t_from_ptr(dest, src_ptr);
	TEST(dest == src);
}

void check_load_uint_32(void)
{
	uint32_t src, dest;
	
	src = 33997766;
	rtos_load_uint32_t(dest, src);
	TEST(dest == src);
}

/***********************************************************************************************************************************************/
void test(char *file_name, uint16_t line_number)
{
	if (failed.cnt < MAX_FAILED_TESTS){
		failed.test[failed.cnt].file_name = file_name;
		failed.test[failed.cnt].line_number = line_number;
		failed.cnt++;
	}
}

void init_tests(void)
{	
	uint16_t sp;
	
	asm volatile(
		"in %A0, __SP_L__	\n\t"
		"in %B0, __SP_H__	\n\t"
		:"=r"(sp)
		:
	);
/****** STACK ******/
	TEST(BOARD_stack_size > 100);
	TEST(sp > (uint16_t)&__stack[BOARD_stack_size - BOARD_local_variable_stack_size - 20]);
	TEST(sp <= (uint16_t)&__stack[BOARD_stack_size - 1]);
	TEST(__stack[0] == RTOS_stack_overflow_tag);
	TEST(__stack[1] == RTOS_stack_overflow_tag);
	TEST(__stack[BOARD_stack_size - 1] == RTOS_stack_overflow_tag);
	TEST(__stack[BOARD_stack_size - 2] == RTOS_stack_overflow_tag);
	
/****** RTOS MACROS ******/
	TEST(rtos_cnt_bits_uint8_t(0x00) == 0);
	TEST(rtos_cnt_bits_uint8_t(0xF0) == 4);
	TEST(rtos_cnt_bits_uint8_t(0xFF) == 8);
	check_load_uint_8();
	check_load_uint_16();
	check_load_uint_32();
	
/****** SYSTEM TIME ******/
	__rtos_system_time = 3456;
	TEST(rtos_get_system_time_ms() == __rtos_system_time);

/****** INTERRUPTS ******/
	sei();
	TEST(rtos_get_global_interrupt_state() == TRUE);
	TEST(rtos_cli() == TRUE);
	TEST(rtos_get_global_interrupt_state() == FALSE);
	rtos_sei(TRUE);
	TEST(rtos_get_global_interrupt_state() == TRUE);
	cli();
	rtos_irq_report(_IrqTIMER3_COMPA);
	TEST(((uint64_t)1 << _IrqTIMER3_COMPA) == __rtos_irq_reg);
	rtos_irq_report(_IrqPCINT0);
	TEST( (((uint64_t)1 << _IrqTIMER3_COMPA) | ((uint64_t)1 << _IrqPCINT0) ) == __rtos_irq_reg);
	TEST(rtos_irq_get(_IrqTIMER3_COMPA) == TRUE);
	TEST(((uint64_t)1 << _IrqPCINT0) == __rtos_irq_reg);
	TEST(rtos_irq_get(_IrqPCINT0) == TRUE);
	TEST(rtos_irq_get(_IrqPCINT0) == FALSE);
	TEST(rtos_irq_get(_IrqTIMER3_COMPA) == FALSE);

/****** PERIPHERALS ******/
	rtos_peripheral_switch_on(_SPI);
	TEST(rtos_peripheral_get_state(_SPI) == ON);
	rtos_peripheral_switch_on(_AC);
	TEST(rtos_peripheral_get_state(_AC) == ON);
	TEST(rtos_peripheral_get_state(_ADC) == ON);	//for the analog comparator we have to switch on the analog to digital converter as well
	rtos_peripheral_switch_off(_SPI);
	TEST(rtos_peripheral_get_state(_SPI) == OFF);
	rtos_peripheral_switch_off(_AC);
	rtos_peripheral_switch_off(_ADC);
	TEST(rtos_peripheral_get_state(_AC) == OFF);
	TEST(rtos_peripheral_get_state(_ADC) == OFF);	

/****** IDLE TASK ******/
	check_sleep_mode();
	
/****** EVENT FILE ******/
	event_test();

/****** HEAP FILE ******/
	heap_test();
	
/****** SEMAPHORE FILE ******/
	semaphore_test();

/****** TASK FILE ******/
	task_test();

/****** TIMERS FILE ******/
	timers_test();

	while(failed.cnt);
	
	for(uint8_t i=0; i<TEST_NUMBER_OF_TASKS; i++)
		test_rtos_remove_task_from_scheduler(i);
	initialization = FALSE;
}




#endif 
