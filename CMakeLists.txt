cmake_minimum_required(VERSION 3.5)

project(nucleo-f446re-st7735spi-shield)

# cmake must be called with following options:
# cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/mingw-w64-x86_64-arm-none-eabi-toolchain.cmake <...>
#  or
# cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/gnu-tools-arm-none-eabi-10.3-2021.07.win32.cmake <...>
#  or
# cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/gnu-tools-arm-none-eabi-10.3-2021.07.linux64.cmake <...>

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

set(CMAKE_VERBOSE_MAKEFILE on)

if(NOT (${CMAKE_SYSTEM_NAME} STREQUAL "ARM_NONE_EABI"))
	message(FATAL_ERROR "Invalid toolchains!")
endif(NOT (${CMAKE_SYSTEM_NAME} STREQUAL "ARM_NONE_EABI"))

enable_language(ASM)
enable_language(C)


set(VERSION				"0.8.0")

set(GCC_CPU  "cortex-m4")
set(FPU_OPTS "-mfpu=fpv4-sp-d16 -mfloat-abi=hard")
set(STDLIB_SPEC "--specs=nano.specs")

set(COMMON_FLAGS "-mcpu=${GCC_CPU} -ffunction-sections -fdata-sections -fstack-usage ${STDLIB_SPEC} ${FPU_OPTS} -mthumb")

set(CMAKE_ASM_FLAGS					"-x assembler-with-cpp ${COMMON_FLAGS}")

set(CMAKE_C_FLAGS					"-Wall -std=gnu99 ${COMMON_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE			"-O3 -g0  -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG				"-g2 -O1 -DDEBUG=1 -DDEBUG_MSG=1")
set(CMAKE_C_FLAGS_RELWITHDEBINFO	"-g2 -O2 -DDEBUG=1")
set(CMAKE_C_FLAGS_MINSIZEREL		"-Os -DNDEBUG")

set(CMAKE_CXX_FLAGS					"-Wall -std=c++03  ${COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE			${CMAKE_C_FLAGS_RELEASE})
set(CMAKE_CXX_FLAGS_DEBUG			${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO	${CMAKE_C_FLAGS_RELWITHDEBINFO})
set(CMAKE_CXX_FLAGS_MINSIZEREL		${CMAKE_C_FLAGS_MINSIZEREL})

add_definitions(-DSTM32 -DSTM32F4 -DSTM32F446xx -DSTM32F446RETx -DHSE_VALUE=8000000)

set(TARGET_LDSCRIPT	    "STM32F446RETX_FLASH.ld")

set(CMAKE_EXE_LINKER_FLAGS   "-T${CMAKE_SOURCE_DIR}/${TARGET_LDSCRIPT} -Wl,-Map=${PROJECT_NAME}.map -Wl,--gc-sections -static ${FPU_OPTS} -mthumb -Wl,--start-group -lc -lm -Wl,--end-group")

include_directories(Inc)
include_directories(Drivers/CMSIS/Include)
include_directories(Drivers/CMSIS/Device/ST/STM32F4xx/Include)
include_directories(fonts)

add_subdirectory(Src)

configure_file(flash.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/flash.sh)
