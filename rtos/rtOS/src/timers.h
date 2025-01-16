/*
 * timers.h
 *
 * Created: 22.12.2023 11:28:46
 *  Author: tom
 */ 


#ifndef TIMERS_H_
#define TIMERS_H_

uint16_t __timer_get_time_ms(void);
void	 __timer_clear_time_ms(void);
uint16_t __timer_ms_to_ticks_16bits(uint16_t time_ms);
uint32_t __timer_ms_to_ticks_32bits(uint32_t time_ms);




#define __TIMER_WITH_NOTIFY		(TRUE)
#define __TIMER_WITH_LISTENER	(!__TIMER_WITH_NOTIFY)

/* The definition of the timers */
typedef struct timer{
	union{
		struct{
			uint32_t	TCNT:31;
			uint32_t	n:1;
		};
		struct{
			uint16_t 	TCNTL:16;
			uint16_t	TCNTH:15;
		};
	};
	
	union{
		task_handle_t 	 *timer_owner;
		void				(*timer_notify_f)(void);
	};
	struct timer 			*next_timer;

}timer_handle_t;





#if BOARD_include_timers == TRUE
void __timer_stop_all_for_given_task(task_handle_t *owner);
void __timer_refresh_timers(uint16_t time);
void __timer_start_timer(timer_handle_t *timer, uint32_t tcnt, task_handle_t *listener, void (*notify_f)(void));
void __timer_stop_timer(timer_handle_t *timer, uint8_t if_notify_listener);



/**********************************************************************************************//**
 * @fn	void timer_start_notify_task(timer_handle_t *timer, uint32_t tcnt, task_handle_t *listener)
 *
 * @brief	If you want to notify some listener/task when the countdown is finished, use this function to start the timer 
 *			max time 2 162 796 244[ms]
 *
 * @param 	timer   	If non-null, the timer to run.
 * @param 	listener	If non-null, the listener task to run.
 * @param 	tcnt		The time value to count in ms.
 **************************************************************************************************/
#define timer_start_notify_task(...)	VRG(timer_start_notify_task, __VA_ARGS__)
#define timer_start_notify_task2(timer, tcnt)			__timer_start_timer(timer, tcnt, task_this(), NULL)
#define timer_start_notify_task3(timer, tcnt, listener)	__timer_start_timer(timer, tcnt, listener, NULL)


/**********************************************************************************************//**
 * @fn	void timer_start_notify_function(timer_handle_t *timer, uint32_t tcnt, void (*notify_f)(void))
 *
 * @brief	If you want to call some function when the countdown is finished, use this function to start the timer 
 *			max time 2 162 796 244[ms]
 *
 * @param 	timer   	If non-null, the timer to run.
 * @param 	notify_f	If non-null, the notify function to call.
 * @param 	tcnt		The time value to count in ms.
 **************************************************************************************************/
#define timer_start_notify_function(timer, tcnt, notify)\
	__timer_start_timer(timer, tcnt, NULL, notify)


/**********************************************************************************************//**
 * @fn	void timer_start(timer_handle_t *timer, uint32_t tcnt)
 *
 * @brief	If you want to start the timer without sending a notification when the countdown ends, use this function to start the timer
 *			max time 2 162 796 244[ms] 
 *
 * @param 	timer   	If non-null, the timer to run.
 * @param 	tcnt		The time value to count in ms.
 **************************************************************************************************/
#define timer_start(timer, tcnt)\
	__timer_start_timer(timer, tcnt, NULL, NULL)


/**********************************************************************************************//**
 * @fn	uint32_t timer_get_time(timer_handle_t *timer)
 *
 * @brief	use to get the remaining time
 *
 * @param 	timer			  	If non-null, the timer to read.
 * @returns	uint32_t.
 **************************************************************************************************/
uint32_t timer_get_time(timer_handle_t *timer);



/**********************************************************************************************//**
 * @fn	void timer_stop_Notify(timer_handle_t *timer)
 *
 * @brief	use to stop the counter early and start the notification mechanism
 *
 * @param 	timer			  	If non-null, the timer to stop.
 **************************************************************************************************/
#define timer_stop_Notify(timer)\
	__timer_stop_timer(timer, TRUE)


/**********************************************************************************************//**
 * @fn	void timer_stop(timer_handle_t *timer)
 *
 * @brief	use to stop the counter early without the notification mechanism
 *
 * @param 	timer			  	If non-null, the timer to stop.
 **************************************************************************************************/
#define timer_stop(timer)\
	__timer_stop_timer(timer, FALSE)


/**********************************************************************************************//**
 * @fn	int8_t timer_cmp_timers_time(timer_handle_t *tim1, timer_handle_t *tim2)
 *
 * @brief	use to compare the remaining time of two timers
 *
 * @param 	tim1			  	If non-null, the timer to compare.
 * @param 	tim2			  	If non-null, the timer to compare.
 * @returns			0 - if equal, 
 *					1 - if the time of tim1 is greater than tim2, 
 *					-1 - if the time of tim1 is less than tim2.
 **************************************************************************************************/
int8_t timer_cmp_timers_time(timer_handle_t *tim1, timer_handle_t *tim2);


/**********************************************************************************************//**
 * @fn	uint8_t timer_is_counting_down(timer_handle_t *timer)
 *
 * @brief	use to check if the timer is counting down
 *
 * @param 	timer			  	If non-null, the timer to check.
 * @returns			TRUE - if is counting down,
 *					FALSE - if is not counting down.
 **************************************************************************************************/
#define timer_is_counting_down(timer) (timer_get_time(timer) != 0 ? TRUE : FALSE)


#endif /* RTOS_INCLUDE_TIMERS */


#endif /* TIMERS_H_ */