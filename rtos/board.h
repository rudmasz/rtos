#ifndef __BOARDL_H_
#define __BOARDL_H_

#ifndef __AVR_ATmega1284__
#define __AVR_ATmega1284__
#endif

#define BOARD_heap_number_of_blocks		0x10
#define BOARD_heap_single_block_size	0x20



#define BOARD_stack_size				300
#define BOARD_local_variable_stack_size	32
#define BOARD_startup_time_ms			0
#define BOARD_watch_dog_time			WDTO_500MS				//CZAS RESETU WATCHDOGA EDYCJA PATRZ BIBLIOTEKA wdt.h
#define BOARD_cpu_clock					14745600
#define BOARD_include_timers			TRUE
#define BOARD_has_external_clock_input	FALSE

#define BOARD_reset_device_time_s		10

#endif
