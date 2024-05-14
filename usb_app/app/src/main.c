/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/irq.h>

#include "board.h"
#include "usb_host_config.h"
#include "usb_host.h"
#include "fsl_device_registers.h"
#include "fsl_common.h"

#if ((!USB_HOST_CONFIG_KHCI) && (!USB_HOST_CONFIG_EHCI) && (!USB_HOST_CONFIG_OHCI) && (!USB_HOST_CONFIG_IP3516HS))
#error Please enable USB_HOST_CONFIG_KHCI, USB_HOST_CONFIG_EHCI, USB_HOST_CONFIG_OHCI, or USB_HOST_CONFIG_IP3516HS in file usb_host_config.
#endif

#define HOST_ECHO 1

#define USB_THREAD_STACK_SIZE 4096
#define USB_THREAD_PRIOROTY 2

#include "usb_phy.h"
#include "usb_host_msd.h"
#include "host_msd_fatfs.h"
#include "app.h"
#include "board.h"
/*!
 * @brief host callback function.
 *
 * device attach/detach callback function.
 *
 * @param deviceHandle        device handle.
 * @param configurationHandle attached device's configuration descriptor information.
 * @param eventCode           callback event code, please reference to enumeration host_event_t.
 *
 * @retval kStatus_USB_Success              The host is initialized successfully.
 * @retval kStatus_USB_NotSupported         The application don't support the configuration.
 */
static usb_status_t USB_HostEvent(usb_device_handle deviceHandle,
                                  usb_host_configuration_handle configurationHandle,
                                  uint32_t eventCode);
static void USB_HostApplicationInit(void);
extern void USB_HostClockInit(void);
extern void USB_HostIsrEnable(void);
extern void USB_HostTaskFn(void *param);



/*******************************************************************************
 * Variables
 ******************************************************************************/

K_THREAD_STACK_DEFINE(usb_thread_stack, USB_THREAD_STACK_SIZE);
struct k_thread usb_thread_data;

/*! @brief USB host msd fatfs instance global variable */
extern usb_host_msd_fatfs_instance_t g_MsdFatfsInstance;
usb_host_handle g_HostHandle;
static k_tid_t usb_thread_id;

/******************************************************************************/
/* Functions */
/******************************************************************************/

/******************************************************************************/
void usb_thread(void *p1, void *p2, void*p3);

/******************************************************************************/
void dynamic_usb_isr(const void *param)
{
    USB_HostEhciIsrFunction(g_HostHandle);
}

/******************************************************************************/

int main(void)
{
	USB_HostApplicationInit();

    usb_thread_id = k_thread_create(&usb_thread_data, usb_thread_stack,
                                    K_THREAD_STACK_SIZEOF(usb_thread_stack),
                                    usb_thread,
                                    NULL, NULL, NULL,
                                    USB_THREAD_PRIOROTY,
                                    0,
                                    K_NO_WAIT);

    while (1) {
        k_msleep(100);
	}

	return 0;
}

/*******************************************************************************/
void USB_HostClockInit(void)
{
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };

    if (CONTROLLER_ID == kUSB_ControllerEhci0)
    {
        CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
        CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, 480000000U);
    }
    else
    {
        CLOCK_EnableUsbhs1PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
        CLOCK_EnableUsbhs1Clock(kCLOCK_Usb480M, 480000000U);
    }
    usb_status_t stat = USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, &phyConfig);
    assert(stat == kStatus_USB_Success);
}

void USB_HostIsrEnable(void)
{
    uint8_t irqNumber;

    uint8_t usbHOSTEhciIrq[] = USBHS_IRQS;
    irqNumber                = usbHOSTEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];

    irq_connect_dynamic(irqNumber, USB_HOST_INTERRUPT_PRIORITY, dynamic_usb_isr, NULL, 0);
    irq_enable(irqNumber);
}


/*******************************************************************************/
void USB_HostTaskFn(void *param)
{
    USB_HostEhciTaskFunction(param);
}

/*******************************************************************************/
/*!
 * @brief USB isr function.
 */

#if ((defined USB_HOST_CONFIG_COMPLIANCE_TEST) && (USB_HOST_CONFIG_COMPLIANCE_TEST))
extern usb_status_t USB_HostTestEvent(usb_device_handle deviceHandle,
                                      usb_host_configuration_handle configurationHandle,
                                      uint32_t eventCode);
#endif

static usb_status_t USB_HostEvent(usb_device_handle deviceHandle,
                                  usb_host_configuration_handle configurationHandle,
                                  uint32_t eventCode)
{
#if ((defined USB_HOST_CONFIG_COMPLIANCE_TEST) && (USB_HOST_CONFIG_COMPLIANCE_TEST))
    usb_host_configuration_t *configuration;
    usb_status_t status1;
    usb_status_t status2;
    uint8_t interfaceIndex = 0;
#endif
    usb_status_t status = kStatus_USB_Success;
    switch (eventCode & 0x0000FFFFU)
    {
        case kUSB_HostEventAttach:
#if ((defined USB_HOST_CONFIG_COMPLIANCE_TEST) && (USB_HOST_CONFIG_COMPLIANCE_TEST))
            status1 = USB_HostTestEvent(deviceHandle, configurationHandle, eventCode);
            status2 = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
            if ((status1 == kStatus_USB_NotSupported) && (status2 == kStatus_USB_NotSupported))
            {
                status = kStatus_USB_NotSupported;
            }
#else
            status = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
#endif
            break;

        case kUSB_HostEventNotSupported:
#if ((defined USB_HOST_CONFIG_COMPLIANCE_TEST) && (USB_HOST_CONFIG_COMPLIANCE_TEST))
            configuration = (usb_host_configuration_t *)configurationHandle;
            for (interfaceIndex = 0; interfaceIndex < configuration->interfaceCount; ++interfaceIndex)
            {
                if (((usb_descriptor_interface_t *)configuration->interfaceList[interfaceIndex].interfaceDesc)
                        ->bInterfaceClass == 9U) /* 9U is hub class code */
                {
                    break;
                }
            }

            if (interfaceIndex < configuration->interfaceCount)
            {
                usb_echo("unsupported hub\r\n");
            }
            else
            {
                usb_echo("Unsupported Device\r\n");
            }
#else
            usb_echo("Unsupported Device\r\n");
#endif
            break;

        case kUSB_HostEventEnumerationDone:
#if ((defined USB_HOST_CONFIG_COMPLIANCE_TEST) && (USB_HOST_CONFIG_COMPLIANCE_TEST))
            status1 = USB_HostTestEvent(deviceHandle, configurationHandle, eventCode);
            status2 = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
            if ((status1 != kStatus_USB_Success) && (status2 != kStatus_USB_Success))
            {
                status = kStatus_USB_Error;
            }
#else
            status = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
#endif
            break;

        case kUSB_HostEventDetach:
#if ((defined USB_HOST_CONFIG_COMPLIANCE_TEST) && (USB_HOST_CONFIG_COMPLIANCE_TEST))
            status1 = USB_HostTestEvent(deviceHandle, configurationHandle, eventCode);
            status2 = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
            if ((status1 != kStatus_USB_Success) && (status2 != kStatus_USB_Success))
            {
                status = kStatus_USB_Error;
            }
#else
            status = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
#endif
            break;

        case kUSB_HostEventEnumerationFail:
            usb_echo("enumeration failed\r\n");
            break;

        default:
            break;
    }
    return status;
}

/******************************************************************************/
static void USB_HostApplicationInit(void)
{
    usb_status_t status = kStatus_USB_Success;

    USB_HostClockInit();

#if ((defined FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    status = USB_HostInit(CONTROLLER_ID, &g_HostHandle, USB_HostEvent);
    if (status != kStatus_USB_Success)
    {
        usb_echo("host init error\r\n");
        return;
    }
    USB_HostIsrEnable();

    usb_echo("host init done\r\n");
};

/******************************************************************************/
void usb_thread(void *p1, void *p2, void*p3)
{
    usb_echo("USB Thread started\r\n");

	while (1) {
        USB_HostTaskFn(g_HostHandle);
        USB_HostMsdTask(&g_MsdFatfsInstance);
	}
}