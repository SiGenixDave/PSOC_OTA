/*******************************************************************************
* File Name: main.c
*
* Version: 1.50
*
* Description:
*  Simple BLE example project that demonstrates how to configure and use
*  Cypress's BLE component APIs and application layer callback. Device
*  Information service is used as an example to demonstrate configuring
*  BLE service characteristics in the BLE component.
*
* References:
*  BLUETOOTH SPECIFICATION Version 4.1
*
* Hardware Dependency:
*  CY8CKIT-042 BLE
*
********************************************************************************
* Copyright 2014-2016, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/

#include <project.h>
#include "options.h"
#include <stdio.h>
#include "debug.h"
#include "ota_mandatory.h"
#include "ota_optional.h"

#define LED_ON              (0u)
#define LED_OFF             (1u)

static void PrintProjectHeader(void);

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  Starts the bootloader component.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
int main()
{
    uint32_t wakeupReason = CySysPmGetResetReason();
    
    #if (DEBUG_UART_ENABLED == YES)
        UART_Start();
    #endif /* (DEBUG_UART_ENABLED == YES) */

    if (wakeupReason == CY_PM_RESET_REASON_WAKEUP_HIB)
    {
        DBG_PRINT_TEXT("\r\n");
        DBG_PRINT_TEXT("\r\n");
        DBG_PRINT_TEXT(">\r\n");
        DBG_PRINT_TEXT("> Bootloader Wakeup reason: From Hibernate\r\n");
        DBG_PRINT_TEXT(">\r\n");
        DBG_PRINT_TEXT("\r\n"); 
        DBG_PRINT_TEXT("\r\n");    
        
        /* Schedule Bootloadable to start after a reset, no need to check integrity of the 
           bootloadable */
        Bootloader_SET_RUN_TYPE(Bootloader_SCHEDULE_BTLDB);
        CySoftwareReset();    
        /* Code does not advance past here after a wake from hibernate */
    }
    
    GREEN_LED_Write(LED_ON);
    RED_LED_Write(LED_ON);
    
    CyGlobalIntEnable;
    
    
    PrintProjectHeader();

	CyBle_AesCcmInit();    
    
    wakeupReason = CySysPmGetResetReason();    
    Bootloader_Start();
    

    
    for(;;)
    {
        /* Should newer get here. */
    }
}


/*******************************************************************************
* Function Name: PrintProjectHeader()
********************************************************************************
* Summary:
*  Prints project header to UART.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void PrintProjectHeader()
{
    DBG_PRINT_TEXT("\r\n");
    DBG_PRINT_TEXT("\r\n");
    DBG_PRINT_TEXT(">\r\n");
    DBG_PRINT_TEXT("> BLE_OTA_External_Memory_Bootloader Application Started\r\n");
    DBG_PRINT_TEXT("> Version: 1.50\r\n");    
    DBG_PRINT_TEXT("> Compile Date and Time : " __DATE__ " " __TIME__ "\r\n");
    DBG_PRINT_TEXT(">\r\n");
    DBG_PRINT_TEXT("\r\n"); 
    DBG_PRINT_TEXT("\r\n");
    
    return;
}

/* [] END OF FILE */
