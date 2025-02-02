## **System Description**

This system is a lightweight Real-Time Operating System (RTOS) designed for AVR microcontrollers. It provides efficient task scheduling, interrupt handling, memory management, and power optimization. The RTOS integrates various peripheral management features, including timers, semaphores, mutexes, events, and dynamic memory allocation, making it well-suited for embedded applications.

**System Features**

-   The RTOS scheduler operates in a **cooperative round-robin mode**, ensuring tasks execute in turns. Each task must explicitly yield the CPU, promoting fair resource distribution. To prevent any task from monopolizing the CPU, a **watchdog timer** (`BOARD_watch_dog_time`) enforces a maximum execution time. **Additional Scheduler function**: 
	- Refreshes time counters and time-delayed tasks.
	- Checks for reported interrupts and activates tasks waiting on those interrupts.
	- Efficiently performs context switching to maintain smooth task execution.
-   Interrupt handling with a dedicated reporting mechanism that stores interrupts in a dedicated register and processes them through the scheduler.
-   Power management is handled by an idle task that enters low-power sleep modes and dynamically controls peripheral states. When no active tasks, enabled peripherals, or pending interrupts are present, the idle task shifts the CPU into an energy-saving mode, such as  `SLEEP_MODE_IDLE`  or  `SLEEP_MODE_EXT_STANDBY`.
-   Reset event management, tracking causes such as watchdog, power-on, JTAG, external, and brownout resets.
- Dynamic memory management using a heap-based allocation system.
-   Event-driven task synchronization using semaphores, mutexes, and events to coordinate task execution.
-   Support for task delays and context switching.
-   Error detection and reporting mechanisms.
- The system can detect and respond to various CPU reset events, such as:
	-   `Brownout reset`
	-   `Power-on reset`
	-   `JTAG reset`
	-   `External reset`
	-   `Watchdog reset`, which provides details about the task that triggered it.
	
**User-Defined Handlers**
Users can define custom handlers for these reset events using function pointers. For example:
```c
void response_on_watchdog_reset(task_handle_t *err_task);
void (*rtos_response_on_watchdog_reset)(task_handle_t *current_task) = response_on_watchdog_reset;

void response_on_watchdog_reset(task_handle_t *err_task)
{
    // Custom reset handling code
}
```
This allows flexible and application-specific handling of reset conditions.


## **System Components**

### 1.  **Heap Management**

The heap is divided into fixed-sized blocks, where the size of each block is defined by the parameter `BOARD_heap_single_block_size`. The total number of blocks available in the heap is determined by `BOARD_heap_number_of_blocks`. Allocated memory from the heap is always rounded up to the nearest multiple of the block size.

**Heap Initialization**

The heap is initialized using the `heap_init()` function. This sets up the internal structures, such as allocation markers and free block counters, and prepares a semaphore to guard memory operations.

**Dynamic Memory Allocation**:
- The `heap_malloc(byte_num)` function allows the allocation of a specified number of bytes in the heap. It rounds up the requested size to the nearest multiple of the block size, ensuring that memory is always allocated in whole blocks.
- If sufficient contiguous blocks are not available, the function returns `NULL`.
- The `condWait_heap_malloc(byte_num)` macro enhances `heap_malloc(byte_num)` by adding task-waiting behavior. If memory is unavailable, the calling task is added to a waiting queue until memory becomes available. **This function must only be called within a task context**; otherwise, it can cause a memory leak and trigger a system reset.

**Memory Deallocation**

Memory allocated via the heap can be freed using the `heap_free(mem_addr)` function. It releases all blocks associated with a given memory address back to the heap and wakes up any tasks waiting for free memory.
  
**Heap Status and Validation**:
- The `heap_get_size_of_free_memory()` function returns the total size of free memory currently available in the heap.
- The `heap_check_if_dynamic_mem(mem_addr)` function verifies whether a given memory address falls within the heap’s managed range, returning `TRUE` or `FALSE`.

**Internal Mechanics**:
- Memory is managed using an array of allocation markers, where each marker corresponds to a block in the heap. A block marked as free is identified with `FREE_BLOCK_MARKER`. When memory is allocated, contiguous blocks are marked with a unique identifier derived from the starting block index.
- The heap employs a semaphore (`mem_guard`) to protect against concurrent access by multiple tasks, ensuring thread-safe operations.
---
### 2.  **Timers**

The timer module provides functionality for creating and managing software timers that operate in milliseconds. These timers are used for task scheduling, timeout handling, and event triggering in the RTOS. The system ensures precise timing operations using an interrupt-driven approach.

**Key Features**
-   Provides millisecond-level timing.
-   Supports task-based and function-based notifications upon timer expiration.
-   Uses a linked list to manage active timers.
-   Allows comparison of timers.
-   Supports stopping and refreshing timers.
-   Uses an interrupt-driven mechanism to track elapsed time.
-   Implements default arguments through the VRG macro for some functions.

**Starting a Timer**

&emsp;Timers can be started using convenience macros:

-   `timer_start_notify_task(timer, tcnt, listener)`: Notifies a task `listener` after tcnt milliseconds.
-   `timer_start_notify_function(timer, tcnt, notify_f)`: Calls a `notify_f` function after tcnt milliseconds.
-   `timer_start(timer, tcnt)`: Starts a timer without notifications.

**Stopping a Timer**

&emsp;A running timer can be stopped using the corresponding macros:
-   `timer_stop_Notify(timer)`: Stops the timer and triggers notification.
-   `timer_stop(timer)`: Stops the timer without notification.
  
**Retrieving Timer Information**:
- `timer_get_time(timer)`: Get remaining time.
- `timer_cmp_timers_time(timer1, timer2)`: Compare two timers.
- `timer_is_counting_down(timer)`: Check if a timer is counting down.  
---
### 3.  **Semaphores and Mutexes**

The system provides two primary synchronization mechanisms:
1. **Semaphores**: Counting semaphores manage a limited number of available resources, allowing tasks to increment or decrement the count and wait if no resources are available.

2. **Mutexes**: Mutexes are specialized semaphores designed to provide ownership-based exclusive access to shared resources, ensuring only one task can hold the mutex at a time.

Both semaphores and mutexes maintain pending task lists for cases where resources are unavailable, freezing tasks until they can proceed.

**Semaphores**:

- **Initialization**
	Semaphores are initialized with `semaphore_init(sem, max_count, init_count)` (or its macro forms). This function allows specifying the maximum count and initial count of the semaphore. A default argument mechanism (`VRG` macro) enables optional initialization with default values, where the initial count defaults to the maximum count.

- **Accessing Semaphore Information**:
	- `semaphore_get_count(sem)` retrieves the current count of a semaphore.
	- `semaphore_get_max_count(sem)` retrieves the maximum count value.
	- `semaphore_is_pending_list_empty(sem)` checks whether the semaphore’s pending task list is empty.

- **Usage**:
	- `semaphore_wait(sem)`: Decrements the semaphore count to gain access to a resource. Returns `TRUE` if access is granted or `FALSE` if unavailable.
	- `semaphore_signal(sem)`: Increments the semaphore count to release access to a resource. If tasks are pending, it wakes the next task in the queue.
	- `semaphore_remove_from_pending_list(task, sem)`: Removes a specific task from the semaphore’s pending task list.

**Mutexes**:
- **Initialization**
	Mutexes are initialized with `mutex_init(mutex)`, which sets them up as ownership-based semaphores.
	
- **Accessing Mutex Information**
	`mutex_is_pending_list_empty(mutex)` checks if the mutex has tasks waiting for access. This is implemented using the semaphore equivalent.
	
- **Usage**:
	- `mutex_check_access(mutex, task)`: Checks if a given task (defaulting to the current task) owns the mutex.
	- `mutex_unlock(mutex, task)`: Releases the mutex, transferring ownership to the next waiting task if applicable.
	- `mutex_remove_from_pending_list(task, mutex)`: Removes a task from the pending list for a mutex.  

**`condWait` Functions**
The system provides specialized macros for tasks that need to freeze execution until they acquire a semaphore or mutex:
- **`condWait_semaphore_wait(sem)`**: This macro allows tasks to wait for semaphore access. It freezes the calling task until the semaphore becomes available.
- **`condWait_mutex_lock(mutex)`**: This macro allows tasks to wait for mutex access. The task freezes until it successfully locks the mutex.

**General Workflow**:
1. **Initialization**: Create and initialize semaphores or mutexes.
2. **Task Interaction**:
	- Use `semaphore_wait(sem)` or `condWait_semaphore_wait(sem)` to acquire a semaphore.
	- Use `condWait_mutex_lock(mutex)` to acquire a mutex.
	- Release resources with `semaphore_signal(sem)` or `mutex_unlock(mutex)`.
3. **Pending Lists**: If tasks cannot immediately access a resource, they are added to the pending list and frozen until the resource becomes available.
---

### 4. **Events**
This system implements an event-driven notification mechanism. Its primary function is to facilitate communication between multiple tasks, ensuring that tasks can be notified and synchronized when specific actions or conditions occur. Here is an overview of its components and functionality:

**Event Actions (`event_action_t`)**:
- The system uses `event_action_t` (a typedef of `semaphore_t`) as the primary structure to manage event notifications.
- Event actions are initialized using `event_action_init(action)`, which prepares them to notify multiple tasks. This function ensures the event action is set up with an internal semaphore initialized to handle two states and no initial signals.

**Task Notification System**

The function `event_action_notify_listeners(sender)` wakes up all tasks that are waiting on a specific action. This allows multiple tasks to be informed and proceed when an event occurs.

**`condWait` Condition-Based Waiting**:
- The macro `condWait_event_action_wait(sender)` allows a task to suspend its execution and add itself to the list of tasks waiting for an action. This ensures a task will only resume once the associated event action is triggered.
- The macro `condWait_event_wait_signal(sig_src, sig_mask, time_ms)` suspends the execution of the calling task until a specific condition is met, defined as `*sig_src & sig_mask == sig_mask`. If the condition isn't met, and `time_ms` is non-zero, the task is also put into a timed suspension before checking the condition again. This ensures that tasks can wait for both precise conditions and timed delays.
---

### 5. **Task Management**

This system provides mechanisms for task creation, scheduling, suspension, and synchronization. The system supports both statically allocated tasks and dynamically allocated tasks in heap memory.

**Task States**

Each task in the system can exist in one of the following states:
<table>
  <tr>
    <th><small>State</small></th>
    <th><small>Description</small></th>
  </tr>
  <tr>
    <td><small><b>STOPPED</b></small></td>
    <td><small>The task is not running, and all its parameters are reset.</small></td>
  </tr>
  <tr>
    <td><small><b>READY</b></small></td>
    <td><small>The task is ready to execute but is waiting for CPU time.</small></td>
  </tr>
  <tr>
    <td><small><b>RUNNING</b></small></td>
    <td><small>The task is currently being executed.</small></td>
  </tr>
  <tr>
    <td><small><b>SLEEP_INFINITE</b></small></td>
    <td><small>The task is indefinitely suspended until explicitly resumed.</small></td>
  </tr>
  <tr>
    <td><small><b>SLEEP_TIMED</b></small></td>
    <td><small>The task is sleeping for a defined period.</small></td>
  </tr>
  <tr>
    <td><small><b>JOIN</b></small></td>
    <td><small>The task is waiting for another task to complete.</small></td>
  </tr>
  <tr>
    <td><small><b>WAIT_SEMA</b></small></td>
    <td><small>The task is waiting for a semaphore or mutex.</small></td>
  </tr>
  <tr>
    <td><small><b>INTERRUPT</b></small></td>
    <td><small>The task is waiting for an external interrupt.</small></td>
  </tr>
</table>

**Task Structure**

Each task is represented by a task_handle_t structure, which contains:
<table>  <tr>  <th><small>Task Attribute</small></th>  <th><small>Description</small></th>  </tr>  <tr>  <td><small><b>PC (Program Counter)</b></small></td>  <td><small>Execution address for resuming the task.</small></td>  </tr><tr>  <td><small><b>code_addr</b></small></td>  <td><small>Initial address of the task function.</small></td>  </tr><tr>  <td><small><b>State Variables</b></small></td>  <td><small>Stores task state and condition-related data.</small></td>  </tr>  <tr>  <td><small><b>Family Relationships</b></small></td>  <td><small>Parent-child relationships for dependencies.</small></td>  </tr>  <tr>  <td><small><b>Linked List Pointers</b></small></td>  <td><small>Supports a doubly linked list for scheduling.</small></td>  </tr>  <tr>  <td><small><b>Mutex List Pointer</b></small></td>  <td><small>Keeps track of the first held mutex.</small></td>  </tr>  <tr>  <td><small><b>Destructor Function</b></small></td>  <td><small>Cleanup function called upon task deletion.</small></td>  </tr>  </table>

For dynamically allocated tasks, `task_dynamic_handle_t` extends task_handle_t by providing extra memory space (`variables[]`) that tasks can use.

**Task Handling and Scheduling**

The system uses a **doubly linked list** to manage tasks. The ready task queue follows a **circular linked list** structure, where:
-   The currently running task points to the next task to be executed.
-   Tasks can be added/removed from the list dynamically.

**Key Task Management Functions**:
-  **`task_init()`** – Initializes task-related structures.
-  **`task_setup(task, task_code_addr, destructor_call_addr)`** – Initializes a new task.
-  **`task_new(task_code_addr, destructor_call_addr)`** – Creates a new task dynamically in heap memory.
-  **`task_start(task)`** – Starts or resumes a task.
-  **`task_erase(if_permanent, task)`** – Deletes a task and optionally frees memory.

**Task Context and Local Variables**

The system does **not** save the CPU registers and stack of a task when switching between tasks. Instead, **only the program counter (PC)** is stored in the task handle. Because of this:

-   **Local variables should be avoided** in the task body, as their values will be lost when the task is switched.
-   **Global variables marked as `volatile` should be used** to retain state across task switches.
-   **Static local variables with the `volatile` modifier** can also be used to store persistent task data.

**Dynamic Memory Management**

If a task handler is allocated in heap memory, **additional memory** is available due to rounding the allocated size to the `BOARD_heap_single_block_size`. The number of extra available bytes is defined as `TASK_number_of_dynamic_variables`.

To use this extra space, a user must:
   - Define a structure for task-specific variables.  
   - Use `TASK_init_dynamic_variables_pointer(struct_t, name)` to initialize a pointer to the extra memory.  
   - Access the fields using the created pointer.  

If the structure size exceeds `TASK_number_of_dynamic_variables`, a **compilation error** will occur.

**Example Usage**:
```c
// main.c
TASK_my_task_t menu_led_f(void) {
    struct local_var { uint8_t led; };
    DDRD = 0xff;
    TASK_init_dynamic_variables_pointer(local_var, ptr);  // Uses extra heap
    
    TASK_do {
        PORTD = ptr->led++;
        condWait_task_delay(100);  // Suspend for 100ms
    } TASK_loop();
}
```
In this example:
-   The `local_var` structure is used to store dynamic variables.
-   The `TASK_init_dynamic_variables_pointer` macro ensures that `ptr_local` correctly points to the allocated space.
-   The `TASK_do` and `TASK_loop()` macros create a loop in which the task sleeps for 100 ms, assigns the value of `ptr->led` to the `PORTD` register, and increments `ptr->led`.

**Interrupt Handling**

Tasks can be paused until an external interrupt is triggered using  `condWait_task_wait_irq(irq_nr)`, which halts execution until the specified interrupt occurs.

**Task Dependencies**
Tasks can have parent-child relationships:
-   `condWait_task_join(child_task)` – Puts the current task to sleep until child_task completes.
-   `condWait_task_try_to_join(child_task)` – Similar, but exits if joining is not possible.
-   `task_check_relationship(parent, child)` – Checks if a child task is a descendant of a parent task.

**Task Delay and Sleep**:
-   `condWait_task_delay(time_ms)` – Suspends the current task for a given time.
-   `condWait_task_infinite_sleep()` – Puts the task to **permanent sleep** without waking the parent.
-   `condWait_task_infinite_sleep_wup()` – Similar, but wakes up the parent.

**Task Termination**:
-   `task_delete(task)` – Deletes a task and frees its handler memory.
-   `task_stop(task)` – Stops a task but keeps its handler memory.
---

### 6. **System Startup and Configuration**

The RTOS framework enables users to define and execute tasks with scheduling capabilities. System initialization is handled via the function pointer  `void(*rtos_initialize_avr_device)(void)`, ensuring that essential components are set up before execution.

**Key Files**:
-   **main.c**: Demonstrates how to:
    -   Initialize the system.
    -   Define and start tasks.
    -   Use timers for periodic events.
-   **board.h**: Defines essential hardware-related constants for system configuration.  **This file must be created and included by the user**; otherwise, compilation will fail.
    

**Constants Defined in board.h**

The board.h file contains essential definitions for system configuration:

| Constant Name                      | Description                                               | Default Value            |
|------------------------------------|-----------------------------------------------------------|--------------------------|
| `__AVR_ATmega1284__`               | Defines the target AVR microcontroller                    | `ATmega1284`             |
| `BOARD_heap_number_of_blocks`      | Number of memory blocks allocated in the heap             | `0x10 (16)`              |
| `BOARD_heap_single_block_size`     | Size of a single heap memory block (bytes)                | `0x20 (32)`              |
| `BOARD_stack_size`                 | Stack size in bytes for tasks                             | `300`                    |
| `BOARD_local_variable_stack_size`  | Stack size for local variables inside tasks               | `32`                     |
| `BOARD_startup_time_ms`            | System startup delay (milliseconds)                       | `0`                      |
| `BOARD_watch_dog_time`             | Watchdog timer reset interval                             | `WDTO_500MS (500 ms)`    |
| `BOARD_cpu_clock`                  | CPU clock frequency (Hz)                                  | `14745600 (14.7456 MHz)` |
| `BOARD_include_timers`             | Enables timer functionality (`TRUE` or `FALSE`)          | `TRUE`                   |
| `BOARD_has_external_clock_input`   | Indicates if an external 32.768 kHz oscillator is connected (`TRUE` or `FALSE`) | `FALSE` |


Users can modify these constants to suit their specific hardware and application needs.

**Example of Task Initialization in main.c**

The main.c file demonstrates how to define and start a simple task (`menu_led_f`) using the RTOS framework.

```c
#include <avr/io.h>
#include "rtos.h"

task_handle_t menu_led;
timer_handle_t timer;
void INI();
void(*rtos_initialize_avr_device)(void) = INI;

void timer_function(void)
{
    PORTD ^= 0xff;
    timer_start_notify_function(&timer, 1000, timer_function);
}

TASK_my_task_t menu_led_f(void)
{
    static volatile uint8_t led = 0;
    DDRD = 0xff;

    TASK_do{
        PORTD = led++;
        condWait_task_delay(100);

        if(led == 0){
            timer_function();
            condWait_task_delay(4999);
            timer_stop(&timer);
        }

    }TASK_loop();
}

void INI()
{
    task_setup(&menu_led, menu_led_f);
    task_start(&menu_led);
}
```
**Key Components in main.c**:
-  **Defining a Task `menu_led_f`**:
	-   This task manipulates `PORTD` to control an `led`.
	-   It increments the `led` state and waits for a specific time `condWait_task_delay(100)`.
	-   It also starts and stops a timer when the `led` counter resets.

- **Timer Function `timer_function`**:
	-   Toggles all bits of  `PORTD`  (flashes LEDs).
	-   Restarts itself with a 1000 ms (1 second) delay.

- **System Initialization `INI`**
	-   Sets up the  `menu_led`  task by linking it to  `menu_led_f`.
	-   Starts the task execution.
	-   The system initializes via the function pointer `(*rtos_initialize_avr_device) = INI`, ensuring all necessary configurations are performed.
---

### 7. **Important Constraints**

-   Functions starting with **`condWait`**  **must only be used inside a task function**. Calling these functions outside the task body will result in a **memory leak** and a **system reset** due to improper handling of task-specific data.
-   **Local variables in tasks are lost when switching tasks** unless explicitly saved.
-   **Dynamically allocated tasks handlers can use additional heap space** but must stay within the memory limit defined by `TASK_number_of_dynamic_variables`.
-  **Default Argument Mechanism**:
Many functions in this system are extended using a default argument mechanism implemented via the `VRG` macro. This macro provides flexibility by allowing:
	- Overloading of function calls with fewer arguments.
	- Default values for omitted arguments.
	
	Examples:
	- `semaphore_init(sem, max_count)` instead of `semaphore_init(sem, max_count, init_count)` defaults the initial count to the maximum count if not explicitly specified.
	- `mutex_unlock(mutex)`  instead of `mutex_unlock(mutex, task)`defaults to unlocking the mutex for the current task.

---

### 8. **TODO List (Planned Enhancements)**

- [ ] `ADC + NTC10K`: Implement analog-to-digital conversion support with NTC10K temperature sensors.  
- [ ] `1-Wire Interface (Interrupt-Based)`: Optimize CPU usage by handling 1-Wire protocol via interrupts.  
- [ ] `DS18B20 Sensor`: Implement temperature reading from DS18B20 sensors.  
- [ ] `USART`: Enable serial communication support.  
- [ ] `MODBUS RTU`: Implement MODBUS RTU communication protocol.  
- [ ] `TWI (I2C)`: Integrate two-wire interface (I2C) for peripheral communication.  
- [ ] `Msgbox`: Enhance heap memory management to allow merging separated memory blocks into a virtual memory space for Ethernet communication.  
- [ ] `SPI`: Implement SPI peripheral support.  
- [ ] `Ethernet UDP`: Enable UDP-based network communication.  
- [ ] `Bootloader with MODBUS RTU Update Support`: Implement a bootloader that allows firmware updates via MODBUS RTU.  
  