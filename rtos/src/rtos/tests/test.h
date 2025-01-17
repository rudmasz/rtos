/*
 * test.h
 *
 * Created: 19.09.2024 16:49:30
 *  Author: tom
 */ 


#ifndef TEST_H_
#define TEST_H_

#include "rtos.h"

#define WDTO_15MS   0

/** \ingroup avr_watchdog
    See \c WDTO_15MS */
#define WDTO_30MS   1

/** \ingroup avr_watchdog
    See \c WDTO_15MS */
#define WDTO_60MS   2

/** \ingroup avr_watchdog
    See \c WDTO_15MS */
#define WDTO_120MS  3

/** \ingroup avr_watchdog
    See \c WDTO_15MS */
#define WDTO_250MS  4

/** \ingroup avr_watchdog
    See \c WDTO_15MS */
#define WDTO_500MS  5

/** \ingroup avr_watchdog
    See \c WDTO_15MS */
#define WDTO_1S     6

/** \ingroup avr_watchdog
    See \c WDTO_15MS */
#define WDTO_2S     7

#define  wdt_disable()
#define wdt_reset()
#define wdt_enable(m)
#define sleep_enable()
#define sleep_cpu()
#define sleep_disable()

#define TEST_NUMBER_OF_TASKS	5


void set_sleep_mode(uint8_t mode);


void test(char *file_name, uint16_t line_number);
__attribute__((noinline)) void test_rtos_back_jump(void);


#define TEST(x) ((x) ? NULL : \
					test(__FILE__, __LINE__))

#define rtos_back_jump()\
			asm volatile(\
				"ldi r30, pm_lo8(test_rtos_back_jump)	\n\t"\
				"ldi r31, pm_hi8(test_rtos_back_jump)	\n\t"\
				"icall									\n\t"\
			::)


void test_rtos_add_task_to_scheduler(uint8_t task_id, void (*task_function)(void));
void test_rtos_remove_task_from_scheduler(uint8_t task_id);
void test_rtos_set_as_current_running_task(uint8_t task_id);
task_handle_t *test_rtos_task_handle(uint8_t task_id);
void test_rtos_task_call(uint8_t task_id, uint8_t reset_pc);


void event_test(void);
void heap_test(void);
void semaphore_test(void);
void task_test(void);
void timers_test(void);




#endif /* TEST_H_ */