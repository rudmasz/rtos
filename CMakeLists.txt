cmake_minimum_required(VERSION 3.20)
project(avr-rtos C ASM)

# ------------------------------------------------------------------ settings
set(MCU   atmega1284   CACHE STRING "target AVR device")
set(EXAMPLE blink      CACHE STRING "subdirectory of examples/ to build")

option(RTOS_BUILD_TESTS "compile the built-in tests (RUN_TESTS)" OFF)
option(RTOS_SIMULATOR   "build for simavr (RUN_SIMULATOR): Timer2 runs from the CPU clock" OFF)

# Each example carries its own board.h next to its source, so everything that
# describes one example lives in one directory. A product built on this platform
# keeps board.h in its own repository the same way.
# Fail with one readable line instead of a landslide of missing-header errors.
foreach(f main.c board.h)
    if(NOT EXISTS ${CMAKE_SOURCE_DIR}/examples/${EXAMPLE}/${f})
        message(FATAL_ERROR "No examples/${EXAMPLE}/${f} - check the EXAMPLE value.")
    endif()
endforeach()

# --------------------------------------------------------------- compilation
add_compile_options(
    -mmcu=${MCU}
    -Os -g2 -std=gnu99
    -Wall -Wextra
    -funsigned-char -funsigned-bitfields
    -fpack-struct -fshort-enums
    # One section per function and per object, so the --gc-sections below can
    # drop individual unused ones. Without these the linker can only keep or
    # discard whole .o files, and every .o holds something that is used.
    -ffunction-sections -fdata-sections
)
add_compile_definitions(DEBUG)

# Under RUN_SIMULATOR Timer2 is clocked from the CPU instead of the 32 kHz TOSC
# input, so on real hardware the system time would run about 14x too fast.
# Make that impossible to miss: at configure time and after every build.
if(RTOS_SIMULATOR)
    add_compile_definitions(RUN_SIMULATOR)
    message(WARNING "SIMULATOR build (RUN_SIMULATOR) - do not flash this firmware onto hardware")
endif()

# ------------------------------------------------------------------- linking
# Reserve 8 bytes at the start of RAM (RAMSTART = 0x0100) for the structure the
# bootloader and the application use to exchange data; .data starts right after.
add_link_options(
    -mmcu=${MCU}
    -Wl,--gc-sections
    -mrelax
    -Wl,-section-start=.bootloader=0x800100
    -Wl,-section-start=.data=0x800108
)

# Expose the IntelliSense database at the project root so it works whether the
# build was driven from VS Code or from a terminal. Points at the active preset.
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    file(CREATE_LINK
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
        SYMBOLIC)
endif()

# ------------------------------------------------------------ debug support
# The debug server has to be told the same device and clock the firmware was
# built for. Rather than repeating them in launch.json, where they would drift
# the moment EXAMPLE or MCU changes, emit an argument file that PyAvrOCD reads
# with its @file syntax. launch.json then only points at this file.
file(STRINGS ${CMAKE_SOURCE_DIR}/examples/${EXAMPLE}/board.h _board_clock_line
     REGEX "#define[ \t]+BOARD_cpu_clock")
string(REGEX MATCH "BOARD_cpu_clock[ \t]+([0-9]+)" _match "${_board_clock_line}")
set(BOARD_CPU_CLOCK "${CMAKE_MATCH_1}")

if(NOT BOARD_CPU_CLOCK)
    message(WARNING "Could not read BOARD_cpu_clock from examples/${EXAMPLE}/board.h; "
                    "the debug configuration will need the clock set by hand.")
else()
    message(STATUS "Debug target: ${MCU} at ${BOARD_CPU_CLOCK} Hz (from examples/${EXAMPLE}/board.h)")
    # Written to a fixed path rather than the preset's binary dir: launch.json
    # can then name it with ${workspaceFolder} alone. Relying on
    # ${command:cmake.buildDirectory} there proved fragile - when VS Code leaves
    # it unresolved, PyAvrOCD gets a missing file, no -d, and the session dies
    # with "Debug server process failed to initialize".
    # The last configured preset wins, which is the one being debugged.
    file(WRITE ${CMAKE_SOURCE_DIR}/build/pyavrocd.args
         "-d\n${MCU}\n-F\n${BOARD_CPU_CLOCK}\n")
endif()

# ----------------------------------------------- platform as a static library
# Sources are listed explicitly on purpose. A forgotten file fails loudly at
# link time, while file(GLOB) would let a stray file into the binary silently.
add_library(rtos STATIC
    src/rtos/event.c
    src/rtos/heap.c
    src/rtos/rtos.c
    src/rtos/semaphore.c
    src/rtos/task.c
    src/rtos/timers.c
)

target_include_directories(rtos PUBLIC
    src/rtos            # core headers
    src/rtos/port       # hardware layer, selected by rtos.h from the device type
    examples/${EXAMPLE} # board.h, supplied by the example or product
)

# RUN_TESTS is PUBLIC because rtos.h keys symbol visibility off it; the library
# and anything linking it must agree on the value.
if(RTOS_BUILD_TESTS)
    target_sources(rtos PRIVATE
        tests/event_test.c
        tests/heap_test.c
        tests/rtos_test.c
        tests/semaphore_test.c
        tests/task_test.c
        tests/timers_test.c
    )
    target_include_directories(rtos PUBLIC tests)
    target_compile_definitions(rtos PUBLIC RUN_TESTS)
endif()

# ------------------------------------------------------------------ examples
add_executable(${EXAMPLE} examples/${EXAMPLE}/main.c)
target_link_libraries(${EXAMPLE} PRIVATE rtos)
target_link_options(${EXAMPLE} PRIVATE -Wl,-Map=$<TARGET_FILE_DIR:${EXAMPLE}>/${EXAMPLE}.map)

# ------------------------------------------------- artefacts and size report
add_custom_command(TARGET ${EXAMPLE} POST_BUILD
    COMMAND ${AVR_OBJCOPY} -O ihex -R .eeprom -R .fuse -R .lock -R .signature
            $<TARGET_FILE:${EXAMPLE}> $<TARGET_FILE_DIR:${EXAMPLE}>/${EXAMPLE}.hex
    COMMAND ${AVR_OBJDUMP} -h -S $<TARGET_FILE:${EXAMPLE}>
            > $<TARGET_FILE_DIR:${EXAMPLE}>/${EXAMPLE}.lss
    COMMAND ${AVR_SIZE} $<TARGET_FILE:${EXAMPLE}>
    VERBATIM
)
if(RTOS_SIMULATOR)
    add_custom_command(TARGET ${EXAMPLE} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "*** SIMULATOR BUILD - not for hardware ***"
        VERBATIM
    )
endif()
