cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app LANGUAGES C)

target_sources(app PRIVATE 
    src/main.c
    # src/host_msd_command.c
    src/host_msd_fatfs.c
)

# This variable is used in usb/ cmake files
set(MCUX_SDK_PROJECT_NAME app)


target_include_directories(app PUBLIC 
    include
    )

include_directories(include)

# USB Example 

include(config.cmake)

add_definitions(-DFSL_RTOS_ZEPHYR)
add_definitions(-DCONFIG_ZEPHYR)
add_definitions(-DHOST_ECHO=1)

# OSA
set(CONFIG_USE_component_osa 1)

# set(CONFIG_USE_middleware_baremetal 1)
set(CONFIG_USE_driver_common 1)
set(CONFIG_USE_component_lists 1)
set(CONFIG_USE_component_osa_interface 1)
set(CONFIG_USE_osa_zephyr 1)

include(lists/component_lists.MIMXRT1062.cmake)
include(osa/component_osa.cmake)
include(osa/component_osa_zephyr.cmake)

# USB
set(CONFIG_USE_middleware_usb_common_header 1)
set(CONFIG_DEVICE_ID "MIMXRT1062xxxxB")
set(CONFIG_USE_middleware_usb_host_stack 1)
set(CONFIG_USE_middleware_usb_host_ehci 1) 
set(CONFIG_USE_middleware_usb_phy 1)
set(CONFIG_USE_middleware_usb_host_common_header 1)
set(CONFIG_USE_middleware_usb_host_ehci_config_header 1)
set(CONFIG_USE_driver_memory 1)

include(usb/middleware_usb_common_header.cmake)
include(usb/middleware_usb_host_common_header.cmake)
include(usb/middleware_usb_host_stack.cmake)
include(usb/middleware_usb_host_msd.cmake)
include(usb/middleware_usb_host_ehci.cmake)
include(usb/middleware_usb_phy.cmake)

# FATFS
set(CONFIG_USE_middleware_baremetal 1)
include(fatfs/middleware_fatfs.cmake)
include(fatfs/middleware_fatfs_usb.cmake)