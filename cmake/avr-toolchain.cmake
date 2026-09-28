# Cross-compilation toolchain for AVR targets.
# Usage: cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/avr-toolchain.cmake

set(CMAKE_SYSTEM_NAME      Generic)   # bare metal: no operating system
set(CMAKE_SYSTEM_PROCESSOR avr)

# While probing the compiler, CMake compiles a small program and tries to link
# it into an executable. That cannot work on AVR, so stop the probe at a static
# library instead. Without this, configuration fails with an error that gives
# no hint about the real cause.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Optional escape hatch: point at a specific toolchain without editing anything,
#   cmake --preset atmega1284 -DAVR_TOOLCHAIN_BIN=/opt/homebrew/opt/avr-gcc@15/bin
# Left empty, the tools are taken from PATH.
set(AVR_TOOLCHAIN_BIN "" CACHE PATH "directory holding avr-gcc and binutils")

find_program(CMAKE_C_COMPILER   avr-gcc     HINTS ${AVR_TOOLCHAIN_BIN} REQUIRED)
find_program(CMAKE_ASM_COMPILER avr-gcc     HINTS ${AVR_TOOLCHAIN_BIN} REQUIRED)
find_program(AVR_OBJCOPY        avr-objcopy HINTS ${AVR_TOOLCHAIN_BIN} REQUIRED)
find_program(AVR_OBJDUMP        avr-objdump HINTS ${AVR_TOOLCHAIN_BIN} REQUIRED)
find_program(AVR_SIZE           avr-size    HINTS ${AVR_TOOLCHAIN_BIN} REQUIRED)

# Report which compiler actually got picked. PATH order is easy to get wrong,
# and a silent fallback to an older avr-gcc is not something to discover later.
execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                OUTPUT_VARIABLE AVR_GCC_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "AVR toolchain: ${CMAKE_C_COMPILER} (avr-gcc ${AVR_GCC_VERSION})")

# Look for headers and libraries in the AVR toolchain only, never on the host.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM SEARCH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
