
# Build the executable based on the source files
set(TARGET_NAME PHD)

add_executable(${TARGET_NAME}
  ${CMAKE_SOURCE_DIR}/Libraries/CMSIS/Device/WIZnet/W7500/Source/GCC/startup_w7500.s
  ${CMAKE_SOURCE_DIR}/Projects/PHD/main.c 
  ${CMAKE_SOURCE_DIR}/Projects/PHD/W7500x_it.c
  ${CMAKE_SOURCE_DIR}/Projects/PHD/retarget.c
  ${CMAKE_SOURCE_DIR}/Projects/PHD/i2c.c
  ${CMAKE_SOURCE_DIR}/Projects/PHD/ssd1306.c
  ${CMAKE_SOURCE_DIR}/Projects/PHD/max30102.c
  ${CMAKE_SOURCE_DIR}/Projects/PHD/pulse.c
  ${CMAKE_SOURCE_DIR}/Projects/PHD/waveform.c  
  ${CMAKE_SOURCE_DIR}/Projects/PHD/httpParser.c
  ${CMAKE_SOURCE_DIR}/Projects/PHD/httpServer.c
  )

target_include_directories(${TARGET_NAME} PUBLIC
  ${CMAKE_SOURCE_DIR}/Projects/PHD
  )

target_link_libraries(${TARGET_NAME} PRIVATE
    SYSTEM_W7500X_FILES  
    GPIO_FILES
    UART_FILES
    ADC_FILES
    CRG_FILES
    DUALTIMER_FILES
    EXTI_FILES
    FLASH_FILES
    MIIM_FILES
    MISC_FILES
    PWM_FILES
    RNG_FILES
    RTC_FILES
    SSP_FILES
    WDT_FILES
    DNS_FILES
    DHCP_FILES
    SNTP_FILES
    SOCKET_FILES
    LOOPBACK_FILES
    WIZCHIP_CONF_FILES
    WZTOE_FILES
)

complie_link_option(${TARGET_NAME})
create_binary_file(${TARGET_NAME})
create_elf_file(${TARGET_NAME})

