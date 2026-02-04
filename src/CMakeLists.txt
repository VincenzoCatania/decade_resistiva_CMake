cmake_minimum_required(VERSION 3.16)
project(decade C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Flags da preset
set(CMAKE_C_FLAGS "${MCU_FLAGS}")
set(CMAKE_ASM_FLAGS "${MCU_FLAGS}")

# Linker script da preset
set(CMAKE_EXE_LINKER_FLAGS
    "${MCU_FLAGS} -T${LDSCRIPT} -Wl,--gc-sections -Wl,-Map=${PROJECT_NAME}.map"
)

# SET MCU TARGET
add_definitions(-DSTM32F051x8)
add_definitions(-USE_HAL_DRIVER)


# Include dirs
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/bsp
    ${CMAKE_SOURCE_DIR}/bsp/driver
    ${CMAKE_SOURCE_DIR}/bsp/CMSIS
    ${CMAKE_SOURCE_DIR}/bsp/CMSIS/Include
    ${CMAKE_SOURCE_DIR}/bsp/CMSIS/Device
    ${CMAKE_SOURCE_DIR}/bsp/CMSIS/Device/ST
    ${CMAKE_SOURCE_DIR}/bsp/driver
    ${CMAKE_SOURCE_DIR}/scheduler
    ${CMAKE_SOURCE_DIR}/Startup
    ${CMAKE_SOURCE_DIR}/task
    ${CMAKE_SOURCE_DIR}/utility
    ${CMAKE_SOURCE_DIR}/manager
    ${CMAKE_SOURCE_DIR}/bsp/STM32F0xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/bsp/CMSIS/Device/ST/STM32F0xx/Include
)

# Sorgenti in modo ricorsivo tbd separazione in moduli
file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/*.c
    ${CMAKE_SOURCE_DIR}/bsp/*.c
    ${CMAKE_SOURCE_DIR}/bsp/driver/*.c
    ${CMAKE_SOURCE_DIR}/scheduler/*.c
    ${CMAKE_SOURCE_DIR}/*.s
    ${CMAKE_SOURCE_DIR}/task/*.c
    ${CMAKE_SOURCE_DIR}/utility/*.c
    ${CMAKE_SOURCE_DIR}/manager/*.c
)

add_executable(decade.elf ${SOURCES})
