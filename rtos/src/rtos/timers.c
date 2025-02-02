/*
 * timers.c
 *
 * Created: 22.12.2023 11:26:21
 *  Author: tom
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include "rtos.h"



#if BOARD_include_timers == TRUE
static volatile	timer_handle_t	*volatile __timer_timers;
#endif

static volatile	uint16_t		__timer_system_time;


/**********************************************************************************************//**
 * @fn	ISR(RTOS_peripheral_system_timer_vect)
 *
 * @brief	Interrupt routine count ms for system time
 *
 **************************************************************************************************/

ISR(RTOS_peripheral_system_timer_vect)
{
	__timer_system_time++;
}


/**********************************************************************************************//**
 * @fn	uint16_t __timer_ms_to_ticks_16bits(uint16_t time_ms)
 *
 * @brief	the function returns the value of the system clock ticks for a given time in ms
 *
 *
 * @returns	An uint16_t.
 **************************************************************************************************/

uint16_t __timer_ms_to_ticks_16bits(uint16_t time_ms)
{	
	uint16_t ticks = 0;

	if(time_ms & 0x8000)ticks = RTOS_peripheral_system_clock_ticks_for_0x8000ms;
	if(time_ms & 0x4000)ticks += RTOS_peripheral_system_clock_ticks_for_0x4000ms;
	if(time_ms & 0x2000)ticks += RTOS_peripheral_system_clock_ticks_for_0x2000ms;
	if(time_ms & 0x1000)ticks += RTOS_peripheral_system_clock_ticks_for_0x1000ms;
	if(time_ms & 0x0800)ticks += RTOS_peripheral_system_clock_ticks_for_0x0800ms;
	if(time_ms & 0x0400)ticks += RTOS_peripheral_system_clock_ticks_for_0x0400ms;
	if(time_ms & 0x0200)ticks += RTOS_peripheral_system_clock_ticks_for_0x0200ms;
	if(time_ms & 0x0100)ticks += RTOS_peripheral_system_clock_ticks_for_0x0100ms;
	if(time_ms & 0x0080)ticks += RTOS_peripheral_system_clock_ticks_for_0x0080ms;
	if(time_ms & 0x0040)ticks += RTOS_peripheral_system_clock_ticks_for_0x0040ms;
	if(time_ms & 0x0020)ticks += RTOS_peripheral_system_clock_ticks_for_0x0020ms;
	if(time_ms & 0x0010)ticks += RTOS_peripheral_system_clock_ticks_for_0x0010ms;
	if(time_ms & 0x0008)ticks += RTOS_peripheral_system_clock_ticks_for_0x0008ms;
	if(time_ms & 0x0004)ticks += RTOS_peripheral_system_clock_ticks_for_0x0004ms;
	if(time_ms & 0x0002)ticks += RTOS_peripheral_system_clock_ticks_for_0x0002ms;
	if(time_ms & 0x0001)ticks += RTOS_peripheral_system_clock_ticks_for_0x0001ms;

	return ticks;
}


/**********************************************************************************************//**
 * @fn	uint32_t __timer_ms_to_ticks_32bits(uint32_t time_ms)
 *
 * @brief	the function returns the value of the system clock ticks for a given time in ms
 *
 *
 * @returns	An uint32_t.
 **************************************************************************************************/

uint32_t __timer_ms_to_ticks_32bits(uint32_t time_ms)
{	
	uint32_t ticks = 0;
	
	if(time_ms & 0x40000000)ticks = RTOS_peripheral_system_clock_ticks_for_0x40000000ms;
	if(time_ms & 0x20000000)ticks += RTOS_peripheral_system_clock_ticks_for_0x20000000ms;
	if(time_ms & 0x10000000)ticks += RTOS_peripheral_system_clock_ticks_for_0x10000000ms;
	if(time_ms & 0x08000000)ticks += RTOS_peripheral_system_clock_ticks_for_0x08000000ms;
	if(time_ms & 0x04000000)ticks += RTOS_peripheral_system_clock_ticks_for_0x04000000ms;
	if(time_ms & 0x02000000)ticks += RTOS_peripheral_system_clock_ticks_for_0x02000000ms;
	if(time_ms & 0x01000000)ticks += RTOS_peripheral_system_clock_ticks_for_0x01000000ms;
	if(time_ms & 0x00800000)ticks += RTOS_peripheral_system_clock_ticks_for_0x00800000ms;
	if(time_ms & 0x00400000)ticks += RTOS_peripheral_system_clock_ticks_for_0x00400000ms;
	if(time_ms & 0x00200000)ticks += RTOS_peripheral_system_clock_ticks_for_0x00200000ms;
	if(time_ms & 0x00100000)ticks += RTOS_peripheral_system_clock_ticks_for_0x00100000ms;
	if(time_ms & 0x00080000)ticks += RTOS_peripheral_system_clock_ticks_for_0x00080000ms;
	if(time_ms & 0x00040000)ticks += RTOS_peripheral_system_clock_ticks_for_0x00040000ms;
	if(time_ms & 0x00020000)ticks += RTOS_peripheral_system_clock_ticks_for_0x00020000ms;
	if(time_ms & 0x00010000)ticks += RTOS_peripheral_system_clock_ticks_for_0x00010000ms;

	return ticks + (uint32_t)__timer_ms_to_ticks_16bits((uint16_t)time_ms);
}


/**********************************************************************************************//**
 * @fn	uint16_t __timer_get_time_ms(void)
 *
 * @brief	function returns the value of the ms counter, you must disable the global interrupt before calling this function
 *
 *
 * @returns	An uint16_t.
 **************************************************************************************************/

uint16_t __timer_get_time_ms(void)
{	
	return __timer_system_time;
}


/**********************************************************************************************//**
 * @fn	void __timer_clear_time_ms(void)
 *
 * @brief	function clears the ms counter, you must disable the global interrupt before calling this function
 *
 **************************************************************************************************/

void __timer_clear_time_ms(void)
{	
	__timer_system_time = 0;
}


#if BOARD_include_timers == TRUE

/**********************************************************************************************//**
 * @fn	void __timer_start_timer(timer_handle_t *timer, uint32_t tcnt, task_handle_t *listener, void (*notify_f)(void))
 *
 * @brief	Used by the system to start the timer. After the time has elapsed the timer can wake up the task or trigger a given function 
 *
 * @param 	timer   	If non-null, the timer to run.
 * @param 	listener	If non-null, the listener task to run.
 * @param 	notify_f	If non-null, the notify function to call.
 * @param 	tcnt		The time value to count in ms, max value is 2 162 687 832 [ms].
 **************************************************************************************************/

void __timer_start_timer(timer_handle_t *timer, uint32_t tcnt, task_handle_t *listener, void (*notify_f)(void))
{
	if( (tcnt == 0x00000000) || (timer == NULL) ) return;
	
	timer_handle_t *timer1  = (timer_handle_t *)__timer_timers;
	uint32_t c_time = 0;
	uint8_t irq_flag;
		
	irq_flag = rtos_cli();
	uint16_t current_time = __timer_system_time;
	rtos_sei(irq_flag);

	if(listener != NULL){
		timer->timer_owner = listener;
		timer->n = __TIMER_WITH_LISTENER;

	}else if(notify_f != NULL){
		timer->timer_notify_f = notify_f;
		timer->n = __TIMER_WITH_NOTIFY;
			
	}else{
		timer->timer_notify_f = NULL;
	}
	while( (timer1 != timer) && (timer1 != NULL) ){
		timer1 = timer1->next_timer;
	}
	if(timer1 == NULL){
		timer->next_timer = (timer_handle_t *)__timer_timers;
		__timer_timers	  = timer;
	}
	/**
	* We set the time [ms] for the countdown. 
	* We add the current system time to the tcnt and apply the error correction by subtracting 7.08 microseconds from each millisecond.
	*/
	if(!(tcnt & 0x80000000))tcnt += current_time;	//The TCNT timer register uses only 31 bits, so adding the current time no longer makes sense for much larger values. 
	
	if(tcnt >= 2162687832){ // if time is bigger than 2 162 687 832 [ms]
		c_time = 0x7FFFFFFF;	 //fixed time
			
	}else{
		c_time = __timer_ms_to_ticks_32bits(tcnt);
	}
	timer->TCNT = c_time;
}

/**********************************************************************************************//**
 * @fn	void __timer_stop_timer(timer_handle_t *timer, uint8_t if_notify_listener)
 *
 * @brief	Used by the system to stop the timer.
 *
 * @param 	timer			  	If non-null, the timer to stop.
 * @param 	if_notify_listener	if TRUE the task owning this timer will be woken up or notification function will be triggered.
 **************************************************************************************************/

void __timer_stop_timer(timer_handle_t *timer, uint8_t if_notify_listener)
{
	if(timer == NULL)return;
	
	timer_handle_t **timer_l = ((timer_handle_t **)&__timer_timers);

	while( (*timer_l != timer) && (*timer_l != NULL) ){
		timer_l = &((*timer_l)->next_timer);
	}
	
	if(*timer_l != NULL){
		timer->TCNT = 0x00000000;

		if( (timer->timer_owner != NULL) && (if_notify_listener == TRUE) ){
			if(timer->n == __TIMER_WITH_NOTIFY){
				timer->timer_notify_f();
				
			}else{
				task_state_t task_state = task_get_state(timer->timer_owner);
				
				if( (task_state == STOPPED) ||
					(task_state == SLEEP_INFINITE) ||
					(task_state == SLEEP_TIMED) )
				{
					task_unfreeze(timer->timer_owner);
				}
			}
		}
		if(timer->TCNT == 0){		//need to be checked because timer_notify_f() can restart timer and TCNT will be non zero
			timer->timer_owner	= NULL;
			*timer_l			= timer->next_timer;
			timer->next_timer	= NULL;
		}
	}
}


/**********************************************************************************************//**
 * @fn	void __timer_stop_all_for_given_task(task_handle_t *owner)
 *
 * @brief	Used by the system to stop all timers for a specific task
 *
 * @param 	owner	If non-null, the owner task.
 **************************************************************************************************/

void __timer_stop_all_for_given_task(task_handle_t *owner)
{
	timer_handle_t **timer  = (timer_handle_t **)&__timer_timers;

	while(*timer != NULL){
		if( ((*timer)->n == __TIMER_WITH_LISTENER) && ((*timer)->timer_owner == owner) ){
			timer_stop(*timer);
		
		}else{
			timer = &((*timer)->next_timer);
		}
	}
}


/**********************************************************************************************//**
 * @fn	void __timer_refresh_timers(uint16_t time)
 *
 * @brief	Used by the system to refresh the time in all timers
 *
 * @param	time	value to subtract from all counters.
 **************************************************************************************************/

void __timer_refresh_timers(uint16_t time)
{
	timer_handle_t **timer = ((timer_handle_t **)&__timer_timers);

	while(*timer != NULL){
		if((*timer)->TCNTL > time){
			(*timer)->TCNTL -= time;
		
		}else if((*timer)->TCNTH > 0){
			(*timer)->TCNTH--;
			(*timer)->TCNTL = (0xFFFF - time + (*timer)->TCNTL + 1);
			
		}else{
			(*timer)->TCNT = 0x00000000;

			if((*timer)->timer_owner != NULL){
				if((*timer)->n == __TIMER_WITH_NOTIFY){
					(*timer)->timer_notify_f();
						
				}else{
					task_state_t task_state = task_get_state((*timer)->timer_owner);
					
					if( (task_state == STOPPED) ||
						(task_state == SLEEP_INFINITE) ||
						(task_state == SLEEP_TIMED) )
					{
						task_unfreeze((*timer)->timer_owner);
					}
				}
			}
			if((*timer)->TCNT == 0x00000000){  //need to be checked because timer_notify_f() can restart timer and TCNT will be non zero
				timer_handle_t *t_off = *timer;
				(*timer)->timer_owner	= NULL;
				*timer				= (*timer)->next_timer;
				t_off->next_timer	= NULL;
				continue;	
			}
		}
		timer = &((*timer)->next_timer);
	}
}


/**********************************************************************************************//**
 * @fn	uint32_t timer_get_time(timer_handle_t *timer)
 *
 * @brief	use to get the remaining time
 *
 * @param 		timer			  	If non-null, the timer to read.
 * @returns		An uint32_t.
 **************************************************************************************************/

uint32_t timer_get_time(timer_handle_t *timer)
{
	return (timer != NULL) ? timer->TCNT : 0;	
}


/**********************************************************************************************//**
 * @fn	int8_t timer_cmp_timers_time(timer_handle_t *tim1, timer_handle_t *tim2)
 *
 * @brief	use to compare the remaining time of two timers
 *
 * @param 		tim1			  	If non-null, the timer to compare.
 * @param 		tim2			  	If non-null, the timer to compare.
 * @returns		0 - if equal, 
 *				1 - if the time of tim1 is greater than tim2, 
 *				-1 - if the time of tim1 is less than tim2.
 **************************************************************************************************/

int8_t timer_cmp_timers_time(timer_handle_t *tim1, timer_handle_t *tim2)
{
	if( (tim1 == NULL) || (tim2 == NULL) ||
		(tim1->TCNT == tim2->TCNT)
	)return 0;
	
	if(tim1->TCNT < tim2->TCNT) return -0x01;

	return 0x01;
}
#endif

