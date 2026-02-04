set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m0)

set(TOOLCHAIN_PATH "C:/ST/STM32CubeIDE_1.17.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.12.3.rel1.win32_1.1.0.202410251130/tools/bin")

set(CMAKE_C_COMPILER   "${TOOLCHAIN_PATH}/arm-none-eabi-gcc.exe")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PATH}/arm-none-eabi-g++.exe")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_PATH}/arm-none-eabi-gcc.exe")

set(CMAKE_OBJCOPY "${TOOLCHAIN_PATH}/arm-none-eabi-objcopy.exe")
set(CMAKE_SIZE    "${TOOLCHAIN_PATH}/arm-none-eabi-size.exe")

set(ARCH_FLAGS "-mcpu=cortex-m0 -mthumb -mfloat-abi=soft")

set(CMAKE_C_FLAGS_INIT   "${ARCH_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${ARCH_FLAGS}")
set(CMAKE_ASM_FLAGS_INIT "${ARCH_FLAGS}")

set(CMAKE_EXE_LINKER_FLAGS_INIT
    "${ARCH_FLAGS} -Wl,--gc-sections -specs=nosys.specs"
)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
