cmake_minimum_required(VERSION 3.5)

set(SRC_LIST
	../Startup/startup_stm32f446retx.s
	syscalls.c
	sysmem.c
	system_stm32f4xx.c

	main.c
	stm32f4xx_it.c
	st7735_hwspi.c
	stm32f4xx_i2c_simple.c
	eeprom_i2c.c
	rgbcolor.c
	gfx.c
	fonts.c

	form_defs.c
	form_test_text_freesans_hints_mono.c
	form_test_text_freesans_hints_auto.c
	form_test_text_freeserif_hints_mono.c
	form_test_text_freeserif_hints_auto.c
	form_test_text_notosans_hints_mono.c
	form_test_text_notosans_hints_auto.c
	form_test_text_notoserif_hints_mono.c
	form_test_text_notoserif_hints_auto.c
	form_test_image_wb.c
	form_bench.c
)

set(ADD_LIBS)
#set(ADD_LIBS "-u _printf_float")
#set(ADD_LIBS "-u _printf_float -u _scanf_float")

add_executable(${PROJECT_NAME}.elf ${SRC_LIST})
target_link_libraries(${PROJECT_NAME}.elf ${ADD_LIBS})

add_custom_command(TARGET ${PROJECT_NAME}.elf
	POST_BUILD
	COMMAND ${CMAKE_OBJCOPY} -O ihex -R .lock -R .signature -R .user_signatures ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
	COMMAND ${CMAKE_OBJCOPY} -O binary -R .lock -R .signature -R .user_signatures ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
	COMMAND ${CMAKE_OBJCOPY} -O srec -R .lock -R .signature -R .user_signatures ${PROJECT_NAME}.elf ${PROJECT_NAME}.srec
	COMMAND ${CMAKE_OBJDUMP} -h -S ${PROJECT_NAME}.elf > ${PROJECT_NAME}.lss
	COMMAND ${CMAKE_SIZE} --format=sysv --radix=10 ${PROJECT_NAME}.elf
	COMMAND ${CMAKE_SIZE} --radix=10 ${PROJECT_NAME}.elf
)

set(addin_cleanup_files)
list(APPEND addin_cleanup_files ${PROJECT_NAME}.hex)
list(APPEND addin_cleanup_files ${PROJECT_NAME}.bin)
list(APPEND addin_cleanup_files ${PROJECT_NAME}.eep)
list(APPEND addin_cleanup_files ${PROJECT_NAME}.srec)
list(APPEND addin_cleanup_files ${PROJECT_NAME}.lss)
list(APPEND addin_cleanup_files ${PROJECT_NAME}.map)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${addin_cleanup_files}")
