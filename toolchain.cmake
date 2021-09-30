set(CMAKE_SYSTEM_NAME Generic)
set(ProgramName "Memory_Testing_FW")

# Variables
#set(BoardName  "stm32f429")
#set(BoardClass "STM32F4")
#set(CPU "cortex-m4")
#set(FPUType "hard")
#set(FPUSpecification "fpv4-sp-d16")
#set(specs "rdimon.specs")


if(MINGW OR CYGWIN OR WIN32)
    set(UTIL_SEARCH_CMD where)
elseif(UNIX OR APPLE)
    set(UTIL_SEARCH_CMD which)
endif()

set(TOOLCHAIN_PREFIX arm-none-eabi-)

execute_process(
  COMMAND ${UTIL_SEARCH_CMD} ${TOOLCHAIN_PREFIX}gcc
  OUTPUT_VARIABLE BINUTILS_PATH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

get_filename_component(ARM_TOOLCHAIN_DIR ${BINUTILS_PATH} DIRECTORY)
# Without that flag CMake is not able to pass test compilation check
if (${CMAKE_VERSION} VERSION_EQUAL "3.6.0" OR ${CMAKE_VERSION} VERSION_GREATER "3.6")
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
else()
   # set(CMAKE_EXE_LINKER_FLAGS_INIT "--specs=nosys.specs")
endif()

set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)

SET(ASM_OPTIONS "-mcpu=${CPU} -g3 -c -x assembler-with-cpp  -mfpu=${FPUSpecification} -mfloat-abi=${FPUType} -mthumb")



# Set compile flags
string(TOUPPER "${BoardName}" BoardNameUC)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=${CPU} -std=gnu11 -g3 -DUSE_HAL_DRIVER -D${BoardNameUC}xx -O3 -ffunction-sections -Wall -fstack-usage  -mfpu=${FPUSpecification} -mfloat-abi=${FPUType} -mthumb")
set(CMAKE_EXE_LINKER_FLAGS "-mcpu=${CPU} -g3 -T\"${CMAKE_CURRENT_SOURCE_DIR}/${BoardNameUC}ZITx_FLASH.ld\" -Wl,-Map=\"${ProgramName}.map\" -Wl,--gc-sections -static -Wl,--start-group -lc -lrdimon -Wl,--end-group --specs=${specs} -mfpu=${FPUSpecification} -mfloat-abi=${FPUType} -mthumb -u _printf_float -Wl,--start-group -lc -lm -Wl,--end-group")
