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

#include "main.h"

CYBLE_CONN_HANDLE_T connHandle;

static void BootloaderModeEntry(void);
static void InitInterrupts(void);

/*******************************************************************************
* Function Name: main()
********************************************************************************
* Summary:
*  Main function for the project.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Theory:
*  The function starts BLE and UART components.
*  This function process all BLE events and also implements the low power 
*  functionality.
*
*******************************************************************************/
int main()
{
    /* Clean flags */
    bootloadingMode = 0u;
    bootloadingModeReq = 0u;
    packetRXFlag = 0u;

    if (CySysPmGetResetReason() == CY_PM_RESET_REASON_WAKEUP_HIB)
    {
        DBG_PRINT_TEXT("\r\n");
        DBG_PRINT_TEXT("\r\n");
        DBG_PRINT_TEXT(">\r\n");
        DBG_PRINT_TEXT("> Bootloadable Wakeup reason: From Hibernate\r\n");
        DBG_PRINT_TEXT(">\r\n");
        DBG_PRINT_TEXT("\r\n"); 
        DBG_PRINT_TEXT("\r\n");    
    }    
    
    PrintProjectHeader();

    CyGlobalIntEnable;

    /* Start BLE component */
    CyBle_Start(AppCallBack);
    
    /* Start and configure interrupts */
    InitInterrupts();
    
    /* Initialization of encryption in BLE stack if it is enabled */
    #if (ENCRYPTION_ENABLED == YES)
        CR_Initialization();
    #endif /*(ENCRYPTION_ENABLED == YES)*/
    
    /* Disable (hide) bootloader service */
    ChangeBootloaderServiceState(DISABLE_BTS);
    
    while(1u == 1u)
    {
        /* CyBle_ProcessEvents() allows BLE stack to process pending events */
        CyBle_ProcessEvents();

        /* To achieve low power in the device */
        LowPowerImplementation();

        /* Verify if SW2 button is pressed. If so - enable the bootloader service */
        BootloaderModeEntry();
    }
}


/*******************************************************************************
* Function Name: AppCallBack()
********************************************************************************
*
* Summary:
*   This finction handles events that are generated by BLE stack.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
void AppCallBack(uint32 event, void* eventParam)
{
    CYBLE_API_RESULT_T apiResult;
    CYBLE_GAP_BD_ADDR_T localAddr;
    CYBLE_GAP_CONN_UPDATE_PARAM_T connUpdateParam;
    uint32  i = 0u;
   
    switch (event)
    {
        /**********************************************************
        *                       General Events
        ***********************************************************/
        case CYBLE_EVT_STACK_ON: /* This event received when component is Started */
            /* Enter into discoverable mode so that remote can search it. */
            apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            if(apiResult != CYBLE_ERROR_OK)
            {
                DBG_PRINTF("StartAdvertisement API Error: %d \r\n", apiResult);
            }
            DBG_PRINTF("Bluetooth On, StartAdvertisement with addr: ");
            localAddr.type = 0u;
            CyBle_GetDeviceAddress(&localAddr);
            for(i = CYBLE_GAP_BD_ADDR_SIZE; i > 0u; i--)
            {
                DBG_PRINTF("%2.2x", localAddr.bdAddr[i-1]);
            }
            DBG_PRINTF("\r\n");
            break;
        case CYBLE_EVT_HARDWARE_ERROR:    /* This event indicates that some internal HW error has occurred. */
            DBG_PRINTF("CYBLE_EVT_HARDWARE_ERROR\r\n");
            break;
            

        /**********************************************************
        *                       GAP Events
        ***********************************************************/
        case CYBLE_EVT_GAP_AUTH_REQ:
            DBG_PRINTF("EVT_AUTH_REQ: security=%x, bonding=%x, ekeySize=%x, err=%x \r\n",
                (*(CYBLE_GAP_AUTH_INFO_T *)eventParam).security,
                (*(CYBLE_GAP_AUTH_INFO_T *)eventParam).bonding,
                (*(CYBLE_GAP_AUTH_INFO_T *)eventParam).ekeySize,
                (*(CYBLE_GAP_AUTH_INFO_T *)eventParam).authErr);
            break;
        case CYBLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:
            DBG_PRINTF("EVT_GAP_KEYINFO_EXCHNGE_CMPLT \r\n");
            break;
        case CYBLE_EVT_GAP_AUTH_COMPLETE:
            DBG_PRINTF("AUTH_COMPLETE\r\n");
            break;
        case CYBLE_EVT_GAP_AUTH_FAILED:
            DBG_PRINTF("EVT_AUTH_FAILED: %x \r\n", *(uint8 *)eventParam);
            break;
        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            DBG_PRINTF("EVT_GAP_DEVICE_CONNECTED: %d \r\n", connHandle.bdHandle);
            if (((*(CYBLE_GAP_CONN_PARAM_UPDATED_IN_CONTROLLER_T *)eventParam).connIntv > 0x0006u) && \
                                                        (bootloadingMode == 1u))
            {
                /* If connection settings do not match expected ones - request parameter update */
                connUpdateParam.connIntvMin   = 0x0006u;
                connUpdateParam.connIntvMax   = 0x0006u;
                connUpdateParam.connLatency   = 0x0000u;
                connUpdateParam.supervisionTO = 0x0064u;
                apiResult = CyBle_L2capLeConnectionParamUpdateRequest(cyBle_connHandle.bdHandle, &connUpdateParam);
                DBG_PRINTF("CyBle_L2capLeConnectionParamUpdateRequest API: 0x%2.2x \r\n", apiResult);
            }
            LED_WRITE_MACRO(LED_OFF);
            break;
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            DBG_PRINTF("EVT_GAP_DEVICE_DISCONNECTED\r\n");
            apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            if(apiResult != CYBLE_ERROR_OK)
            {
                DBG_PRINTF("StartAdvertisement API Error: %d \r\n", apiResult);
            }
            break;
        case CYBLE_EVT_GAP_ENCRYPT_CHANGE:
            DBG_PRINTF("EVT_GAP_ENCRYPT_CHANGE: %x \r\n", *(uint8 *)eventParam);
            break;
        case CYBLE_EVT_GAPC_CONNECTION_UPDATE_COMPLETE:
            DBG_PRINTF("EVT_CONNECTION_UPDATE_COMPLETE: %x \r\n", *(uint8 *)eventParam);
            break;
        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            if(CYBLE_STATE_DISCONNECTED == CyBle_GetState())
            {   
                /* Fast and slow advertising period complete, go to low power  
                 * mode (Hibernate mode) and wait for an external
                 * user event to wake up the device again */
                DBG_PRINTF("Entering low power mode...\r\n");
                Bootloading_LED_Write(LED_ON);
                Advertising_LED_1_Write(LED_ON);
                Advertising_LED_2_Write(LED_ON);
                Bootloader_Service_Activation_ClearInterrupt();
                Wakeup_Interrupt_ClearPending();
                Wakeup_Interrupt_Start();
                CySysPmHibernate();
            }
            break;

            
        /**********************************************************
        *                       GATT Events
        ***********************************************************/
        case CYBLE_EVT_GATTS_WRITE_REQ:
            DBG_PRINTF("EVT_GATT_WRITE_REQ: %x = ",((CYBLE_GATTS_WRITE_REQ_PARAM_T *)eventParam)->handleValPair.attrHandle);
            break;
        case CYBLE_EVT_GATT_CONNECT_IND:
            connHandle = *(CYBLE_CONN_HANDLE_T *)eventParam;
            break;
        case CYBLE_EVT_GATT_DISCONNECT_IND:
            connHandle.bdHandle = 0;
            break;
        case CYBLE_EVT_GATTS_WRITE_CMD_REQ:
            DBG_PRINTF("CYBLE_EVT_GATTS_WRITE_CMD_REQ\r\n");
            break;
        case CYBLE_EVT_GATTS_PREP_WRITE_REQ:
            (void)CyBle_GattsPrepWriteReqSupport(CYBLE_GATTS_PREP_WRITE_NOT_SUPPORT);
            DBG_PRINTF("CYBLE_EVT_GATTS_PREP_WRITE_REQ\r\n");
            break;
        case CYBLE_EVT_HCI_STATUS:
            DBG_PRINTF("CYBLE_EVT_HCI_STATUS\r\n");
        default:
            break;
        }
}


/*******************************************************************************
* Function Name: BootloaderModeEntry()
********************************************************************************
* Summary:
* This function in case if key press event was detected - debounces it and
* switches to bootloader emulator.
*
* Parameters:
* None
*
* Return:
* None
*
*******************************************************************************/
static void BootloaderModeEntry(void)
{
    if ((Bootloader_Service_Activation_Read() == 0u) || (bootloadingModeReq == 1u))
    {
        CyDelay(100u);
        
        if ((Bootloader_Service_Activation_Read() == 0u) || (bootloadingModeReq == 1u))
        {
            DBG_PRINTF("Bootloader service activated!\r\n");
            ChangeBootloaderServiceState(ENABLE_BTS);

            LED_WRITE_MACRO(LED_OFF);
            bootloadingMode = 1u;
            bootloadingModeReq = 0u;

            /* Force client to rediscover services in range of bootloader service */
            WriteAttrServChanged();

            BootloaderEmulator_Start();
        }
    }
}


/*******************************************************************************
* Function Name: InitInterrupts()
********************************************************************************
* Summary:
* Initializes and starts interruprs used by the project.
*
* Parameters:
* None
*
* Return:
* None
*
*******************************************************************************/
static void InitInterrupts(void)
{
    Bootloader_Service_Activation_ClearInterrupt();
    Wakeup_Interrupt_ClearPending();
    Wakeup_Interrupt_StartEx(Wakeup_Interrupt_Bootloader);
    WDT_Start();
}

/* [] END OF FILE */

