#ifndef __BOARDL_H_
#define __BOARDL_H_

#ifndef __AVR_ATmega1284__								//set atmega type
#define __AVR_ATmega1284__
#endif

#define BOARD_heap_number_of_blocks		0x10			//set the number of heap blocks 
#define BOARD_heap_single_block_size	0x20			//set single heap block size in bytes


#define BOARD_stack_size				300				//set stack size in bytes
#define BOARD_local_variable_stack_size	32				//set frame size for local variables in task
#define BOARD_startup_time_ms			0				//set startup delay for the system 
#define BOARD_watch_dog_time			WDTO_500MS		//set watchdog reset time
#define BOARD_cpu_clock					14745600		//set the frequency of the oscillator
#define BOARD_include_timers			TRUE			//set TRUE if you want to use timers
#define BOARD_has_external_clock_input	FALSE			//set TRUE if you connected an external 32.768KHz oscillator


#endif
