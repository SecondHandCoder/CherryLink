CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_PROCESSOR cortex-m3)
SET(CMAKE_VERBOSE_MAKEFILE ON)

IF(CHERRY_BUILD_TYPE MATCHES BOOT)
	MESSAGE(STATUS "BUILD BOOT")
	ADD_DEFINITIONS(-D__BUILD_BOOT__ -D__CONFIG_BOOT_SIZE__=0x2000)
	SET(OPTIMIZATION_LEVEL "-Os")
	SET(CMAKE_PROJECT_NAME CherryLinkBoot)
ELSE()	
	MESSAGE(STATUS "BUILD APP")
	ADD_DEFINITIONS(-D__CONFIG_BOOT_SIZE__=0x2000)
	SET(CHERRY_BUILD_TYPE APP)
	SET(OPTIMIZATION_LEVEL "-Ofast")
	SET(CMAKE_PROJECT_NAME CherryLinkApp)
ENDIF()

PROJECT(${CMAKE_PROJECT_NAME} C CXX ASM)

SET(CMAKE_BUILD_TYPE Release)
SET(CROSS_COMPILE_PREFIX arm-none-eabi-)
SET(PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR}/../..)

SET(MCU_FLAGS "-mcpu=cortex-m3 -mthumb -mthumb-interwork -Wall ${OPTIMIZATION_LEVEL}")
SET(SECTIONS_FLAGS "-ffunction-sections -fdata-sections -flto -fno-common -ffreestanding -MMD -MP")
SET(LINK_MAP_FLAGS "-Map=${PROJECT_NAME}.map")
SET(LINK_FLAGS "${MCU_FLAGS} ${SECTIONS_FLAGS} -Wl,--gc-sections,${LINK_MAP_FLAGS} --specs=nano.specs --specs=nosys.specs")
IF(CHERRY_BUILD_TYPE MATCHES BOOT)
	SET(LINK_SCRIPTS_FLAGS "-T ${CMAKE_SOURCE_DIR}/linker/CherryLinkBoot.lds")
ELSE()
	SET(LINK_SCRIPTS_FLAGS "-T ${CMAKE_SOURCE_DIR}/linker/CherryLinkApp.lds")	
ENDIF()

SET(CMAKE_C_COMPILER ${CROSS_COMPILE_PREFIX}gcc)
SET(CMAKE_ASM_COMPILER ${CROSS_COMPILE_PREFIX}gcc)
SET(CMAKE_CXX_COMPILER ${CROSS_COMPILE_PREFIX}g++)
SET(CMAKE_OBJCOPY ${CROSS_COMPILE_PREFIX}objcopy)
SET(CMAKE_SIZE ${CROSS_COMPILE_PREFIX}size)
SET(CMAKE_C_COMPILER_WORKS TRUE)
SET(CMAKE_CXX_COMPILER_WORKS TRUE)

SET(CMAKE_C_FLAGS_RELEASE " ${MCU_FLAGS} ${SECTIONS_FLAGS}")
SET(CMAKE_CXX_FLAGS_RELEASE " ${MCU_FLAGS} ${SECTIONS_FLAGS} -fno-exceptions")
SET(CMAKE_ASM_FLAGS_RELEASE " -c ${MCU_FLAGS} ${SECTIONS_FLAGS} -x assembler-with-cpp -Wa,-mimplicit-it=thumb")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE " ${LINK_FLAGS} ${LINK_SCRIPTS_FLAGS}")

SET(CMAKE_C_STANDARD 11)
SET(CMAKE_CXX_STANDARD 14)

IF(CHERRY_BUILD_TYPE MATCHES BOOT)
INCLUDE_DIRECTORIES(
    ${PROJECT_ROOT_DIR}/bootloader
	${PROJECT_ROOT_DIR}/board
	${PROJECT_ROOT_DIR}/cherry-usb
	${PROJECT_ROOT_DIR}/cherry-usb/common
	${PROJECT_ROOT_DIR}/cherry-usb/core
	${PROJECT_ROOT_DIR}/cherry-usb/port/ch32
	${PROJECT_ROOT_DIR}/cmsis-pack
	${PROJECT_ROOT_DIR}/startup
)

SET(PROJECT_SOURCES
    ${PROJECT_ROOT_DIR}/bootloader/boot_main.c
	${PROJECT_ROOT_DIR}/bootloader/usb_main.c
	${PROJECT_ROOT_DIR}/board/ch32f205_backup.c
	${PROJECT_ROOT_DIR}/board/ch32f205_clk.c
	${PROJECT_ROOT_DIR}/board/ch32f205_crc.c
	${PROJECT_ROOT_DIR}/board/ch32f205_flash.c
	${PROJECT_ROOT_DIR}/board/ch32f205_usb.c
	${PROJECT_ROOT_DIR}/cherry-usb/core/usbd_core.c
	${PROJECT_ROOT_DIR}/cherry-usb/port/ch32/usb_dc_usbhs.c
	${PROJECT_ROOT_DIR}/startup/startup.c
    ${PROJECT_ROOT_DIR}/project/gcc/nanolib.c
)
ELSE()
INCLUDE_DIRECTORIES(
    ${PROJECT_ROOT_DIR}/application
	${PROJECT_ROOT_DIR}/board
	${PROJECT_ROOT_DIR}/cherry-usb
	${PROJECT_ROOT_DIR}/cherry-usb/class/cdc
	${PROJECT_ROOT_DIR}/cherry-usb/common
	${PROJECT_ROOT_DIR}/cherry-usb/core
	${PROJECT_ROOT_DIR}/cherry-usb/port/ch32
	${PROJECT_ROOT_DIR}/cmsis-pack
	${PROJECT_ROOT_DIR}/cmsis-dap
	${PROJECT_ROOT_DIR}/rt-thread
	${PROJECT_ROOT_DIR}/rt-thread/include
	${PROJECT_ROOT_DIR}/rt-thread/ipc
	${PROJECT_ROOT_DIR}/startup
)

SET(PROJECT_SOURCES
    ${PROJECT_ROOT_DIR}/application/app_main.c
	${PROJECT_ROOT_DIR}/application/usb_main.c
	${PROJECT_ROOT_DIR}/board/ch32f205_backup.c
	${PROJECT_ROOT_DIR}/board/ch32f205_clk.c
	${PROJECT_ROOT_DIR}/board/ch32f205_dap.c
	${PROJECT_ROOT_DIR}/board/ch32f205_rt_thread.c
	${PROJECT_ROOT_DIR}/board/ch32f205_time.c
	${PROJECT_ROOT_DIR}/board/ch32f205_usb.c
	${PROJECT_ROOT_DIR}/board/ch32f205_watchdog.c   
	${PROJECT_ROOT_DIR}/cherry-usb/class/cdc/usbd_cdc.c
	${PROJECT_ROOT_DIR}/cherry-usb/core/usbd_core.c
	${PROJECT_ROOT_DIR}/cherry-usb/port/ch32/usb_dc_usbhs.c
	${PROJECT_ROOT_DIR}/cmsis-dap/dap_main.c
	${PROJECT_ROOT_DIR}/cmsis-dap/dap_vendor.c
	${PROJECT_ROOT_DIR}/cmsis-dap/jtag.c
	${PROJECT_ROOT_DIR}/cmsis-dap/swd.c
	${PROJECT_ROOT_DIR}/project/gcc/startup/startup_ch32f20x_app.s
	${PROJECT_ROOT_DIR}/rt-thread/ipc/completion.c
	${PROJECT_ROOT_DIR}/rt-thread/ipc/ringbuffer.c
	${PROJECT_ROOT_DIR}/rt-thread/libcpu/cortex-m3/context_gcc.S
	${PROJECT_ROOT_DIR}/rt-thread/libcpu/cortex-m3/cpuport.c
	${PROJECT_ROOT_DIR}/rt-thread/src/clock.c
	${PROJECT_ROOT_DIR}/rt-thread/src/components.c
	${PROJECT_ROOT_DIR}/rt-thread/src/idle.c
	${PROJECT_ROOT_DIR}/rt-thread/src/ipc.c
	${PROJECT_ROOT_DIR}/rt-thread/src/irq.c
	${PROJECT_ROOT_DIR}/rt-thread/src/kservice.c
	${PROJECT_ROOT_DIR}/rt-thread/src/mem.c
	${PROJECT_ROOT_DIR}/rt-thread/src/mempool.c
	${PROJECT_ROOT_DIR}/rt-thread/src/object.c
	${PROJECT_ROOT_DIR}/rt-thread/src/scheduler_up.c
	${PROJECT_ROOT_DIR}/rt-thread/src/thread.c
	${PROJECT_ROOT_DIR}/rt-thread/src/timer.c
	${PROJECT_ROOT_DIR}/startup/startup.c
    ${PROJECT_ROOT_DIR}/project/gcc/nanolib.c
)
ENDIF()

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME}.elf ${PROJECT_SOURCES} ${PROJECT_DEBUG_SOURCES})
ADD_CUSTOM_COMMAND(TARGET ${CMAKE_PROJECT_NAME}.elf POST_BUILD 
COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.bin COMMAND ${CMAKE_SIZE} ${CMAKE_PROJECT_NAME}.elf)