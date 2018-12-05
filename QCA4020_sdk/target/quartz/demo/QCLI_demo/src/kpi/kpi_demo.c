/*
 * Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 */
// Copyright (c) 2018 Qualcomm Technologies, Inc.
// All rights reserved.
// Redistribution and use in source and binary forms, with or without modification, are permitted (subject to the limitations in the disclaimer below) 
// provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, 
// this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// Neither the name of Qualcomm Technologies, Inc. nor the names of its contributors may be used to endorse or promote products derived 
// from this software without specific prior written permission.
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, 
// BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "qcli_api.h"
#include "qapi_wlan.h"
#include "qapi_status.h"
#include "qapi_fs.h"
#include "qapi_crypto.h"
#include "qapi_ssl.h"
#include "qurt_signal.h"  
#include "qurt_timer.h"
#include "qurt_error.h"
#include "util.h"
#include "qapi_netbuf.h"
#include "qapi_socket.h"
#include "qapi_netservices.h"
#include "qapi_ns_gen_v4.h"
#include "qapi_ns_gen_v6.h"
#include "qapi_firmware_upgrade.h"
#include "qapi_delay.h"
#include "qapi_fatal_err.h"
#include "qapi_reset.h"
#include "qapi_tlmm.h"
#include "qapi_securefs.h"
#include "qapi_slp.h"
#include "plugins/ftp/ota_ftp.h"
#include "plugins/http/ota_http.h"
#include "plugins/zigbee/ota_zigbee.h"
#include "plugins/ble/ota_ble.h"
#include "kpi_demo.h"


/*
 * This file contains the command handlers for kpi demo
 *
 */
QCLI_Command_Status_t dummy_cmd_2(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t m4_boot_time(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t wlan_boot_time(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t wlan_test_setup_command(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t wlan_storerecall_test(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t wlan_cummulative_throughput_test(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t kpi_demo_cleanup(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t wlan_connection_time_test(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t wlan_connection_time_packet_test(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t wlan_device_discovery_simulate_new_device(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t wlan_device_discovery_simulate_smartphone(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t kpi_demo_fw_update(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t kpi_demo_securefs(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Group_Handle_t qcli_kpi_handle; /* Handle for kpi demo Command Group. */

extern uint32_t Custom_Platform_Get_32Khz_Ticks(void);
extern qbool_t PAL_Uart_Deinit(void);
extern qbool_t PAL_Uart_Init(void);

const QCLI_Command_t kpi_cmd_list[] =
{
    {m4_boot_time, true, "m4_boot_time", "Usage: m4_boot_time (no options) \n", "Display M4 boot time"},
    {wlan_boot_time, true, "wlan_boot_time", "Usage: wlan_boot_time (no options) \n", "Display WLAN boot time"},
    {wlan_test_setup_command, true, "wlan_test_setup", "Usage: wlan_test_setup ssid wpa2_key operating_channel \n", "Setup kpi demo test parameters"},
    {wlan_storerecall_test, true, "wlan_storerecall_test", "Usage: wlan_storerecall_test security_type \n", "Display total storerecall time & breakdown"},
    {wlan_cummulative_throughput_test, true, "wlan_cummulative_throughput_test", "Usage: wlan_cummulative_throughput_test ip_address port rate security_type\n", "Measure power with cumulative throughput test"},
    {wlan_connection_time_packet_test, true, "wlan_connection_time_packet_test", "Usage: wlan_connection_time_packet_test ip_address port security_type ip_type packet_count  \n", "Display time breakdown for connection & ip packet"},
    {wlan_connection_time_test, true, "wlan_connection_time_test", "Usage: wlan_connection_time_test security_type \n", " Display time breakdown for AP connection \n"},
    {wlan_device_discovery_simulate_new_device, true, "wlan_device_discovery_simulate_new_device", "Usage: wlan_device_discovery_simulate_new_device (no options) ", " Command to simulate a new device for onboarding test "},
    {wlan_device_discovery_simulate_smartphone, true, "wlan_device_discovery_simulate_smartphone", "Usage: wlan_device_discovery_simulate_smartphone ssid passcode", " Command to simulate a smartphone device for onboarding test" },
    {kpi_demo_fw_update, true, "kpi_demo_firmware_update", "Usage: plugin_type interface server_address filename param(optional)" , "Command to do fw upgrade time measurement"},
    {kpi_demo_cleanup, true, "kpi_demo_cleanup", "Usage: kpi_demo_cleanup (no options) \n", "Cleanup assigned memory for kpi demo"},
    {kpi_demo_securefs, true, "kpi_securefs", "Usage: kpi_securefs test_type password(optional) \n", "Secure FS KPI tests"},
    {dummy_cmd_2,false,"dummy",NULL,NULL},
};

const QCLI_Command_Group_t kpi_cmd_group =
{
    "KPI_Demo",              /* Group_String: will display cmd prompt as "KPI Demo> " */
    sizeof(kpi_cmd_list)/sizeof(kpi_cmd_list[0]),   /* Command_Count */
    kpi_cmd_list        /* Command_List */
};

kpi_demo_ctx *kpi_ctx; /* main context for the whole demo */
qbool_t kpi_quit = 0;
uint32_t deviceId = 1; /* Device ID 1 is specifically tied to station mode */
char *ssid_onboarding = "kpi_demo_ap"; /* default onboarding SSID */
char *password_onboarding = "12345678"; /* default onboarding passphrase */
static qapi_TLMM_Config_t kpiTimeCheck;
static qapi_GPIO_ID_t     kpi_time_check_gpio_id;

/* The demo assumes a certain things to make it simple to 
 * expand based on needs. This code is expected to do a certain
 * set of things but its expandable to fit specific uses cases
 * as required. The framework can be used for any form of
 * KPI (Key Performance Indicator) test cases.
 * 1. DeviceID are used to support multiple virtual devices.
 *    By default, DeviceID 0 = SoftAP, 1 = Station
 * 2. The setup command can be used to setup any variable
 *    parameters.
 * 3. Some error handling/corner use cases might be missing
 *    as the goal of demo is to do performance testing rather
 *    than stability testing.
 */


/*****************************************************************************
 * This function is used to register the Fs Command Group with QCLI.
 *****************************************************************************/
void Initialize_KPI_Demo(void)
{
    /* Attempt to reqister the Command Groups with the qcli framework.*/
    qcli_kpi_handle = QCLI_Register_Command_Group(NULL, &kpi_cmd_group);
    if (qcli_kpi_handle)
    {
        QCLI_Printf(qcli_kpi_handle, "KPI Demo Registered\n");
    }

    return;
}

/* Empty function to keep the CLI happy */
QCLI_Command_Status_t dummy_cmd_2(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    return QCLI_STATUS_SUCCESS_E;  
}

/*
 * kpi_fw_upgrade_callback should be used with any firmware upgrade
 * related QAPI calls. This is the callback function which is invoked
 * from fw upgrade QAPI's. This can be extended to support specific
 * operations. Please refer to OTA documentation in programmers guide
 * for more information.
 */
void kpi_fw_upgrade_callback(int32_t state, uint32_t status)
{
    QCLI_Printf(qcli_kpi_handle, "kpi_fw_upgrade_callback: %d %d\r\n", state, status);
    return;
}


/*
 * kpi_wlan_callback_handler should be used with
 * any wlan related qapi's which requires a callback
 * function. This function will be typically be
 * invoked when certain driver functions are completed.
 * Other reference implementations can be found in
 * wifi demo.
 */
void kpi_wlan_callback_handler(uint8_t  deviceId, 
                               uint32_t cbId, 
                               void *pApplicationContext,
                               void     *payload,
                               uint32_t payload_Length)
{

    switch(cbId)
    {

     /* indicates connection notification from the driver */
     case QAPI_WLAN_CONNECT_CB_E:  
     {
         qapi_WLAN_Connect_Cb_Info_t *cxnInfo  = (qapi_WLAN_Connect_Cb_Info_t *)(payload);

         /* Implies the connection notification was for SoftAP mode */
         if(deviceId == 0)
         {
             qurt_signal_set(&kpi_ctx->wlan_kpi_event, KPI_STA_CONN_EVENT_MASK);
         }
          /* indicates a STA disconnection event */
         else if(deviceId == 1 && cxnInfo->value == FALSE)
         {
             qurt_signal_set(&kpi_ctx->wlan_kpi_event, KPI_STA_DISCONN_EVENT_MASK);
         }
         /* Implies the connection notification is for station mode */
         else if(deviceId == 1) {

             if(kpi_ctx->wpa2_psk != NULL && cxnInfo->value == FOUR_WAY_HANDSHAKE_COMPLETION) {
                 QCLI_Printf(qcli_kpi_handle,"4-way Handshake successful \n");
                 qurt_signal_set(&kpi_ctx->wlan_kpi_event, KPI_CONNECT_EVENT_MASK);
             }
             else if(kpi_ctx->wpa2_psk != NULL && cxnInfo->value == TRUE)
             {
                 qurt_signal_set(&kpi_ctx->wlan_kpi_event, KPI_CONNECT_ASSOC_MASK);
             }
             else if(kpi_ctx->wpa2_psk == NULL) /*open mode */
             {
                 QCLI_Printf(qcli_kpi_handle,"WLAN connected \n");
                 qurt_signal_set(&kpi_ctx->wlan_kpi_event, KPI_CONNECT_EVENT_MASK);
             }
         } 
     }
     break;
     /* case to handle any fatal, this will result in a system crash */
     case QAPI_WLAN_ERROR_HANDLER_CB_E:
     {
         /* currently no information is provided on fatal error, hence left blank */
         QCLI_Printf(qcli_kpi_handle,"Fatal error occured. Error= ");

         /* take necessary action based on error */
         QAPI_FATAL_ERR(0,0,0);
       break;
     }
     /* 
      * Indicates, WLAN chip is out of suspend mode
      * This event only occur when the WLAN chip is
      * explicity put to suspend mode
      */
     case QAPI_WLAN_RESUME_HANDLER_CB_E:
     {
         QCLI_Printf(qcli_kpi_handle, "\r\n WLAN FIRMWARE RESUME Completed");
         qurt_signal_set(&kpi_ctx->wlan_kpi_event, KPI_RESUME_DONE_EVENT_MASK);
       break;
     }
     /* indicates completion of the scan results, use for onboarding uses cases */
     case QAPI_WLAN_SCAN_COMPLETE_CB_E:
     {
        qapi_WLAN_Scan_List_t scan_results;
        int32_t error = 0, i = 0;
        qapi_WLAN_BSS_Scan_Info_t *list = NULL;

        QCLI_Printf(qcli_kpi_handle, "\r\n Scan completion callback \n");

        scan_results.scan_List = malloc((sizeof(qapi_WLAN_BSS_Scan_Info_t) * __QAPI_MAX_SCAN_RESULT_ENTRY));
        scan_results.num_Scan_Entries = __QAPI_MAX_SCAN_RESULT_ENTRY;
        error = qapi_WLAN_Get_Scan_Results(deviceId, (qapi_WLAN_BSS_Scan_Info_t *)(scan_results.scan_List), (int16_t *)&(scan_results.num_Scan_Entries));
        if (error != QAPI_OK){
            QCLI_Printf(qcli_kpi_handle,"Failed to get scan results.\r\n");
            free(scan_results.scan_List);
            kpi_quit = TRUE;

            /* signal the waiting function */
            qurt_signal_set(&kpi_ctx->wlan_kpi_event, KPI_ONBOARDING_SSID_FOUND_MASK);
        }
        else 
        {
            list = (qapi_WLAN_BSS_Scan_Info_t *)(scan_results.scan_List);

            /* comparision to see if the onboarding SSID is found in scan results */
            for(i=0;i< scan_results.num_Scan_Entries;i++)
            {
                if(memcmp(ssid_onboarding,list[i].ssid,list[i].ssid_Length) == 0) {
                    kpi_ctx->is_onboarding_ssid_found = TRUE;
                    break;
                }
            }

            free(scan_results.scan_List);

            kpi_quit = FALSE;

            /* signal the waiting function */
            qurt_signal_set(&kpi_ctx->wlan_kpi_event, KPI_ONBOARDING_SSID_FOUND_MASK);

        }

         break;
      }

     default:
         break;
    } 

    return;

}

/* 
 * kpi_get_time - generic function to get
 * current time.
 */
uint32_t kpi_get_time(time_struct_t *time)
{
    uint32_t ticks, ms;

    ticks = (uint32_t)qurt_timer_get_ticks();
    ms = (uint32_t)qurt_timer_convert_ticks_to_time(ticks, QURT_TIME_MSEC);

    if (time)
    {
        time->seconds = ms / 1000;
        time->milliseconds = ms % 1000;
        time->ticks = Custom_Platform_Get_32Khz_Ticks();
    }

    return ms;
}

/*
 * kpi_ipconfig_dhcpc_success_cb - A callback regsitered
 * with qapi dhcp api. This callback is called on a
 * successful dhcp completion for a client.
 */
static int32_t kpi_ipconfig_dhcpc_success_cb(uint32_t addr, uint32_t mask, uint32_t gw)
{
    char ip_str[20];
    char mask_str[20];
    char gw_str[20];

    QCLI_Printf(qcli_kpi_handle, "DHCP SUCCESS: IP=%s  Subnet Mask=%s  Gateway=%s\n",
            inet_ntop(AF_INET, &addr, ip_str, sizeof(ip_str)),
            inet_ntop(AF_INET, &mask, mask_str, sizeof(mask_str)),
            inet_ntop(AF_INET, &gw, gw_str, sizeof(gw_str)));

    qurt_signal_set(&kpi_ctx->wlan_kpi_event, KPI_DHCP_EVENT_MASK);

    kpi_ctx->params.tx_params.source_ipv4_addr = addr;
    kpi_ctx->dest_ip = gw;
    memcpy(kpi_ctx->ip, ip_str, 20);
 
    return 0;
}

/* 
 * kpi_ipconfig_dhcps_success_cb - This function indicates 
 * a successful ip address was assigned to a station connected
 * to the device softAP.
 */
static int32_t kpi_ipconfig_dhcps_success_cb(uint8_t *macaddr, uint32_t ipaddr)
{
    if (macaddr != NULL)
    {
        char ip_str[20];

        QCLI_Printf(qcli_kpi_handle, "DHCP Client IP Success: Client IP=%s  Client MAC=%02x:%02x:%02x:%02x:%02x:%02x\n",
                inet_ntop(AF_INET, &ipaddr, ip_str, sizeof(ip_str)),
                macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
    }

    /* 
     * connected device IP address, assumption is that only one device
     * should connect to the AP given that this is a controlled test.
     */
    kpi_ctx->device_ipv4_addr = ipaddr;

    return 0;
}

void kpi_gpio_setup()
{

    uint32_t ret;

    kpiTimeCheck.pin  = KPI_TIME_CHECK_GPIO;
    kpiTimeCheck.func = 0;
    kpiTimeCheck.dir  = QAPI_GPIO_OUTPUT_E; 
    kpiTimeCheck.pull = QAPI_GPIO_NO_PULL_E;
    kpiTimeCheck.drive= QAPI_GPIO_2MA_E; 

    ret = qapi_TLMM_Get_Gpio_ID( &kpiTimeCheck, &kpi_time_check_gpio_id);

    if (ret == QAPI_OK)
    {
        ret = qapi_TLMM_Config_Gpio(kpi_time_check_gpio_id, &kpiTimeCheck);

        if(ret != QAPI_OK)
            qapi_TLMM_Release_Gpio_ID(&kpiTimeCheck, kpi_time_check_gpio_id);
    }
    else
    {
        qapi_TLMM_Release_Gpio_ID(&kpiTimeCheck, kpi_time_check_gpio_id);
    }
}

void kpi_gpio_toggle(uint32_t type){

    uint32_t ret = QAPI_OK;

    if(type == KPI_GPIO_HIGH) {
        ret = qapi_TLMM_Drive_Gpio(kpi_time_check_gpio_id, kpiTimeCheck.pin, QAPI_GPIO_HIGH_VALUE_E);
    }
    else if(type == KPI_GPIO_LOW) {
        ret = qapi_TLMM_Drive_Gpio(kpi_time_check_gpio_id, kpiTimeCheck.pin, QAPI_GPIO_LOW_VALUE_E);
    }

    if(ret != QAPI_OK)
        qapi_TLMM_Release_Gpio_ID(&kpiTimeCheck, kpi_time_check_gpio_id);
}

void kpi_gpio_release(){

    qapi_TLMM_Release_Gpio_ID(&kpiTimeCheck, kpi_time_check_gpio_id);

}

QCLI_Command_Status_t kpi_demo_securefs(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{

    /* test type */
    /* type 1 - create file and close it */
    /* type 2 - write a file */
    /* type 3 - read a file */
    /* type 4 - do all the operations listed above */

    /* password is always required */
    /* default file size 1 KB */

    char *filename = "/spinor/kpi_test_file.bin";
    void *securefsCtxtPtr = NULL;
    int oflags = QAPI_FS_O_RDWR | QAPI_FS_O_CREAT;
    char *userPassword = NULL, *read_buf = NULL;
    uint32_t t1=0, t2=0, t3=0, t4=0, i, k, buf_size = KPI_SECUREFS_BUFSIZE;
    int32_t bytes_count = KPI_SECUREFS_BUFSIZE;
    size_t bytes_read, bytes_written;
    uint32_t test_type = KPI_SECUREFS_ALL;
    qapi_Status_t status = QAPI_OK;
    char *data = NULL;
    int32_t multiplier = RTC_CLOCK_MULTIPLIER;
    int32_t divider = RTC_CLOCK_DIVIDER;


    if(Parameter_Count < 2) {
        QCLI_Printf(qcli_kpi_handle,"All parameters are required \n");
        QCLI_Printf(qcli_kpi_handle,"Usage: kpi_securefs test_type password filesize(optional) buffsize(optional) \n");
        QCLI_Printf(qcli_kpi_handle,"Test type: 1= Create File 2=Write File 3=Read File 4=All \n");
        return QCLI_STATUS_SUCCESS_E;
    }

    test_type = Parameter_List[0].Integer_Value;
    userPassword = Parameter_List[1].String_Value;

    if(Parameter_Count >= 3){
        if(Parameter_List[2].Integer_Is_Valid) 
            bytes_count = Parameter_List[2].Integer_Value;
    }

    if(Parameter_Count == 4){
        if(Parameter_List[3].Integer_Is_Valid)
            buf_size = Parameter_List[3].Integer_Value;
    }

    if(buf_size > bytes_count)
    {
        QCLI_Printf(qcli_kpi_handle, "Buffer size cannot be greater than bytes to be written or read \n");
        return QCLI_STATUS_ERROR_E;
    }

    /* setup write data beforehand */
    if(test_type == KPI_SECUREFS_WRITE_FILE || test_type == KPI_SECUREFS_ALL){

       /*max buffer is 1024 bytes */
       data = malloc(buf_size);

       if(data == NULL) {
           QCLI_Printf(qcli_kpi_handle, "Failed allocate buffer, please select a smaller buffer size \n");
           goto kpi_securefs_error;
       }

       for(i = 0; i < buf_size;i++)
       {
           data[i] = i;
       }
    }

    /* create time */
    kpi_gpio_toggle(KPI_GPIO_HIGH);
    t1 = Custom_Platform_Get_32Khz_Ticks();

    status = qapi_Securefs_Open(&securefsCtxtPtr,filename, oflags,(const uint8_t *)userPassword, strlen(userPassword));

    if(status != QAPI_OK) {
            QCLI_Printf(qcli_kpi_handle, "Failed to open the file \n");
            goto kpi_securefs_error;
    }

    t2 = Custom_Platform_Get_32Khz_Ticks();
    kpi_gpio_toggle(KPI_GPIO_LOW);

    if(test_type == KPI_SECUREFS_ALL || test_type == KPI_SECUREFS_WRITE_FILE) {

       k = bytes_count;

       while(k > 0) {

           if(k > buf_size) {

               status = qapi_Securefs_Write(securefsCtxtPtr,data, buf_size, &bytes_written);

               if(status == QAPI_OK && bytes_written == buf_size)
                   k = k - buf_size;
               else { 
                   QCLI_Printf(qcli_kpi_handle, "Error while writing, bytes left to be written %d \n", k);
                   goto kpi_securefs_error;
               }
           }
           else if(k <= buf_size) {

               status = qapi_Securefs_Write(securefsCtxtPtr,data, k, &bytes_written);

               if(status == QAPI_OK && bytes_written == k)
                   k = 0;
               else { 
                   QCLI_Printf(qcli_kpi_handle, "Error while writing, bytes left to be written %d \n", k);
                   goto kpi_securefs_error;
               }
           }

       }

    }

    kpi_gpio_toggle(KPI_GPIO_HIGH);
    t3 = Custom_Platform_Get_32Khz_Ticks();

    if(test_type == KPI_SECUREFS_ALL || test_type == KPI_SECUREFS_READ_FILE) {

      read_buf = malloc(buf_size);

      while(k > 0) {

        if(k > buf_size) {
       
            status = qapi_Securefs_Read(securefsCtxtPtr, read_buf, buf_size, &bytes_read);

            if(status == QAPI_OK && bytes_read == buf_size)
                k = k - buf_size;
            else {
                QCLI_Printf(qcli_kpi_handle, "Error while reading, bytes left to be read %d \n", k);
                goto kpi_securefs_error;
            }
        }
        else if(k <= buf_size) {

            status = qapi_Securefs_Read(securefsCtxtPtr, read_buf, buf_size, &bytes_read);

            if(status == QAPI_OK && bytes_read == buf_size)
                k = 0;
            else {
                QCLI_Printf(qcli_kpi_handle, "Error while reading, bytes left to be read %d \n", k);
                goto kpi_securefs_error;
            }
        }
      }
    }

    if(securefsCtxtPtr != NULL) {
        qapi_Securefs_Close(securefsCtxtPtr);
    }

    t4 = Custom_Platform_Get_32Khz_Ticks();
    kpi_gpio_toggle(KPI_GPIO_HIGH);

    /* print total breakdown of the time taken */
    if(test_type == KPI_SECUREFS_ALL || test_type == KPI_SECUREFS_CREATE_FILE) {
         QCLI_Printf(qcli_kpi_handle,"Securefs file open time = %d ms \n", 
              ((t2 - t1)*multiplier)/divider);
    }

    if(test_type == KPI_SECUREFS_ALL || test_type == KPI_SECUREFS_WRITE_FILE) {
         QCLI_Printf(qcli_kpi_handle,"Securefs time to write input data = %d ms \n", 
              ((t3 - t2)*multiplier)/divider);
    }

    if(test_type == KPI_SECUREFS_ALL || test_type == KPI_SECUREFS_READ_FILE) {
         QCLI_Printf(qcli_kpi_handle,"Securefs time to read given data size = %d ms \n",
             ((t4 - t3)*multiplier)/divider);
    }

   if(test_type == KPI_SECUREFS_ALL) {
         QCLI_Printf(qcli_kpi_handle,"Total time test = %d ms \n",
             ((t4 - t1)*multiplier)/divider);
   }

   if(data != NULL)
       free(data);

   return QCLI_STATUS_SUCCESS_E;

kpi_securefs_error:

   if(securefsCtxtPtr != NULL) {
      qapi_Securefs_Close(securefsCtxtPtr);
   }

   if(data != NULL) {
       free(data);
   }

   QCLI_Printf(qcli_kpi_handle,"Error occured while perform the operation, Status = %d \n", status);

   return QCLI_STATUS_SUCCESS_E;

}


/*
 * m4_boot_time - This function display the 
 * M4 boot time which is the overall boot time
 * for the SOC. The breakdown is currently unavailable and
 * oscilloscope method is preferred for now.
 * please refer to programmer's guide for the same.
 * This function will reboot the device.
 */
QCLI_Command_Status_t m4_boot_time(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{

    /* Display M4 Boot time */
    qapi_System_Reset();

    return QCLI_STATUS_SUCCESS_E;

}

extern uint32_t *g_boot_time_measure;
extern uint32_t *g_wlan_strrcl_time_measure;

/*
 *This function displays the wlan boot time.
 */
QCLI_Command_Status_t wlan_boot_time(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{

   uint32_t temp1, temp2, duration;
   int32_t multiplier = RTC_CLOCK_MULTIPLIER;
   int32_t divider = RTC_CLOCK_DIVIDER;
   int32_t time_elapsed;

   /* enable wlan to measure the wlan boot time */
   QCLI_Printf(qcli_kpi_handle,"Enabling wlan \n");

    if (0 == qapi_WLAN_Enable(QAPI_WLAN_ENABLE_E))
    {
           QCLI_Printf(qcli_kpi_handle, "wlan enabled \n");
    }
    else {
           QCLI_Printf(qcli_kpi_handle, "wlan enabled failed \n");
           return QCLI_STATUS_ERROR_E;
    }
    /* print boot time results */
    QCLI_Printf(qcli_kpi_handle,"Boot Time Results for WLAN");

    temp1 = g_boot_time_measure[WLAN_BOOT_KF_INIT_INDEX];
    temp2 = g_boot_time_measure[WLAN_BOOT_KF_POWER_ON_INDEX];

    if (temp2 < temp1)
    {
        /* Assume the systick wraps around once */
        duration = ~temp1 + 1 + temp2;
    }
    else
    {
        duration = temp2 - temp1;
    }

    time_elapsed = (duration * multiplier)/divider;

    QCLI_Printf(qcli_kpi_handle,"Boot Time: time to power up wlan chip %d ms \n", time_elapsed);

    temp1 = g_boot_time_measure[WLAN_BOOT_KF_POWER_ON_INDEX];
    temp2 = g_boot_time_measure[WLAN_BOOT_KF_FW_DOWNLOAD_INDEX];

    if (temp2 < temp1)
    {
        /* Assume the systick wraps around once */
        duration = ~temp1 + 1 + temp2;
    }
    else
    {
        duration = temp2 - temp1;
    }

    time_elapsed = (duration * multiplier)/divider;

    QCLI_Printf(qcli_kpi_handle,"Boot Time: time to download wlan firmware %d ms \n", time_elapsed);

    temp1 = g_boot_time_measure[WLAN_BOOT_KF_FW_DOWNLOAD_INDEX];
    temp2 = g_boot_time_measure[WLAN_BOOT_KF_WMI_READY_TIME];

    if (temp2 < temp1)
    {
        /* Assume the systick wraps around once */
        duration = ~temp1 + 1 + temp2;
    }
    else
    {
        duration = temp2 - temp1;
    }

    time_elapsed = (duration * multiplier)/divider;

    QCLI_Printf(qcli_kpi_handle,"Boot Time: time between firmware download & wmi_ready, KF Boot Time %d ms \n", time_elapsed);

    temp1 = g_boot_time_measure[WLAN_BOOT_KF_INIT_INDEX];
    temp2 = g_boot_time_measure[WLAN_BOOT_KF_WMI_READY_TIME];

    if (temp2 < temp1)
    {
        /* Assume the systick wraps around once */
        duration = ~temp1 + 1 + temp2;
    }
    else
    {
        duration = temp2 - temp1;
    }

    time_elapsed = (duration * multiplier)/divider;

    QCLI_Printf(qcli_kpi_handle,"Boot Time: Total Boot time %d ms \n", time_elapsed);

    if(0 == qapi_WLAN_Enable(QAPI_WLAN_DISABLE_E))
    {
        QCLI_Printf(qcli_kpi_handle,"Wlan successfully disabled \n");
    }

    return QCLI_STATUS_SUCCESS_E;

}

QCLI_Command_Status_t kpi_demo_cleanup(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{

    if(kpi_ctx->ssid != NULL)
        free(kpi_ctx->ssid);

    if(kpi_ctx->wpa2_psk != NULL)
        free(kpi_ctx->wpa2_psk);

    if(kpi_ctx != NULL)
        free(kpi_ctx);

    kpi_ctx = NULL;

    return QCLI_STATUS_SUCCESS_E;
}

/* 
 * wlan_test_setup_command - This function takes care of setting up
 * basic infrastructure for the KPI demo. Currently it sets following
 * 1. SSID 2. WPA2_PASSPHRASE 3. Operating Channel
 * For future enchancement of this demo, this command should be used
 * for any setup related items.
 *
 */
QCLI_Command_Status_t wlan_test_setup_command(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{

    uint32_t temp_sec_type = 0;

    kpi_ctx = malloc(sizeof(kpi_demo_ctx));

    memset(kpi_ctx, 0, sizeof(kpi_demo_ctx));

    if(Parameter_Count < 2) {
        QCLI_Printf(qcli_kpi_handle,"All parameters are required \n");
        QCLI_Printf(qcli_kpi_handle,"Usage: wlan_test_setup ssid security_type wpa2_key(optional) operating_channel (optional) \n");
        QCLI_Printf(qcli_kpi_handle,"security_type 1=wpa2 0=open security \n");
        return QCLI_STATUS_SUCCESS_E;
    }

    kpi_ctx->ssid = malloc(WLAN_MAX_SSID_LEN);

    if(kpi_ctx->ssid == NULL)
    {
        QCLI_Printf(qcli_kpi_handle," Out of memory \n");
    }

    /* parse command line params */
    memcpy(kpi_ctx->ssid,
        (char *)Parameter_List[0].String_Value,strlen(Parameter_List[0].String_Value)+1);

    /* security type */
    if(Parameter_List[1].Integer_Is_Valid)
    {
        temp_sec_type = Parameter_List[1].Integer_Value;
    }


    if(temp_sec_type == KPI_SEC_MODE_WPA && Parameter_Count > 2) {
        kpi_ctx->wpa2_psk = malloc(WLAN_WPA_KEY_MAX_LEN);

        if(kpi_ctx->wpa2_psk == NULL)
        {
            QCLI_Printf(qcli_kpi_handle," Out of memory \n");
            return QCLI_STATUS_SUCCESS_E;
        }

        memcpy(kpi_ctx->wpa2_psk,
            (char *)Parameter_List[2].String_Value,strlen(Parameter_List[2].String_Value)+1);

        if(Parameter_Count > 3 && Parameter_List[3].Integer_Is_Valid) {
            kpi_ctx->operating_channel = Parameter_List[3].Integer_Value;
        }

    }
    else if (temp_sec_type == KPI_SEC_MODE_OPEN ){
        kpi_ctx->wpa2_psk = NULL;

        if(Parameter_Count > 2 && Parameter_List[2].Integer_Is_Valid) {
            kpi_ctx->operating_channel = Parameter_List[2].Integer_Value;
        }
    }
    else
    {
        QCLI_Printf(qcli_kpi_handle, "Invalid security type or password missing, WPA and Open mode supported \n");
    }

    return QCLI_STATUS_SUCCESS_E;

}

/*
 * wlan_storerecall_test - This function performes the
 * storerecall test and measure time it takes for the 
 * storerecall operation to complete.
 * wlan_test_setup is required to be called.
 * The structure of this function is as follows
 * wlan enable -> connect to AP -> Acquire DHCP
 * wait for one second
 * -> storerecall for 30 seconds -> display data
 * -> storerecall for 150 seconds or longer time -> display data
 * -> test_type 1 - connect time, 2 - connect time & send IP packet time
 */
QCLI_Command_Status_t wlan_storerecall_test(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{ 
    char *interface_name = "wlan1";
    qbool_t if_is_up = FALSE;
    uint32_t temp1, temp2, duration, time_elapsed;
    int32_t multiplier = RTC_CLOCK_MULTIPLIER;
    int32_t divider = RTC_CLOCK_DIVIDER;
    uint32_t signal_set;
    uint32_t long_test_time = 150;
    uint32_t test_type = STORERECALL_TEST_NORMAL;
    uint32_t t1=0, t2=0, t3=0, t4=0;

    if(Parameter_Count < 1) {
        QCLI_Printf(qcli_kpi_handle,"All parameters are required \n");
        QCLI_Printf(qcli_kpi_handle,"Usage: wlan_storerecall_test security_type long_test_time(optional) test_type (optional) port (optional) dest_ip (optional) \n");
        QCLI_Printf(qcli_kpi_handle,"Note: Only open security and WPA2 CCMP is supported in this demo \n");
        QCLI_Printf(qcli_kpi_handle,"0 = open security, 1 = WPA2 Securtiy \n");
        QCLI_Printf(qcli_kpi_handle,"Long test time in seconds \n");
        QCLI_Printf(qcli_kpi_handle,"test_type 1 = wlan connect time test 2 = wlan connect time test and send IP packet test \n");
        QCLI_Printf(qcli_kpi_handle,"Port. The port number to which to establish connection \n");
        QCLI_Printf(qcli_kpi_handle,"dest_ip. The IP address to which to establish connection \n");
        return QCLI_STATUS_SUCCESS_E;
    }

    if(kpi_ctx == NULL)
    {
        QCLI_Printf(qcli_kpi_handle, "Please run the setup command first \n");
        return QCLI_STATUS_SUCCESS_E;
    }

    kpi_ctx->security_mode = Parameter_List[0].Integer_Value;

    if((kpi_ctx->security_mode != KPI_SEC_MODE_OPEN) && (kpi_ctx->security_mode != KPI_SEC_MODE_WPA)) {
        QCLI_Printf(qcli_kpi_handle, "Invalid security type, 0 = open security, 1 = WPA2 Securtiy supported \n");
        return QCLI_STATUS_SUCCESS_E;
    }

    if(Parameter_Count >= 2)
    {
        if(Parameter_List[1].Integer_Is_Valid)
            long_test_time = Parameter_List[1].Integer_Value;
    }

    if(Parameter_Count >= 3)
    {  
        if(Parameter_List[2].Integer_Is_Valid) {
            test_type = Parameter_List[2].Integer_Value;
        }

        if(test_type == STORERECALL_TEST_IP_PACKET && Parameter_Count != 5)
        {
           QCLI_Printf(qcli_kpi_handle,"Port and IP address required for the IP type test \n");
           return QCLI_STATUS_SUCCESS_E;
        }
             
        kpi_ctx->params.tx_params.port = Parameter_List[3].Integer_Value;
        inet_pton(AF_INET,Parameter_List[4].String_Value, &kpi_ctx->params.tx_params.ip_address);
 
    }

    if(kpi_ctx == NULL){
        QCLI_Printf(qcli_kpi_handle,"Please run wlan_test_setup command \n");
        return QCLI_STATUS_SUCCESS_E;
    }

    /* Enable wlan */
    if (0 == qapi_WLAN_Enable(QAPI_WLAN_ENABLE_E))
    {
        QCLI_Printf(qcli_kpi_handle, "Wlan enabled \n");
        qapi_WLAN_Add_Device(deviceId);
        qapi_WLAN_Set_Callback(deviceId, (qapi_WLAN_Callback_t)kpi_wlan_callback_handler, NULL);

    }
    else
    {
        QCLI_Printf(qcli_kpi_handle, "Wlan Enabled failed \n");
        return QCLI_STATUS_ERROR_E;
    }

    qurt_signal_init(&kpi_ctx->wlan_kpi_event);

    if(kpi_ctx->security_mode == KPI_SEC_MODE_OPEN)
    {
        if(wlan_connect(kpi_ctx->ssid, NULL) != QCLI_STATUS_SUCCESS_E)
        {
            QCLI_Printf(qcli_kpi_handle, "wlan connect failed \n");
            goto storerecall_test_error;
        }
    }
    else if(kpi_ctx->security_mode == KPI_SEC_MODE_WPA)
    {
        if(wlan_connect(kpi_ctx->ssid, kpi_ctx->wpa2_psk) != QCLI_STATUS_SUCCESS_E)
        {
            QCLI_Printf(qcli_kpi_handle, "Wlan connect failed \n");
            goto storerecall_test_error;
        }
    }

    QCLI_Printf(qcli_kpi_handle,"getting IP address \n");

    /* get dhcp IP address */
    if (qapi_Net_Interface_Exist(interface_name, &if_is_up) == FALSE ||
        if_is_up == FALSE)
    {
        QCLI_Printf(qcli_kpi_handle, "ERROR: %s does not exist or is DOWN.\n", interface_name);
        goto storerecall_test_error;
    }

    if (qapi_Net_DHCPv4c_Register_Success_Callback(interface_name, kpi_ipconfig_dhcpc_success_cb) != 0 ||
            qapi_Net_IPv4_Config(interface_name, QAPI_NET_IPV4CFG_DHCP_IP_E, NULL, NULL, NULL) != 0)
    {
            QCLI_Printf(qcli_kpi_handle, "ERROR: DHCPv4 new failed\n");
            goto storerecall_test_error;
    }

    /* wait for dhcp callback */
    if(qurt_signal_wait_timed(&kpi_ctx->wlan_kpi_event, KPI_DHCP_EVENT_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK, &signal_set, QURT_TIME_WAIT_FOREVER) != QURT_EOK) {
        QCLI_Printf(qcli_kpi_handle, "Timed out waiting for DHCP success  \n");
        goto storerecall_test_error;
    }

    /* wait one second before starting the storerecall test */
    qapi_Task_Delay(ONE_SECOND_DELAY);

    kpi_gpio_setup();

    kpi_gpio_toggle(KPI_GPIO_LOW);

    /* initiate storerecall for 30 seconds */
    qapi_WLAN_Suspend_Start(30*WLAN_ONE_SEC_SUSPEND);

    /* wait for resume done callback */
    if(qurt_signal_wait_timed(&kpi_ctx->wlan_kpi_event, KPI_RESUME_DONE_EVENT_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK, &signal_set, QURT_TIME_WAIT_FOREVER) != QURT_EOK) {
        /* todo, check for correct signal */
        QCLI_Printf(qcli_kpi_handle, "Timed out waiting for Resume done callback \n");
        goto storerecall_test_error;
    }

    kpi_gpio_toggle(KPI_GPIO_HIGH);

    /* print the storerecall events */
    temp1 = g_wlan_strrcl_time_measure[WLAN_STORERECALL_KF_INIT_INDEX];
    temp2 = g_wlan_strrcl_time_measure[WLAN_STORERECALL_KF_POWER_ON_INDEX];

    if (temp2 < temp1)
    {
        /* Assume the systick wraps around once */
        duration = ~temp1 + 1 + temp2;
    }
    else
    {
        duration = temp2 - temp1;
    }

    time_elapsed = (duration * multiplier)/divider;

    QCLI_Printf(qcli_kpi_handle,"\nstorerecall Time: time to power up wlan chip %d ms \n", time_elapsed);

    temp1 = g_wlan_strrcl_time_measure[WLAN_STORERECALL_KF_POWER_ON_INDEX];
    temp2 = g_wlan_strrcl_time_measure[WLAN_STORERECALL_KF_FW_DOWNLOAD_INDEX];


    if (temp2 < temp1)
    {
        /* Assume the systick wraps around once */
        duration = ~temp1 + 1 + temp2;
    }
    else
    {
        duration = temp2 - temp1;
    }

    time_elapsed = (duration * multiplier)/divider;

    QCLI_Printf(qcli_kpi_handle,"\nstorerecall Time: time to download wlan firmware %d ms \n", time_elapsed);

    temp1 = g_wlan_strrcl_time_measure[WLAN_STORERECALL_KF_FW_DOWNLOAD_INDEX];
    temp2 = g_wlan_strrcl_time_measure[WLAN_STORERECALL_KF_DONE_TIME];


    if (temp2 < temp1)
    {
        /* Assume the systick wraps around once */
        duration = ~temp1 + 1 + temp2;
    }
    else
    {
        duration = temp2 - temp1;
    }

    time_elapsed = (duration * multiplier)/divider;

    QCLI_Printf(qcli_kpi_handle,"storerecall Time: time between firmware download & storerecall completion %d ms \n", time_elapsed);

    temp1 = g_wlan_strrcl_time_measure[WLAN_STORERECALL_KF_INIT_INDEX];
    temp2 = g_wlan_strrcl_time_measure[WLAN_STORERECALL_KF_DONE_TIME];

    if (temp2 < temp1)
    {
        /* Assume the systick wraps around once */
        duration = ~temp1 + 1 + temp2;
    }
    else
    {
        duration = temp2 - temp1;
    }

    time_elapsed = (duration * multiplier)/divider;

    QCLI_Printf(qcli_kpi_handle,"storerecall Time: Total Boot time %d ms \n", time_elapsed);

    /* wait one second before starting the storerecall test */
    qapi_Task_Delay(ONE_SECOND_DELAY);

    kpi_gpio_toggle(KPI_GPIO_LOW);

    /* initiate storerecall for 150 seconds or longer */
    qapi_WLAN_Suspend_Start(long_test_time*WLAN_ONE_SEC_SUSPEND);

    /* wait for resume done callback */
    if(qurt_signal_wait_timed(&kpi_ctx->wlan_kpi_event, KPI_RESUME_DONE_EVENT_MASK | KPI_STA_DISCONN_EVENT_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK, &signal_set, QURT_TIME_WAIT_FOREVER) != QURT_EOK) {
        QCLI_Printf(qcli_kpi_handle, "Timed out waiting for Resume done callback \n");
        goto storerecall_test_error;
    }
    /* disconnect event will be received before storerecall done event */
    else if(signal_set == KPI_STA_DISCONN_EVENT_MASK)
    {
       t1 = qurt_timer_get_ticks();

       if(qurt_signal_wait_timed(&kpi_ctx->wlan_kpi_event, KPI_RESUME_DONE_EVENT_MASK | KPI_STA_CONN_EVENT_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK, &signal_set, QURT_TIME_WAIT_FOREVER) != QURT_EOK) {
           QCLI_Printf(qcli_kpi_handle, "Timed out waiting for resume done event \n");
           goto storerecall_test_error;
       }

       /* wait for resume done event, if connect is received */
       if(signal_set == KPI_STA_CONN_EVENT_MASK)
       {
           t2 = qurt_timer_get_ticks();
           if(qurt_signal_wait_timed(&kpi_ctx->wlan_kpi_event, KPI_RESUME_DONE_EVENT_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK, &signal_set, QURT_TIME_WAIT_FOREVER) != QURT_EOK) {
              QCLI_Printf(qcli_kpi_handle, "Timed out waiting for resume done event \n");
              goto storerecall_test_error;
           }
       }
       else if(signal_set == KPI_RESUME_DONE_EVENT_MASK)
       {
           if(qurt_signal_wait_timed(&kpi_ctx->wlan_kpi_event, KPI_STA_CONN_EVENT_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK, &signal_set, 30000) != QURT_EOK) {
              QCLI_Printf(qcli_kpi_handle, "Timed out waiting for resume done event \n");
              goto storerecall_test_error;
           }
           else
              t2 = qurt_timer_get_ticks();
       }
    }

    kpi_gpio_toggle(KPI_GPIO_HIGH);

    if(test_type == STORERECALL_TEST_IP_PACKET) {

        /* connect to server time */
        kpi_gpio_toggle(KPI_GPIO_LOW);
        t3 = qurt_timer_get_ticks();

         if(kpi_demo_socket_connect(KPI_DEMO_UDP) == QCLI_STATUS_SUCCESS_E) {

            kpi_demo_send_packet_test(KPI_DEMO_PACKET_SIZE, 1);

            kpi_gpio_toggle(KPI_GPIO_HIGH);
        }
        else {
            QCLI_Printf(qcli_kpi_handle, "Socket connection failed \n");
            goto storerecall_test_error;
        }
        t4 = qurt_timer_get_ticks();
        kpi_gpio_toggle(KPI_GPIO_HIGH);

    }

    /* print the storerecall events */
    temp1 = g_wlan_strrcl_time_measure[WLAN_STORERECALL_KF_INIT_INDEX];
    temp2 = g_wlan_strrcl_time_measure[WLAN_STORERECALL_KF_POWER_ON_INDEX];

    if (temp2 < temp1)
    {
        /* Assume the systick wraps around once */
        duration = ~temp1 + 1 + temp2;
    }
    else
    {
        duration = temp2 - temp1;
    }

    time_elapsed = (duration * multiplier)/divider;

    QCLI_Printf(qcli_kpi_handle,"storerecall Time: time to power up wlan chip %d ms \n", time_elapsed);

    temp1 = g_wlan_strrcl_time_measure[WLAN_STORERECALL_KF_POWER_ON_INDEX];
    temp2 = g_wlan_strrcl_time_measure[WLAN_STORERECALL_KF_FW_DOWNLOAD_INDEX];

    if (temp2 < temp1)
    {
        /* Assume the systick wraps around once */
        duration = ~temp1 + 1 + temp2;
    }
    else
    {
        duration = temp2 - temp1;
    }

    time_elapsed = (duration * multiplier)/divider;

    QCLI_Printf(qcli_kpi_handle,"storerecall Time: time to download wlan firmware %d ms \n", time_elapsed);

    temp1 = g_wlan_strrcl_time_measure[WLAN_STORERECALL_KF_FW_DOWNLOAD_INDEX];
    temp2 = g_wlan_strrcl_time_measure[WLAN_STORERECALL_KF_DONE_TIME];

    if (temp2 < temp1)
    {
        /* Assume the systick wraps around once */
        duration = ~temp1 + 1 + temp2;
    }
    else
    {
        duration = temp2 - temp1;
    }

    time_elapsed = (duration * multiplier)/divider;

    QCLI_Printf(qcli_kpi_handle,"storerecall Time: time between firmware download & storerecall completion %d ms \n", time_elapsed);

    temp1 = g_wlan_strrcl_time_measure[WLAN_STORERECALL_KF_INIT_INDEX];
    temp2 = g_wlan_strrcl_time_measure[WLAN_STORERECALL_KF_DONE_TIME];

    if (temp2 < temp1)
    {
        /* Assume the systick wraps around once */
        duration = ~temp1 + 1 + temp2;
    }
    else
    {
        duration = temp2 - temp1;
    }

    time_elapsed = (duration * multiplier)/divider;

    QCLI_Printf(qcli_kpi_handle,"storerecall Time: Total Boot time %d ms \n", time_elapsed);

    /* print total breakdown of the time taken */
    if(test_type >= STORERECALL_TEST_CONNECT_TIME) {
         QCLI_Printf(qcli_kpi_handle,"Wlan connect time after storerecall operation  = %d ms \n", 
              ((t2 - t1)*qurt_timer_convert_ticks_to_time(1, QURT_TIME_MSEC)));
    }

    if(test_type == STORERECALL_TEST_IP_PACKET) {
         QCLI_Printf(qcli_kpi_handle,"Wlan send IP packet time after storerecall operation = %d ms \n",
             ((t4 - t3)*qurt_timer_convert_ticks_to_time(1, QURT_TIME_MSEC)));
    }


storerecall_test_error:

    qurt_signal_delete(&kpi_ctx->wlan_kpi_event);

    kpi_gpio_release();

    qapi_WLAN_Remove_Device(deviceId);

    if(0 == qapi_WLAN_Enable(QAPI_WLAN_DISABLE_E))
    {
        QCLI_Printf(qcli_kpi_handle,"Wlan successfully disabled \n");
    }

    return QCLI_STATUS_SUCCESS_E;
}

/* 
 * wlan_connection_time_packet_test - This function helps determine 
 * time break down for wlan enable, connect time to the AP,
 * DHCP acquisition time & TCP/UDP packet transmission time.
 *
 */
QCLI_Command_Status_t wlan_connection_time_packet_test(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{

    /* input params
     * ip_address, port, security_type, type
     */
    uint32_t packet_count = 1, ip_type = KPI_DEMO_UDP;
    qbool_t if_is_up = FALSE;
    uint32_t signal_set;
    uint32_t t1, t2, t3, t4;
    char *interface_name = "wlan1";

    if(kpi_ctx == NULL)
    {
        QCLI_Printf(qcli_kpi_handle, "Please run wlan_test_setup command before running this command \n");

    }

    if(Parameter_Count < 4) {
        QCLI_Printf(qcli_kpi_handle,"All parameters are required \n");
        QCLI_Printf(qcli_kpi_handle,"Usage: wlan_connection_time_packet_test ip_address port security_type ip_type packet_count \n");
        return QCLI_STATUS_SUCCESS_E;
    }

    /* parse command line params */
    inet_pton(AF_INET,Parameter_List[0].String_Value, &kpi_ctx->params.tx_params.ip_address);
    kpi_ctx->params.tx_params.port = Parameter_List[1].Integer_Value;
    kpi_ctx->security_mode = Parameter_List[2].Integer_Value;
    ip_type = Parameter_List[3].Integer_Value;

    if(Parameter_Count > 4) {
        packet_count = Parameter_List[4].Integer_Value;
    }

    kpi_gpio_setup();

    kpi_gpio_toggle(KPI_GPIO_HIGH);

    /* setup time */
    t1 = qurt_timer_get_ticks();

    if (QAPI_OK == qapi_WLAN_Enable(QAPI_WLAN_ENABLE_E))
    {
        QCLI_Printf(qcli_kpi_handle, "Wlan enabled \n");
        qapi_WLAN_Add_Device(deviceId);
        qapi_WLAN_Set_Callback(deviceId, (qapi_WLAN_Callback_t)kpi_wlan_callback_handler, NULL);
    }
    else
    {
        QCLI_Printf(qcli_kpi_handle, "Wlan Enabled failed \n");
        return QCLI_STATUS_ERROR_E;
    }

    qurt_signal_create(&kpi_ctx->wlan_kpi_event);

    if(kpi_ctx->security_mode == KPI_SEC_MODE_OPEN)
    {
        if(wlan_connect(kpi_ctx->ssid, NULL) != QCLI_STATUS_SUCCESS_E)
        {
            goto packet_test_error;
        }
    }
    else if(kpi_ctx->security_mode == KPI_SEC_MODE_WPA)
    {
        if(wlan_connect(kpi_ctx->ssid, kpi_ctx->wpa2_psk) != QCLI_STATUS_SUCCESS_E)
        {
            goto packet_test_error;
        }
    }

    QCLI_Printf(qcli_kpi_handle,"getting IP address \n");

    /* get dhcp IP address */
    if (qapi_Net_Interface_Exist(interface_name, &if_is_up) == FALSE ||
        if_is_up == FALSE)
    {
        QCLI_Printf(qcli_kpi_handle, "ERROR: %s does not exist or is DOWN.\n", interface_name);
        goto packet_test_error;
    }

    if (qapi_Net_DHCPv4c_Register_Success_Callback(interface_name, kpi_ipconfig_dhcpc_success_cb) != 0 ||
            qapi_Net_IPv4_Config(interface_name, QAPI_NET_IPV4CFG_DHCP_IP_E, NULL, NULL, NULL) != 0)
    {
            QCLI_Printf(qcli_kpi_handle, "ERROR: DHCPv4 new failed\n");
            goto packet_test_error;
    }

    /* wait for dhcp callback */
    if(qurt_signal_wait_timed(&kpi_ctx->wlan_kpi_event, KPI_DHCP_EVENT_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK, &signal_set, QURT_TIME_WAIT_FOREVER) != QURT_EOK) {
        goto packet_test_error;
    }

    kpi_gpio_toggle(KPI_GPIO_LOW);

    /* setup time */
    t2 = qurt_timer_get_ticks();

    if(kpi_demo_socket_connect(ip_type) == QCLI_STATUS_SUCCESS_E) {

        kpi_gpio_toggle(KPI_GPIO_HIGH);

        /* connect to server time */
        t3 = qurt_timer_get_ticks();

        kpi_demo_send_packet_test(KPI_DEMO_PACKET_SIZE, packet_count);

        kpi_gpio_toggle(KPI_GPIO_LOW);

        /* send one packet time */
        t4 = qurt_timer_get_ticks();

        /* print total breakdown of the time taken */
        QCLI_Printf(qcli_kpi_handle,"Setup time = %d ms \n", 
            ((t2 -  t1)*qurt_timer_convert_ticks_to_time(1, QURT_TIME_MSEC)));
        QCLI_Printf(qcli_kpi_handle,"Server connection time = %d ms \n", 
            ((t3 -  t2)*qurt_timer_convert_ticks_to_time(1, QURT_TIME_MSEC)));
        QCLI_Printf(qcli_kpi_handle,"Time to send out IP packet time = %d ms \n",
            ((t4 -  t3)*qurt_timer_convert_ticks_to_time(1, QURT_TIME_MSEC)));
        QCLI_Printf(qcli_kpi_handle,"Total time for the test = %d ms \n",
            ((t4 -  t1)*qurt_timer_convert_ticks_to_time(1, QURT_TIME_MSEC)));

    }

    /* Todo merge this with Quartz boot up time such that
     * This would be the first function called after
     * powering on the system
     */

packet_test_error:

    qurt_signal_delete(&kpi_ctx->wlan_kpi_event);

    kpi_gpio_release();

    qapi_WLAN_Remove_Device(deviceId);

    if(0 == qapi_WLAN_Enable(QAPI_WLAN_DISABLE_E))
    {
        QCLI_Printf(qcli_kpi_handle,"Wlan successfully disabled \n");
    }

    return QCLI_STATUS_SUCCESS_E;
}


/* 
 * wlan_connection_time_test - This function helps determine 
 * time break down for wlan enable, connect time to the AP,
 * DHCP acquisition time.
 *
 */
QCLI_Command_Status_t wlan_connection_time_test(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{

    /* input params
     * security_type
     */
    qbool_t if_is_up = FALSE;
    uint32_t signal_set;
    uint32_t t1, t2, t3, t4;
    char *interface_name = "wlan1";

    if(kpi_ctx == NULL)
    {
        QCLI_Printf(qcli_kpi_handle, "Please run wlan_test_setup command before running this command \n");
    }

    if(Parameter_Count < 1) {
        QCLI_Printf(qcli_kpi_handle,"All parameters are required \n");
        QCLI_Printf(qcli_kpi_handle,"Usage: wlan_connection_time_test security_type \n");
        return QCLI_STATUS_SUCCESS_E;
    }

    kpi_ctx->security_mode = Parameter_List[0].Integer_Value;

    kpi_gpio_setup();

    kpi_gpio_toggle(KPI_GPIO_HIGH);

    /* setup time */
    t1 = qurt_timer_get_ticks();

    if (0 == qapi_WLAN_Enable(QAPI_WLAN_ENABLE_E))
    {
        QCLI_Printf(qcli_kpi_handle, "Wlan enabled \n");
        qapi_WLAN_Add_Device(deviceId);
        qapi_WLAN_Set_Callback(deviceId, (qapi_WLAN_Callback_t)kpi_wlan_callback_handler, NULL);

    }
    else
    {
        QCLI_Printf(qcli_kpi_handle, "Wlan Enabled failed \n");
        return QCLI_STATUS_ERROR_E;
    }

    qurt_signal_init(&kpi_ctx->wlan_kpi_event);

    t4 = qurt_timer_get_ticks();

    if(kpi_ctx->security_mode == KPI_SEC_MODE_OPEN)
    {
        if(wlan_connect(kpi_ctx->ssid, NULL) != QCLI_STATUS_SUCCESS_E)
        {
            goto time_test_error;
        }
    }
    else if(kpi_ctx->security_mode == KPI_SEC_MODE_WPA)
    {
        if(wlan_connect(kpi_ctx->ssid, kpi_ctx->wpa2_psk) != QCLI_STATUS_SUCCESS_E)
        {
            goto time_test_error;
        }
    }

    QCLI_Printf(qcli_kpi_handle,"getting IP address \n");

    kpi_gpio_toggle(KPI_GPIO_LOW);

    /* DHCP setup time - start */
    t2 = qurt_timer_get_ticks();

    /* get dhcp IP address */
    if (qapi_Net_Interface_Exist(interface_name, &if_is_up) == FALSE ||
        if_is_up == FALSE)
    {
        QCLI_Printf(qcli_kpi_handle, "ERROR: %s does not exist or is DOWN.\n", interface_name);
        goto time_test_error;
    }

    if (qapi_Net_DHCPv4c_Register_Success_Callback(interface_name, kpi_ipconfig_dhcpc_success_cb) != 0 ||
            qapi_Net_IPv4_Config(interface_name, QAPI_NET_IPV4CFG_DHCP_IP_E, NULL, NULL, NULL) != 0)
    {
            QCLI_Printf(qcli_kpi_handle, "ERROR: DHCPv4 new failed\n");
            goto time_test_error;
    }

    /* wait for dhcp callback */
    if(qurt_signal_wait_timed(&kpi_ctx->wlan_kpi_event, KPI_DHCP_EVENT_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK, &signal_set, QURT_TIME_WAIT_FOREVER) != QURT_EOK) {
        goto time_test_error;
    }

    kpi_gpio_toggle(KPI_GPIO_HIGH);

    /* setup time - complete */
    t3 = qurt_timer_get_ticks();

    /* print total breakdown of the time taken */
    QCLI_Printf(qcli_kpi_handle,"Wlan enable to EAP completion time = %d ms \n", 
            ((t2 -  t1)*qurt_timer_convert_ticks_to_time(1, QURT_TIME_MSEC)));
    QCLI_Printf(qcli_kpi_handle,"DHCP IP acquire time = %d ms \n", 
            ((t3 -  t2)*qurt_timer_convert_ticks_to_time(1, QURT_TIME_MSEC)));
    /* todo, get the info from driver, reg address will be updated */
    QCLI_Printf(qcli_kpi_handle,"Assocation time = %d ms \n",
            ((kpi_ctx->assoc_time_stamp -  t4)*qurt_timer_convert_ticks_to_time(1, QURT_TIME_MSEC))    );
    QCLI_Printf(qcli_kpi_handle, "4 Way handshake time = %d ms \n",
            ((t2 - kpi_ctx->assoc_time_stamp)*qurt_timer_convert_ticks_to_time(1, QURT_TIME_MSEC)));

time_test_error:

    qurt_signal_delete(&kpi_ctx->wlan_kpi_event);

    kpi_gpio_release();

    qapi_WLAN_Remove_Device(deviceId);

    if(0 == qapi_WLAN_Enable(QAPI_WLAN_DISABLE_E))
    {
        QCLI_Printf(qcli_kpi_handle,"Wlan successfully disabled \n");
    }

    return QCLI_STATUS_SUCCESS_E;
}            

/* 
 * kpi_demo_socket_connect - This function take a type (UDP/TCP)
 * and makes a socket connection to the server. The server
 * for the current form of demo requires the use of 
 * ath_console.exe. But this function can be extended to
 * connect to a number of different servers.
 * port and ip address should be set by the caller.
 */
uint32_t kpi_demo_socket_connect(uint8_t type)
{
    struct sockaddr_in foreign_addr;
    struct sockaddr *to;
    uint32_t tolen;
    int family;
    int tos_opt;
    struct sockaddr_in src_sin;

    family = AF_INET;
    memset(&foreign_addr, 0, sizeof(foreign_addr));
    foreign_addr.sin_addr.s_addr    = kpi_ctx->params.tx_params.ip_address;
    foreign_addr.sin_port           = htons(kpi_ctx->params.tx_params.port);
    foreign_addr.sin_family         = family;

    src_sin.sin_family              = family;
    src_sin.sin_addr.s_addr         = kpi_ctx->params.tx_params.source_ipv4_addr;
    src_sin.sin_port                = htons(0);

    to = (struct sockaddr *)&foreign_addr;
    tolen = sizeof(foreign_addr);
    tos_opt = IP_TOS;


    if(type == KPI_DEMO_UDP)
    {
        /* Create UDP socket */
        if ((kpi_ctx->sock_peer = qapi_socket(family, SOCK_DGRAM, 0)) == -1)
        {
            QCLI_Printf(qcli_kpi_handle, "Socket creation failed\n");
            return -1;
        }

    }
    else if(type == KPI_DEMO_TCP)
    {

        /* Create TCP socket */
        if ((kpi_ctx->sock_peer = qapi_socket(family, SOCK_STREAM, 0)) == -1)
        {
            QCLI_Printf(qcli_kpi_handle, "Socket creation failed\n");
            return -1;
        }

    }

    if (kpi_ctx->params.tx_params.source_ipv4_addr != 0) {
        if (qapi_bind(kpi_ctx->sock_peer, (struct sockaddr*)&src_sin, sizeof(src_sin)) == -1) {
            QCLI_Printf(qcli_kpi_handle, "Socket bind failed\n");
        }
    }

    if (kpi_ctx->params.tx_params.ip_tos > 0)
    {
        qapi_setsockopt(kpi_ctx->sock_peer, IP_OPTIONS, tos_opt, &kpi_ctx->params.tx_params.ip_tos, sizeof(uint8_t));
    }

    /* Connect to the server.*/
    QCLI_Printf(qcli_kpi_handle, "Connecting\n");
    if (qapi_connect( kpi_ctx->sock_peer, to, tolen) == -1)
    {
        QCLI_Printf(qcli_kpi_handle, "Connection failed\n");
    }

    return QCLI_STATUS_SUCCESS_E;

}

/* 
 * kpi_demo_send_packet_test - This function create a data packet and
 * send it to the server to which the connection was established.
 * This function should be used in conjunction of kpi_demo_socket_connect()
 * The packet current create a pattern of Data which is used by the
 * ath_console.exe application.
 */
int32_t kpi_demo_send_packet_test(uint32_t packet_size, uint32_t packet_count)
{

        uint32_t netbuf_id;
        int send_flag = 0;
        uint32_t cur_packet_number = 1,n_send_ok;
        int32_t send_bytes = 0;
        uint32_t is_test_done = 0;
        KPI_EOT_PACKET *endmark;

        kpi_ctx->params.tx_params.packet_number = packet_count;
        kpi_ctx->params.tx_params.test_mode = PACKET_TEST;

        /* allocate the buffer, if needed */
        if ( kpi_ctx->buffer == NULL )
        {
             kpi_ctx->buffer = malloc(packet_size);

             if(kpi_ctx->buffer == NULL)
                return QCLI_STATUS_ERROR_E;
        }

        /* Update the buffer:
         *
         * [START]<4-byte Packet Index><4-byte Packet Size>000102..FF000102..FF0001..[END]
         * Byte counts: 8 + 4 + 4 + (packet_size-22) + 6
         */
         char *pkt_start = "[START]";
         char *pkt_end = "[END]";
         uint32_t val;

         netbuf_id = QAPI_NETBUF_APP;

         /* Add "[START]" */
         qapi_Net_Buf_Update(kpi_ctx->buffer, 0, pkt_start, 8, netbuf_id);

         /* Packet index */
         val = htonl(cur_packet_number);
         qapi_Net_Buf_Update(kpi_ctx->buffer, 8, &val, 4, netbuf_id);

         /* Packet size */
         val = htonl(packet_size);
         qapi_Net_Buf_Update(kpi_ctx->buffer, 12, &val, 4, netbuf_id);
                        

         /* Add pattern
          * The pattern is repeated 00 01 02 03 .. FE FF
          */
         uint8_t data[] = {0xaa, 0xbb, 0xcc, 0xdd};
         qapi_Net_Buf_Update(kpi_ctx->buffer, 16, data, packet_size - 16 - 6, netbuf_id);

         /* Add "[END]" */
         qapi_Net_Buf_Update(kpi_ctx->buffer, packet_size-6, pkt_end, 6, netbuf_id);
        
         do
         {
            
            {
                send_bytes = qapi_send(kpi_ctx->sock_peer, kpi_ctx->buffer, packet_size, send_flag);
            }

            if ( send_bytes != packet_size )
            {
                int errno = qapi_errno(kpi_ctx->sock_peer);
                if ( (EPIPE == errno) ||
                     (ENOTSOCK == errno) ||
                     (EBADF == errno) ||
                     (EIEIO == errno))
                {
                    QCLI_Printf(qcli_kpi_handle, "\nError: send_bytes=%d, errno=%d\n", send_bytes, errno);
                    kpi_ctx->buffer = NULL;
                    break;
                }
                else
                {
                    if ( ENOBUFS != errno )
                    {
                        QCLI_Printf(qcli_kpi_handle, "\nFailed to qapi_send, send_bytes=%d, errno=%d\n", send_bytes, errno);
                    }

                    qapi_Task_Delay(1000);
                }
            }
            else
            {
                cur_packet_number++;
            }

            if (send_bytes > 0)
            {
                kpi_ctx->pktStats.bytes += send_bytes;
                ++n_send_ok;
            }

            /*Test mode can be "number of packets" or "fixed time duration"*/
            if (kpi_ctx->params.tx_params.test_mode == PACKET_TEST)
            {
                if ((cur_packet_number >= kpi_ctx->params.tx_params.packet_number))
                {
                    is_test_done = 1;
                }
            }

        } while ( !((is_test_done) || (NULL == kpi_ctx->buffer)) );   /* send loop */

    endmark = malloc(sizeof(KPI_EOT_PACKET));

    /* send endmark packet to print details stats on ath_console side */
    ((KPI_EOT_PACKET*)endmark)->code    = END_OF_TEST_CODE;
    ((KPI_EOT_PACKET*)endmark)->packet_count = htonl(cur_packet_number);

    qapi_send(kpi_ctx->sock_peer, (char *)endmark, sizeof(KPI_EOT_PACKET), 0);
    qapi_socketclose(kpi_ctx->sock_peer);

    if(kpi_ctx->buffer != NULL) {
        free(kpi_ctx->buffer);
        kpi_ctx->buffer = NULL;
    }

    free(endmark);

    QCLI_Printf(qcli_kpi_handle, "Test Completed \n");

    return QCLI_STATUS_SUCCESS_E;


}

/* 
 *kpi_demo_fw_update - This function does firmware upgrade and measures
 *the total time required to update the firmware. Please refer to programmers
 *guide for detailed information on how to use this command.
 */
QCLI_Command_Status_t kpi_demo_fw_update(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
   /* replicates standard OTA update call*/
   uint32_t t1,t2;

   qapi_Fw_Upgrade_Status_Code_t resp_code;
   qapi_Fw_Upgrade_Plugin_t plugin = { plugin_Ftp_Init,
                                plugin_Ftp_Recv_Data,
                                plugin_Ftp_Abort,
                                plugin_Ftp_Resume,
                                plugin_Ftp_Fin};

    if( (Parameter_Count != 4) || (Parameter_List[3].Integer_Is_Valid == 0) )
    {

        QCLI_Printf(qcli_kpi_handle,"Usage: plugin_type interface server_address filename param(optional) \n");
        return QCLI_STATUS_USAGE_E;
    }

    kpi_gpio_setup();

    kpi_gpio_toggle(KPI_GPIO_HIGH);

    t1 = qurt_timer_get_ticks();

    resp_code = qapi_Fw_Upgrade(Parameter_List[0].String_Value, &plugin, Parameter_List[1].String_Value, Parameter_List[2].String_Value, Parameter_List[3].Integer_Value, kpi_fw_upgrade_callback, NULL );

    kpi_gpio_toggle(KPI_GPIO_LOW);

    t2 = qurt_timer_get_ticks();

    if(QAPI_FW_UPGRADE_OK_E != resp_code)
    {
        QCLI_Printf(qcli_kpi_handle, "Firmware Upgrade Image Download Failed ERR:%d\r\n",resp_code);
        if( resp_code == QAPI_FW_UPGRADE_ERR_TRIAL_IS_RUNNING_E )
        {
            QCLI_Printf(qcli_kpi_handle, "Trial Partition is running, need reboot to do Firmware Upgrade.\r\n");
        }
    } else {
        QCLI_Printf(qcli_kpi_handle, "Firmware Upgrade Image Download Completed successfully\r\n");
        QCLI_Printf(qcli_kpi_handle, "Total time to upgrade the firwamre = %d ms \r\n", ((t2 - t1)*qurt_timer_convert_ticks_to_time(1, QURT_TIME_MSEC)));
    }

    kpi_gpio_release();

   return QCLI_STATUS_SUCCESS_E;

}

/* 
 * wlan_device_discovery_simulate_new_device - This function simulates
 * a new quartz device i.e. like a physical device. This function provides
 * a design to help a smartphone discover a new IOT device and help connect
 * it to a an AP.
 */
QCLI_Command_Status_t wlan_device_discovery_simulate_new_device(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{

    uint32_t error = 0;
    uint32_t ap_device_id = 0;
    qapi_WLAN_Dev_Mode_e opMode;
    uint32_t passphraseLen = 0;
    char *my_ip = "192.168.100.1";
    char *dhcp_pool_start_ip = "192.168.100.10";
    char *dhcp_pool_end_ip = "192.168.100.20";
    char *ip_mask = "255.255.255.0";
    uint32_t startaddr, endaddr, myaddr, ip_addr_mask;
    uint32_t *gw_addr = NULL;
    int leasetime = 0xFFFFFFFF;   /* very very long */
    int family = AF_INET;
    struct sockaddr_in local_addr;
    struct sockaddr_in foreign_addr;
    int32_t fromlen;
    struct sockaddr *from = NULL;
    int on = 1;
    int32_t  conn_sock;
    int32_t  received, message_size;
    char *pb = NULL;
    onboarding_params *onb_p = NULL;
    uint32_t t1, t2, t3, t4;
    qapi_WLAN_Crypt_Type_e cipher;
    qapi_WLAN_Auth_Mode_e wpa_ver;
    uint32_t signal_set;
    uint32_t ssidLength = 0;

    /* Steps needed 
     * 1. Start AP
     * 2. Wait for a station to be connected
     * 3. Once station is connected send Ip packet with SSID & Password
     *
     */

    if(kpi_ctx == NULL){
        QCLI_Printf(qcli_kpi_handle,"Please run wlan_test_setup command \n");
        return QCLI_STATUS_SUCCESS_E;
    }

    kpi_gpio_setup();

    kpi_gpio_toggle(KPI_GPIO_HIGH);

    t1 = qurt_timer_get_ticks();

    qurt_signal_init(&kpi_ctx->wlan_kpi_event);

    if (0 == qapi_WLAN_Enable(QAPI_WLAN_ENABLE_E))
    {
        QCLI_Printf(qcli_kpi_handle, "Wlan enabled \n");
        qapi_WLAN_Add_Device(ap_device_id);
        qapi_WLAN_Set_Callback(ap_device_id, (qapi_WLAN_Callback_t)kpi_wlan_callback_handler, NULL);

    }
    else
    {
        QCLI_Printf(qcli_kpi_handle, "Wlan Enabled failed \n");
        return -1;
    }

    /* set device as AP */
    opMode = QAPI_WLAN_DEV_MODE_AP_E;
                    
    if (0 != qapi_WLAN_Set_Param(ap_device_id, 
                                    __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                    __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                                    &opMode,
                                    sizeof(qapi_WLAN_Dev_Mode_e),
                                    FALSE))
    {
        QCLI_Printf(qcli_kpi_handle, "Not able to set op mode to AP \r\n");
        return QCLI_STATUS_ERROR_E;
    }


    /* connect with ssid set to device discovery ssid in wpa2 mode */
    cipher = QAPI_WLAN_CRYPT_AES_CRYPT_E; 
    wpa_ver = QAPI_WLAN_AUTH_WPA2_PSK_E;

    ssidLength = strlen(ssid_onboarding);

    if(0 != qapi_WLAN_Set_Param(ap_device_id,
                                 __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                 __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
                                 (void *) ssid_onboarding,
                                 ssidLength,
                                 FALSE)) {
        QCLI_Printf(qcli_kpi_handle,"Error during setting of ssid %s \r\n", ssid_onboarding);
        return QCLI_STATUS_ERROR_E;
    }


    QCLI_Printf(qcli_kpi_handle,"Setting security mode to wpa \n");

    if(0 != qapi_WLAN_Set_Param(ap_device_id,
                                       __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                       __QAPI_WLAN_PARAM_GROUP_SECURITY_ENCRYPTION_TYPE,
                                       (void *) &cipher, //cipher is set in set_wpa
                                       sizeof(qapi_WLAN_Crypt_Type_e), 
                                       FALSE))
    {
            QCLI_Printf(qcli_kpi_handle,"Setting cipher failed \n");
            return QCLI_STATUS_ERROR_E;
    }

    if( 0 != qapi_WLAN_Set_Param (ap_device_id,
                                        __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                        __QAPI_WLAN_PARAM_GROUP_SECURITY_AUTH_MODE,
                                        (void *) &wpa_ver,
                                        sizeof(qapi_WLAN_Auth_Mode_e),
                                        FALSE))
    {
            QCLI_Printf(qcli_kpi_handle,"Setting wpa version failed \n");
            return QCLI_STATUS_ERROR_E;
    }

    passphraseLen  = strlen(password_onboarding);
    if((passphraseLen >= 8) && (passphraseLen <= 63))
    {
         if (0 != qapi_WLAN_Set_Param(ap_device_id,
                                            __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                            __QAPI_WLAN_PARAM_GROUP_SECURITY_PASSPHRASE,
                                            (void *)password_onboarding,
                                             passphraseLen,
                                             FALSE))
         {
                QCLI_Printf(qcli_kpi_handle, "Unable to set passphrase\r\n");
                return QCLI_STATUS_ERROR_E;
         }
    }
    else if(passphraseLen == MAX_PASSPHRASE_LENGTH)
    {
         if (0 != qapi_WLAN_Set_Param(ap_device_id,
                                            __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                            __QAPI_WLAN_PARAM_GROUP_SECURITY_PMK,
                                            (void *)password_onboarding,
                                            passphraseLen,
                                            FALSE))
         {
                QCLI_Printf(qcli_kpi_handle, "Unable to set pmk\r\n");
                return QCLI_STATUS_ERROR_E;
         }
    }
    else
    {
         QCLI_Printf(qcli_kpi_handle, "invalid password\r\n");
         return QCLI_STATUS_ERROR_E;
    }

    error = qapi_WLAN_Commit(deviceId);
    if(error != 0)
    { 
        QCLI_Printf(qcli_kpi_handle, "Commit failed \r\n");
        return QCLI_STATUS_ERROR_E;
    }

    error = inet_pton(AF_INET, dhcp_pool_start_ip, &startaddr);
    if (error != 0)
    {
            QCLI_Printf(qcli_kpi_handle, "Invalid start address\n");
            return QCLI_STATUS_ERROR_E;
    }

    error = inet_pton(AF_INET, dhcp_pool_end_ip, &endaddr);
    if (error != 0)
    {
            QCLI_Printf(qcli_kpi_handle, "Invalid end address\n");
            return QCLI_STATUS_ERROR_E;
    }

    error = inet_pton(AF_INET, my_ip, &myaddr);
    if (error != 0)
    {
            QCLI_Printf(qcli_kpi_handle, "Invalid ip address\n");
            return QCLI_STATUS_ERROR_E;
    }


    error = inet_pton(AF_INET, ip_mask, &ip_addr_mask);
    if (error != 0)
    {
            QCLI_Printf(qcli_kpi_handle, "Invalid ip mask\n");
            return QCLI_STATUS_ERROR_E;
    }


    /* set static IP address for wlan0 */
    error = qapi_Net_IPv4_Config("wlan0", QAPI_NET_IPV4CFG_STATIC_IP_E, &myaddr, &ip_addr_mask, gw_addr);
    if (error != 0)
    {
            QCLI_Printf(qcli_kpi_handle, "failed to set ip address\n");
            return QCLI_STATUS_ERROR_E;
    }


    /* start DHCP server and assign static IP address */
    if (qapi_Net_DHCPv4s_Register_Success_Callback("wlan0", kpi_ipconfig_dhcps_success_cb) != 0 ||
            qapi_Net_DHCPv4s_Set_Pool("wlan0", htonl(startaddr), htonl(endaddr), leasetime) != 0)
    {
            QCLI_Printf(qcli_kpi_handle, "Config DHCP pool failed\n");
            return QCLI_STATUS_ERROR_E;
    }

    kpi_gpio_toggle(KPI_GPIO_LOW);

    /* setup time */
    t2 = qurt_timer_get_ticks();

    do {

       /* wait for connection */
       if(qurt_signal_wait_timed(&kpi_ctx->wlan_kpi_event, KPI_STA_CONN_EVENT_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK, &signal_set, QURT_TIME_WAIT_FOREVER) != QURT_EOK) {
            QCLI_Printf(qcli_kpi_handle, "Timed out waiting for the signal \r\n");
            return QCLI_STATUS_ERROR_E;
       }

       kpi_gpio_toggle(KPI_GPIO_HIGH);

        /* device connection time */
       t3 = qurt_timer_get_ticks();

       local_addr.sin_family              = family;
       local_addr.sin_addr.s_addr         = myaddr;
       local_addr.sin_port                = htons(ONBOARDING_PORT);

       memset(&foreign_addr, 0, sizeof(foreign_addr));
       from = (struct sockaddr *)&foreign_addr;
       fromlen = sizeof(struct sockaddr_in);

       /* create a receive buffer, CFG_PACKET_SIZE_MAX_RX bytes maximum */
       kpi_ctx->buffer = malloc(KPI_ONB_MAX_RX_SIZE);

       /* Create RAW socket */
       if ((kpi_ctx->sock_peer = qapi_socket(family, SOCK_RAW, 0)) == -1)
       {
            QCLI_Printf(qcli_kpi_handle, "Socket creation failed\n");
            return QCLI_STATUS_ERROR_E;
       }

       if (myaddr != 0) {
           if (qapi_bind(kpi_ctx->sock_peer, (struct sockaddr*)&local_addr, sizeof(local_addr)) == -1) {
               QCLI_Printf(qcli_kpi_handle, "Socket bind failed\n");
               /* close the socket */
               qapi_socketclose(kpi_ctx->sock_peer);
               return QCLI_STATUS_ERROR_E;
           }
       }

       if (qapi_setsockopt(kpi_ctx->sock_peer, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) != QAPI_OK)
       {
           QCLI_Printf(qcli_kpi_handle, "ERROR: set IP_HDRINCL failure\n");
           qapi_socketclose(kpi_ctx->sock_peer);
           return QCLI_STATUS_ERROR_E;
       }

       fd_set rset;

       while (!kpi_quit)   /* Receive loop */
       {
           do
           {
               if (kpi_quit)
               {
                   goto test_error;
               }

                /* block for 500msec or until a packet is received */
                FD_ZERO(&rset);
                FD_SET(kpi_ctx->sock_peer, &rset);

                conn_sock = qapi_select(&rset, NULL, NULL, 500);
                if (conn_sock == -1)
                    goto test_error;// socket no longer valid

           } while (conn_sock == 0);

           /* Receive data */
           received = message_size = qapi_recvfrom( kpi_ctx->sock_peer, (char*)kpi_ctx->buffer, KPI_ONB_MAX_RX_SIZE, 0, from, &fromlen);

           if (received >= 0)
           {
                   /* remove 20 bytes IP HDR */
                   message_size -= 20;
                   pb = kpi_ctx->buffer + 20;

                   onb_p = malloc(sizeof(struct onb_params));

                   /* copy the rest of the buffer */
                   memcpy(onb_p, pb, sizeof(struct onb_params));

                   /* terminate this process */
                   kpi_quit = TRUE;

                   /* future enchancement, can inform the simulated smartphone that data received */
           }
           else {

                  /* check for error, TODO */

           }
               
        } /* receive_loop */
    } while(kpi_quit != TRUE);

    /* now initiate station mode and connect to the AP */
    if(QCLI_STATUS_SUCCESS_E == wlan_connect(onb_p->ssid, onb_p->passphrase))
    {

        kpi_gpio_toggle(KPI_GPIO_HIGH);

        /* measure time ticks, total time */
        t4 = qurt_timer_get_ticks();

        /* print total breakdown of the time taken */
        QCLI_Printf(qcli_kpi_handle,"Setup time = %d ms \n", 
            ((t2 -  t1)*qurt_timer_convert_ticks_to_time(1, QURT_TIME_MSEC)));
        QCLI_Printf(qcli_kpi_handle,"Device discovery and connection time = %d ms \n", 
            ((t3 -  t2)*qurt_timer_convert_ticks_to_time(1, QURT_TIME_MSEC)));
        QCLI_Printf(qcli_kpi_handle,"Connection to AP time = %d ms \n",
            ((t4 -  t3)*qurt_timer_convert_ticks_to_time(1, QURT_TIME_MSEC)));
        QCLI_Printf(qcli_kpi_handle,"Total onboarding time = %d ms \n",
            ((t4 -  t1)*qurt_timer_convert_ticks_to_time(1, QURT_TIME_MSEC)));
    }
    else
    {
        QCLI_Printf(qcli_kpi_handle,"Connection to SSID %s failed \n", onb_p->ssid);
    }

test_error:

    if(kpi_ctx->buffer != NULL) {
        free(kpi_ctx->buffer);
    }

    if(onb_p != NULL)
    {
        free(onb_p);
    }

    /* close the socket */
    qapi_socketclose(kpi_ctx->sock_peer);

    kpi_gpio_release();

    qapi_WLAN_Remove_Device(ap_device_id);

    if(0 == qapi_WLAN_Enable(QAPI_WLAN_DISABLE_E))
    {
        QCLI_Printf(qcli_kpi_handle,"Wlan successfully disabled \n");
    }

    return QCLI_STATUS_SUCCESS_E;
}

/*
 * wlan_connect - This is a helper function to support this demo.
 * It takes as an input SSID and passphrase and connects to AP
 * in either open security or wpa2 mode.
 */
int32_t wlan_connect(char *conn_ssid, char *passphrase)
{    

    int32_t error = 0;
    int ssidLength = 0;
    uint32_t temp_mode = 0, dataLen = 0; 
    char *bssid = NULL;
    qapi_WLAN_Crypt_Type_e cipher;
    qapi_WLAN_Auth_Mode_e wpa_ver;
    uint32_t signal_set;

    if(passphrase == NULL)
        kpi_ctx->security_mode = KPI_SEC_MODE_OPEN;
    else
        kpi_ctx->security_mode = KPI_SEC_MODE_WPA;

    ssidLength = strlen(conn_ssid);
    
    qapi_WLAN_Get_Param (deviceId, 
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                         __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE,
                         &temp_mode,
                         &dataLen);

    if (ssidLength > __QAPI_WLAN_MAX_SSID_LENGTH || ssidLength < 0) 
    {
       QCLI_Printf(qcli_kpi_handle, "SSID length exceeds Maximum value\r\n");
       return QCLI_STATUS_ERROR_E;
    }
    error = qapi_WLAN_Set_Param (deviceId,
                                 __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                                 __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID,
                                 (void *) conn_ssid,
                                 ssidLength,
                                 FALSE);
    if (error != 0)
    {
        QCLI_Printf(qcli_kpi_handle,"Error during setting of ssid %s error=%08x!\r\n", conn_ssid, error);
        return error;
    }

    if (temp_mode == QAPI_WLAN_DEV_MODE_STATION_E)
    {
        uint8_t bssidToConnect[__QAPI_WLAN_MAC_LEN] = {0};
        if (bssid && (ether_aton(bssid, bssidToConnect) != 0))
        {
            QCLI_Printf(qcli_kpi_handle, "Invalid BSSID to connect\r\n");
            return QCLI_STATUS_ERROR_E;
        }
        error = qapi_WLAN_Set_Param (deviceId,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                             __QAPI_WLAN_PARAM_GROUP_WIRELESS_BSSID,
                             bssidToConnect,
                             __QAPI_WLAN_MAC_LEN,
                             FALSE);
        if (error != 0)
        {
            QCLI_Printf(qcli_kpi_handle,"Error during setting of bssid  error=%08x!\r\n", error);
            return error;
        }

        if(bssid)
           QCLI_Printf(qcli_kpi_handle,"\r\nSetting BSSID to %s \r\n",bssid);

    }
   
    if(KPI_SEC_MODE_WPA == kpi_ctx->security_mode)
    {
        uint32_t passphraseLen = 0;
        cipher = QAPI_WLAN_CRYPT_AES_CRYPT_E; 
        wpa_ver = QAPI_WLAN_AUTH_WPA2_PSK_E;

        QCLI_Printf(qcli_kpi_handle,"Setting security mode to wpa \n");

        if(0 != qapi_WLAN_Set_Param(deviceId,
                                       __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                       __QAPI_WLAN_PARAM_GROUP_SECURITY_ENCRYPTION_TYPE,
                                       (void *) &cipher, //cipher is set in set_wpa
                                       sizeof(qapi_WLAN_Crypt_Type_e), 
                                       FALSE))
        {
            QCLI_Printf(qcli_kpi_handle,"Setting cipher failed \n");
            return -1;
        }

        if( 0 != qapi_WLAN_Set_Param (deviceId,
                                        __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                        __QAPI_WLAN_PARAM_GROUP_SECURITY_AUTH_MODE,
                                        (void *) &wpa_ver,
                                        sizeof(qapi_WLAN_Auth_Mode_e),
                                        FALSE))
        {
            QCLI_Printf(qcli_kpi_handle,"Setting wpa version failed \n");
            return QCLI_STATUS_ERROR_E;
        }
        passphraseLen  = strlen(passphrase);
        if((passphraseLen >= 8) && (passphraseLen <= 63))
        {
            if (0 != qapi_WLAN_Set_Param(deviceId,
                                            __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                            __QAPI_WLAN_PARAM_GROUP_SECURITY_PASSPHRASE,
                                            (void *)passphrase,
                                             passphraseLen,
                                             FALSE))
            {
                QCLI_Printf(qcli_kpi_handle, "Unable to set passphrase\r\n");
                return QCLI_STATUS_ERROR_E;
            }
        }
        else if(passphraseLen == MAX_PASSPHRASE_LENGTH)
        {
            if (0 != qapi_WLAN_Set_Param(deviceId,
                                            __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY,
                                            __QAPI_WLAN_PARAM_GROUP_SECURITY_PMK,
                                            (void *) passphrase,
                                            passphraseLen,
                                            FALSE))
            {
                QCLI_Printf(qcli_kpi_handle, "Unable to set pmk\r\n");
                return QCLI_STATUS_ERROR_E;
            }
       }
       else
       {
           QCLI_Printf(qcli_kpi_handle, "invalid password\r\n");
           return QCLI_STATUS_ERROR_E;
       }
        kpi_ctx->security_mode = KPI_SEC_MODE_OPEN;
   }

    error = qapi_WLAN_Commit(deviceId);
    if(error != 0)
    { 
        QCLI_Printf(qcli_kpi_handle, "Commit failed \r\n");
        return QCLI_STATUS_ERROR_E;
    }

    if(qurt_signal_wait_timed(&kpi_ctx->wlan_kpi_event, KPI_CONNECT_ASSOC_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK, &signal_set, QURT_TIME_WAIT_FOREVER) != QURT_EOK) {
        QCLI_Printf(qcli_kpi_handle, "Timed out waiting for the signal \r\n");
        return QCLI_STATUS_ERROR_E;
    }

    kpi_ctx->assoc_time_stamp = qurt_timer_get_ticks();

    if(kpi_ctx->wpa2_psk != NULL) {

        if(qurt_signal_wait_timed(&kpi_ctx->wlan_kpi_event, KPI_CONNECT_EVENT_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK, &signal_set, QURT_TIME_WAIT_FOREVER) != QURT_EOK) {
            QCLI_Printf(qcli_kpi_handle, "Timed out waiting for the signal \r\n");
            return QCLI_STATUS_ERROR_E;
        }

    }

    return QCLI_STATUS_SUCCESS_E;
}

/*
 * wlan_device_discovery_simulate_smartphone - This function provides a 
 * framework for simulating a smartphone. This function will connect to a
 * predetermined SSID, obtain IP address and provide the new simulated
 * device with the AP, SSID and password. When using this function and the
 * simulated new device framework, two different quartz boards should be 
 * used. Only WPA2 CCMP is currently supported for this demo. But it can be
 * easily extended to other security types if need be.
 * Additionally, this function can also be used to benchmark a smartphone and
 * for comparing the differences in time for onboarding.
 */
QCLI_Command_Status_t wlan_device_discovery_simulate_smartphone(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *interface_name = "wlan1";
    qbool_t if_is_up = FALSE;
    uint32_t signal_set;
    qapi_WLAN_Store_Scan_Results_e store_Scan_Results = QAPI_WLAN_BUFFER_SCAN_RESULTS_NON_BLOCKING_E;
    int family = AF_INET;
    struct sockaddr_in local_addr;
    struct sockaddr_in foreign_addr;
    uint32_t tolen;
    struct sockaddr *to;
    int on = 1;
    onboarding_params *onb_p;
    int32_t send_bytes;

    if(Parameter_Count < 1) {
        QCLI_Printf(qcli_kpi_handle,"All parameters are required \n");
        QCLI_Printf(qcli_kpi_handle,"Usage: wlan_device_discovery_simulate_smartphone ssid passcode \n");
        QCLI_Printf(qcli_kpi_handle,"Note: Only WPA2 CCMP is supported in this demo \n");
        QCLI_Printf(qcli_kpi_handle,"\n");
        return QCLI_STATUS_SUCCESS_E;
    }

    onb_p = malloc(sizeof(struct onb_params));

    memcpy(onb_p->ssid, Parameter_List[0].String_Value, strlen(Parameter_List[0].String_Value));
    onb_p->ssid_length = strlen(Parameter_List[0].String_Value);

    if((strlen(Parameter_List[1].String_Value)) < 8 || (strlen(Parameter_List[1].String_Value) > 64)) {
        QCLI_Printf(qcli_kpi_handle, "Invalid passcode length provided\n");
        return QCLI_STATUS_ERROR_E;
    }

    memcpy(onb_p->passphrase, Parameter_List[1].String_Value, strlen(Parameter_List[1].String_Value));
    onb_p->passphrase_length = strlen(Parameter_List[1].String_Value);

    if(kpi_ctx == NULL){
        QCLI_Printf(qcli_kpi_handle,"Please run wlan_test_setup command \n");
        return QCLI_STATUS_SUCCESS_E;
    }

    qurt_signal_init(&kpi_ctx->wlan_kpi_event);

    if (0 == qapi_WLAN_Enable(QAPI_WLAN_ENABLE_E))
    {
        QCLI_Printf(qcli_kpi_handle, "Wlan enabled \n");
        qapi_WLAN_Add_Device(deviceId);
        qapi_WLAN_Set_Callback(deviceId, (qapi_WLAN_Callback_t)kpi_wlan_callback_handler, NULL);

    }
    else
    {
        QCLI_Printf(qcli_kpi_handle, "Wlan Enabled failed \n");
        return -1;
    }

    /* issue a scan command and wait for the specific AP */
    qapi_WLAN_Start_Scan (deviceId,
                          NULL, store_Scan_Results);


    /* wait for the AP to be found */
    if(qurt_signal_wait_timed(&kpi_ctx->wlan_kpi_event, KPI_ONBOARDING_SSID_FOUND_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK, &signal_set, QURT_TIME_WAIT_FOREVER) != QURT_EOK) {
        QCLI_Printf(qcli_kpi_handle, "Timed out waiting for the signal \r\n");
        return QCLI_STATUS_ERROR_E;
    }

    if(kpi_quit == TRUE) {

        /* delete the signal */

        return QCLI_STATUS_SUCCESS_E;
    }


    /* else AP is found, use the default ap for now */
    kpi_ctx->ssid = ssid_onboarding;
    kpi_ctx->wpa2_psk = password_onboarding;

    if(kpi_ctx->security_mode == KPI_SEC_MODE_OPEN)
    {
        wlan_connect(kpi_ctx->ssid, NULL);
    }
    else if(kpi_ctx->security_mode == KPI_SEC_MODE_WPA)
    {
        wlan_connect(kpi_ctx->ssid, kpi_ctx->wpa2_psk);
    }

    QCLI_Printf(qcli_kpi_handle,"getting IP address \n");

    /* get dhcp IP address */
    if (qapi_Net_Interface_Exist(interface_name, &if_is_up) == FALSE ||
        if_is_up == FALSE)
    {
        QCLI_Printf(qcli_kpi_handle, "ERROR: %s does not exist or is DOWN.\n", interface_name);
        return QCLI_STATUS_ERROR_E;
    }

    if (qapi_Net_DHCPv4c_Register_Success_Callback(interface_name, kpi_ipconfig_dhcpc_success_cb) != 0 ||
            qapi_Net_IPv4_Config(interface_name, QAPI_NET_IPV4CFG_DHCP_IP_E, NULL, NULL, NULL) != 0)
    {
            QCLI_Printf(qcli_kpi_handle, "ERROR: DHCPv4 new failed\n");
            return QCLI_STATUS_ERROR_E;
    }

    /* wait for dhcp callback */
    if(qurt_signal_wait_timed(&kpi_ctx->wlan_kpi_event, KPI_DHCP_EVENT_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK, &signal_set, QURT_TIME_WAIT_FOREVER) != QURT_EOK) {
        return -1;
    }

    kpi_ctx->params.tx_params.ip_tos = 0;

    /* once DHCP address is received, send RAW packet with SSID and passphrase info */
    local_addr.sin_family              = family;
    local_addr.sin_addr.s_addr         = kpi_ctx->params.tx_params.source_ipv4_addr;
    local_addr.sin_port                = htons(ONBOARDING_PORT);

    tolen = sizeof(struct sockaddr_in);
    memset(&foreign_addr, 0, sizeof(foreign_addr));
    foreign_addr.sin_addr.s_addr    = kpi_ctx->dest_ip;
    foreign_addr.sin_family         = family;
    to = (struct sockaddr *)&foreign_addr;

    /* create a send buffer, CFG_PACKET_SIZE_MAX_RX bytes maximum */
    kpi_ctx->buffer = malloc(KPI_ONB_MAX_RX_SIZE);

    /* Create RAW socket */
    if ((kpi_ctx->sock_peer = qapi_socket(family, SOCK_RAW, 0)) == -1)
    {
        QCLI_Printf(qcli_kpi_handle, "Socket creation failed\n");
        return QCLI_STATUS_ERROR_E;
    }

    if (kpi_ctx->params.tx_params.source_ipv4_addr != 0) {
        if (qapi_bind(kpi_ctx->sock_peer, (struct sockaddr*)&local_addr, sizeof(local_addr)) == -1) {
            QCLI_Printf(qcli_kpi_handle, "Socket bind failed\n");
            /* close the socket */
            qapi_socketclose(kpi_ctx->sock_peer);
            return QCLI_STATUS_ERROR_E;
        }
    }

    if (qapi_setsockopt(kpi_ctx->sock_peer, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) != QAPI_OK)
    {
        QCLI_Printf(qcli_kpi_handle, "ERROR: set IP_HDRINCL failure\n");
        qapi_socketclose(kpi_ctx->sock_peer);
        return QCLI_STATUS_ERROR_E;
    }

    if (qapi_connect(kpi_ctx->sock_peer, to, tolen) == -1)
    {
        QCLI_Printf(qcli_kpi_handle, "ERROR: Conection failed\n");
        goto test_error;
    }


    ipv4_header_t *iphdr = (ipv4_header_t *)kpi_ctx->buffer;

    iphdr->ver_ihl = 0x45; /* ver: IPv4, IHL=20 bytes */
    iphdr->tos = kpi_ctx->params.tx_params.ip_tos;
    iphdr->len = htons(KPI_ONB_MAX_RX_SIZE);
    iphdr->id = 0;
    iphdr->flags_offset = 0;
    iphdr->ttl = 255;
    iphdr->protocol = (uint8_t)IP_RAW_HDR;
    iphdr->hdr_chksum = 0;
    iphdr->sourceip = kpi_ctx->params.tx_params.source_ipv4_addr; /* already in net order */
    iphdr->destip   = foreign_addr.sin_addr.s_addr;  /* already in net order */

    memcpy(kpi_ctx->buffer, onb_p, sizeof(struct onb_params));

    do {

        send_bytes = qapi_send(kpi_ctx->sock_peer, kpi_ctx->buffer, KPI_ONB_MAX_RX_SIZE, 0);

        if ( send_bytes != KPI_ONB_MAX_RX_SIZE )
        {
            int errno = qapi_errno(kpi_ctx->sock_peer);

            if ((EPIPE == errno) ||
               (ENOTSOCK == errno) ||
               (EBADF == errno) ||
               (EIEIO == errno))
            {
                QCLI_Printf(qcli_kpi_handle, "Exxxx: send_bytes=%d, errno=%d\n", send_bytes, errno);
                break;
            } 
        }

        /* send the info at 1 sec delay */
        qapi_Task_Delay(1000000);

    } while(kpi_quit != TRUE);

test_error:

    if(kpi_ctx->buffer != NULL) {
        free(kpi_ctx->buffer);
    }

    if(onb_p != NULL)
    {
        free(onb_p);
    }

    /* close the socket */
    qapi_socketclose(kpi_ctx->sock_peer);

    qapi_WLAN_Remove_Device(deviceId);

    if(0 == qapi_WLAN_Enable(QAPI_WLAN_DISABLE_E))
    {
        QCLI_Printf(qcli_kpi_handle,"Wlan successfully disabled \n");
    }

    return QCLI_STATUS_SUCCESS_E;

}

/*
 * wlan_cummulative_throughput_test - This test allows measurement of power
 * for a provided input rate. The function takes rate and latency as parameters
 * which can be used to shape the traffic transmitted from the DUT. Different
 * combinations of latency and throughput rate will yield different power numbers.
 * The goal of the function is to provide a desired rate.
 * Additionally, this test only runs for 1 minute.
 */
QCLI_Command_Status_t wlan_cummulative_throughput_test(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *interface_name = "wlan1";
    qbool_t if_is_up = FALSE;
    int rate = 1;
    uint32_t signal_set;

    /* take port, ip address and rate as input */
    if(Parameter_Count < 4) {
        QCLI_Printf(qcli_kpi_handle,"All parameters are required \n");
        QCLI_Printf(qcli_kpi_handle,"Usage: wlan_cummulative_throughput_test ip_address port rate security_type \n");
        return QCLI_STATUS_SUCCESS_E;
    }

    /* parse command line params */
    inet_pton(AF_INET,Parameter_List[0].String_Value, &kpi_ctx->params.tx_params.ip_address);
    kpi_ctx->params.tx_params.port = Parameter_List[1].Integer_Value;
    rate = Parameter_List[2].Integer_Value;
    kpi_ctx->security_mode = Parameter_List[3].Integer_Value;

    if(kpi_ctx == NULL){
        QCLI_Printf(qcli_kpi_handle,"Please run wlan_test_setup command \n");
        return QCLI_STATUS_SUCCESS_E;
    }

    qurt_signal_create(&kpi_ctx->wlan_kpi_event);

    if (0 == qapi_WLAN_Enable(QAPI_WLAN_ENABLE_E))
    {
        QCLI_Printf(qcli_kpi_handle, "Wlan enabled \n");
        qapi_WLAN_Add_Device(deviceId);
        qapi_WLAN_Set_Callback(deviceId, (qapi_WLAN_Callback_t)kpi_wlan_callback_handler, NULL);

    }
    else
    {
        QCLI_Printf(qcli_kpi_handle, "Wlan Enabled failed \n");
        return -1;
    }

    if(kpi_ctx->security_mode == KPI_SEC_MODE_OPEN)
    {
        if(wlan_connect(kpi_ctx->ssid, NULL) != QCLI_STATUS_SUCCESS_E)
        {
            goto wlan_tput_error;
        }
    }
    else if(kpi_ctx->security_mode == KPI_SEC_MODE_WPA)
    {
        if(wlan_connect(kpi_ctx->ssid, kpi_ctx->wpa2_psk) != QCLI_STATUS_SUCCESS_E)
        {
            goto wlan_tput_error;
        }
    }

    /* get dhcp IP address */
    if (qapi_Net_Interface_Exist(interface_name, &if_is_up) == FALSE ||
        if_is_up == FALSE)
    {
        QCLI_Printf(qcli_kpi_handle, "ERROR: %s does not exist or is DOWN.\n", interface_name);
        return QCLI_STATUS_ERROR_E;
    }

    if (qapi_Net_DHCPv4c_Register_Success_Callback(interface_name, kpi_ipconfig_dhcpc_success_cb) != 0 ||
            qapi_Net_IPv4_Config(interface_name, QAPI_NET_IPV4CFG_DHCP_IP_E, NULL, NULL, NULL) != 0)
    {
        QCLI_Printf(qcli_kpi_handle, "ERROR: DHCPv4 new failed\n");
        return QCLI_STATUS_ERROR_E;
    }

    /* wait for dhcp callback */
    if(qurt_signal_wait_timed(&kpi_ctx->wlan_kpi_event, KPI_DHCP_EVENT_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK, &signal_set, QURT_TIME_WAIT_FOREVER) != QURT_EOK) {
        return -1;
    }
    /* wait one second before starting the throughput test */
    qapi_Task_Delay(ONE_SECOND_DELAY);

    /* initiate throughput test */
    kpi_ctx->params.tx_params.ip_tos = 0;
    kpi_ctx->params.tx_params.test_mode = PACKET_TEST;
    kpi_ctx->params.tx_params.interval_us = 0; /* interval is 0 */
    kpi_ctx->params.tx_params.packet_number = (rate*ONE_MEGABIT_RATE_PACKET_COUNT);
    /* run the test for 1 minute */
    kpi_ctx->total_packet_count = (rate*ONE_MEGABIT_RATE_PACKET_COUNT*60);

    kpi_gpio_setup();

    kpi_gpio_toggle(KPI_GPIO_HIGH);

    kpi_udp_cumulative_tx();

    kpi_gpio_toggle(KPI_GPIO_LOW);

wlan_tput_error:

    kpi_gpio_release();

    qurt_signal_delete(&kpi_ctx->wlan_kpi_event);

    qapi_WLAN_Remove_Device(deviceId);

    if(0 == qapi_WLAN_Enable(QAPI_WLAN_DISABLE_E))
    {
        QCLI_Printf(qcli_kpi_handle,"Wlan successfully disabled \n");
    }

    return QCLI_STATUS_SUCCESS_E;
}

uint32_t total_sleep_duration = 0, total_awake_time = 0, counter_kpt = 0;
uint32_t temp[100];

/* change between rec and perf modes */
uint32_t set_wlan_power_mode(boolean_t pwr_mode, uint8_t pwr_module)
{
    qapi_WLAN_Power_Mode_Params_t pwrMode;
    qapi_Status_t status;
    pwrMode.power_Mode = pwr_mode;
    pwrMode.power_Module = pwr_module;

    status = qapi_WLAN_Set_Param(0,
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS_POWER_MODE_PARAMS,
                               (void *) &pwrMode,
                               sizeof(pwrMode),
                               FALSE);

   if(status != QAPI_OK)
       return status;

   status = qapi_WLAN_Set_Param(deviceId,
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                               __QAPI_WLAN_PARAM_GROUP_WIRELESS_POWER_MODE_PARAMS,
                               (void *) &pwrMode,
                               sizeof(pwrMode),
                               FALSE);

   return status;
}

/* change wlan power policy */
uint32_t set_wlan_power_policy()
{
    qapi_WLAN_Power_Policy_Params_t pm;
    qapi_Status_t status;

    pm.idle_Period_In_Ms = 0;
    pm.ps_Poll_Num = 10;
    pm.dtim_Policy = QAPI_WLAN_DTIM_STICK_E;
    pm.tx_Wakeup_Policy = QAPI_WLAN_TX_WAKEUP_SKIP_ON_SLEEP_E;
    pm.num_Tx_To_Wakeup = 1;
    pm.ps_Fail_Event_Policy = 2;

    status = qapi_WLAN_Set_Param(deviceId,
                            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                            __QAPI_WLAN_PARAM_GROUP_WIRELESS_POWER_MODE_POLICY,
                            &pm,
                            sizeof(qapi_WLAN_Power_Policy_Params_t),
                            FALSE);

    if(status != QAPI_OK)
        return status;

    status = qapi_WLAN_Set_Param(0,
                            __QAPI_WLAN_PARAM_GROUP_WIRELESS,
                            __QAPI_WLAN_PARAM_GROUP_WIRELESS_POWER_MODE_POLICY,
                            &pm,
                            sizeof(qapi_WLAN_Power_Policy_Params_t),
                            FALSE);

     return status;
}

/*
 * kpi_suspend_time - Helper function to calculate the total
 * suspend time remaining babsed on input params
 */
uint32_t kpi_suspend_time()
{
    uint32_t duration = 0;          /* in ticks */
    uint32_t total_tx_time = 0;     /* in msec */
    uint32_t last_tick = kpi_ctx->pktStats.last_time.ticks;
    uint32_t first_tick = kpi_ctx->pktStats.first_time.ticks;
    uint32_t time_interval = 1000; /* 1000 ms total duration */
    uint32_t time_to_suspend = 0;
    qapi_Status_t status;
    int32_t multiplier = RTC_CLOCK_MULTIPLIER;
    int32_t divider = RTC_CLOCK_DIVIDER;

    if (last_tick < first_tick)
    {
        /* Assume the systick wraps around once */
        duration = ~first_tick + 1 + last_tick;
    }
    else
    {
        duration = last_tick - first_tick;
    }

    total_tx_time = (duration*multiplier)/divider;
    temp[counter_kpt++] = total_tx_time;
    total_awake_time += total_tx_time;

    /* already took us 1 sec to transmit the required rate */
    if(total_tx_time > time_interval)
    {
        kpi_get_time(&kpi_ctx->pktStats.first_time);
        return TRUE;
    }

    time_to_suspend = time_interval - total_tx_time;

    /* optimization, M4 can be put in sleep here, before going to suspend */
    status = qapi_Slp_Set_Max_Latency(QAPI_SLP_LAT_POWER);

    if(status != QAPI_OK)
    {
        if(PAL_Uart_Init() == TRUE)
        {
            QCLI_Printf(qcli_kpi_handle, "Test failed \n");
            return FALSE;
        }
    }

    total_sleep_duration += time_to_suspend;

    /* start the suspend process */
    qapi_Task_Delay(time_to_suspend*1000);

    /* optimization, set m4 in perf mode */
    status = qapi_Slp_Set_Max_Latency(QAPI_SLP_LAT_PERF);

    if(status != QAPI_OK)
    {
        if(PAL_Uart_Init() == TRUE)
        {
            QCLI_Printf(qcli_kpi_handle, "Test failed \n");
            return FALSE;
        }
    }

    kpi_get_time(&kpi_ctx->pktStats.first_time);

    return TRUE;
}

/* 
 * kpi_udp_cumulative_tx - Helper function for cumulative throughput test
 */
void kpi_udp_cumulative_tx()
{
    struct sockaddr_in *foreign_addr = NULL;
    struct sockaddr *to;
    uint32_t tolen;
    char ip_str[20];
    int32_t send_bytes;
    uint32_t packet_size = 1400;
    uint32_t cur_packet_number = 0,n_send_ok;
    uint32_t netbuf_id;
    int send_flag=0;
    int family;
    int tos_opt;
    struct sockaddr_in *src_sin = NULL;
    KPI_EOT_PACKET eot_packet, *endmark;
    uint32_t is_test_done = 0;
    uint32_t total_packet_count = 0;
    qapi_Status_t status;

    total_sleep_duration = 0;
    total_awake_time = 0;
    counter_kpt = 0;

    endmark = &eot_packet;

    family = AF_INET;
    memcpy(ip_str, kpi_ctx->ip, 20);

    foreign_addr = malloc(sizeof(struct sockaddr));
    memset(foreign_addr, 0, sizeof(struct sockaddr));
    foreign_addr->sin_addr.s_addr    = kpi_ctx->params.tx_params.ip_address;
    foreign_addr->sin_port           = htons(kpi_ctx->params.tx_params.port);
    foreign_addr->sin_family         = family;

    src_sin = malloc(sizeof(struct sockaddr));
    memset(src_sin, 0, sizeof(struct sockaddr));
    src_sin->sin_family              = family;
    src_sin->sin_addr.s_addr         = kpi_ctx->params.tx_params.source_ipv4_addr;
    src_sin->sin_port                = htons(0);

    to = (struct sockaddr *)foreign_addr;
    tolen = sizeof(struct sockaddr);
    tos_opt = IP_TOS;

    /* Create UDP socket */
    if ((kpi_ctx->sock_peer = qapi_socket(family, SOCK_DGRAM, 0)) == -1)
    {
        QCLI_Printf(qcli_kpi_handle, "Socket creation failed\n");
        goto ERROR_1;
    }

    if (kpi_ctx->params.tx_params.source_ipv4_addr != 0) {
        if (qapi_bind(kpi_ctx->sock_peer, (struct sockaddr*)src_sin, sizeof(struct sockaddr)) == -1) {
            QCLI_Printf(qcli_kpi_handle, "Socket bind failed\n");
            goto ERROR_2;
        }
    }

    if (kpi_ctx->params.tx_params.ip_tos > 0)
    {
        qapi_setsockopt(kpi_ctx->sock_peer, IP_OPTIONS, tos_opt, &kpi_ctx->params.tx_params.ip_tos, sizeof(uint8_t));
    }

    /* Connect to the server.*/
    QCLI_Printf(qcli_kpi_handle, "Connecting\n");
    if (qapi_connect( kpi_ctx->sock_peer, to, tolen) == -1)
    {
        QCLI_Printf(qcli_kpi_handle, "Connection failed\n");
        goto ERROR_2;
    }

    /* Sending.*/
    QCLI_Printf(qcli_kpi_handle, "Sending\n");

    /* WLAN is put in rec power mode here */
    status = set_wlan_power_mode(QAPI_WLAN_POWER_MODE_REC_POWER_E, QAPI_WLAN_POWER_MODULE_USER_E);

    if(status != QAPI_OK)
        goto ERROR_2;

    status = set_wlan_power_policy();

    if(status != QAPI_OK)
        goto ERROR_2;

    /* uart disable */
    QCLI_Printf(qcli_kpi_handle, "Disabling Uart \n");

    if(PAL_Uart_Deinit() != QAPI_OK) {
       goto ERROR_2;
    }

    netbuf_id = QAPI_NETBUF_APP;

    /*Reset all counters*/
    cur_packet_number = 0;
    n_send_ok = 0;

    status = qapi_Slp_Set_Max_Latency(QAPI_SLP_LAT_PERF);

    if(status != QAPI_OK)
        goto ERROR_2;

    kpi_get_time(&kpi_ctx->pktStats.first_time);

    while ( !is_test_done )
    {

        if (kpi_quit)
        {
            kpi_get_time(&kpi_ctx->pktStats.last_time);
            break;
        }

        /* allocate the buffer, if needed */
        if ( kpi_ctx->buffer == NULL )
        {

             while ((kpi_ctx->buffer = qapi_Net_Buf_Alloc(packet_size, netbuf_id)) == NULL) {
                /*Wait till we get a buffer*/
                if (kpi_quit)
                {
                    kpi_get_time(&kpi_ctx->pktStats.last_time);
                    goto ERROR_2;
                }
             }

            /* Update net buffer:
             *
             * [START]<4-byte Packet Index><4-byte Packet Size>000102..FF000102..FF0001..[END]
             * Byte counts: 8 + 4 + 4 + (packet_size-22) + 6
             */
             char *pkt_start = "[START]";
             char *pkt_end = "[END]";
             uint32_t val;


             /* Add "[START]" */
             qapi_Net_Buf_Update(kpi_ctx->buffer, 0, pkt_start, 8, netbuf_id);

             /* Packet index */
             val = htonl(cur_packet_number);
             qapi_Net_Buf_Update(kpi_ctx->buffer, 8, &val, 4, netbuf_id);

             /* Packet size */
             val = htonl(packet_size);
             qapi_Net_Buf_Update(kpi_ctx->buffer, 12, &val, 4, netbuf_id);
                        

             /* Add pattern
              * The pattern is repeated 00 01 02 03 .. FE FF
              */
             uint8_t data[] = {0xaa, 0xbb, 0xcc, 0xdd};
             qapi_Net_Buf_Update(kpi_ctx->buffer, 16, data, packet_size - 16 - 6, netbuf_id);

             /* Add "[END]" */
             qapi_Net_Buf_Update(kpi_ctx->buffer, packet_size-6, pkt_end, 6, netbuf_id);
        }
        else 
        {
            uint32_t val;
            uint32_t idx;

            idx = 8;

            /* Packet index */
            val = htonl(cur_packet_number);
            qapi_Net_Buf_Update(kpi_ctx->buffer, idx, &val, 4, netbuf_id);
        }
        do
        {
            
            {
                send_bytes = qapi_send(kpi_ctx->sock_peer, kpi_ctx->buffer, packet_size, send_flag);
            }

            if ( send_bytes != packet_size )
            {
                int errno = qapi_errno(kpi_ctx->sock_peer);
                if ( (EPIPE == errno) ||
                     (ENOTSOCK == errno) ||
                     (EBADF == errno) ||
                     (EIEIO == errno))
                {
                    if(PAL_Uart_Init() == TRUE) {
                        QCLI_Printf(qcli_kpi_handle, "\nError: send_bytes=%d, errno=%d\n", send_bytes, errno);
                    }
                    kpi_ctx->buffer = NULL;
                    is_test_done = 1;
                    kpi_get_time(&kpi_ctx->pktStats.last_time);
                    break;
                }
                else
                {
                    if ( ENOBUFS != errno )
                    {
                        if(PAL_Uart_Init() == TRUE) {
                            QCLI_Printf(qcli_kpi_handle, "\nFailed to qapi_send, send_bytes=%d, errno=%d\n", send_bytes, errno);
                        }
                    }

                    /* severe push back, let the other processes run (won't happen on blocking sockets) */
                    qapi_Task_Delay(1000);
                }
            }
            else
            {
                cur_packet_number ++;
                total_packet_count++;
            }

            if (send_bytes > 0)
            {
                kpi_ctx->pktStats.bytes += send_bytes;
                ++n_send_ok;
            }

            /*Test mode can be "number of packets" or "fixed time duration"*/
            if (kpi_ctx->params.tx_params.test_mode == PACKET_TEST)
            {
                if ((cur_packet_number >= kpi_ctx->params.tx_params.packet_number))
                {
                    kpi_get_time(&kpi_ctx->pktStats.last_time);

                    if(kpi_suspend_time() == FALSE) /* now suspend for rest of 1 second interval */
                        break;
                    cur_packet_number = 0;
                }
            }

            if (kpi_ctx->params.tx_params.interval_us)
                qapi_Task_Delay(kpi_ctx->params.tx_params.interval_us);

            /* end the test */
            if(total_packet_count > kpi_ctx->total_packet_count) {
                is_test_done = TRUE;
                break;
            }

        } while ( !((is_test_done) || (send_bytes == packet_size) || (NULL == kpi_ctx->buffer)) );   /* send loop */

    } 

    if ( kpi_ctx->buffer != NULL)
    {
        qapi_Net_Buf_Free(kpi_ctx->buffer, netbuf_id);
        kpi_ctx->buffer = NULL;
    }

ERROR_2:

    if(PAL_Uart_Init() != TRUE)
        return;

    QCLI_Printf(qcli_kpi_handle, "Sending test complete notification \n");

    /* send endmark packet to print details stats on ath_console side */
    ((KPI_EOT_PACKET*)endmark)->code    = END_OF_TEST_CODE;
    ((KPI_EOT_PACKET*)endmark)->packet_count = htonl(total_packet_count);

    cur_packet_number = 20;

    while(cur_packet_number != 0)
    {
        qapi_send(kpi_ctx->sock_peer, (char *)endmark, sizeof(KPI_EOT_PACKET), 0);
        qapi_Task_Delay(10000);
        cur_packet_number--;
    }

    qapi_socketclose(kpi_ctx->sock_peer);

    if(src_sin != NULL)
        free(src_sin);

    if(foreign_addr != NULL)
        free(foreign_addr);

ERROR_1:

    QCLI_Printf(qcli_kpi_handle, "Total time spent in tx = %d seconds \n", counter_kpt);

    for(int i=0;i<counter_kpt;i++)
    {
        QCLI_Printf(qcli_kpi_handle, "%d second, awake time %d ms \n", i, temp[i]);
    }

    QCLI_Printf(qcli_kpi_handle, "Total sleep duration= %d ms \n", total_sleep_duration);
    QCLI_Printf(qcli_kpi_handle, "Total awake duration= %d ms \n", total_awake_time);
    QCLI_Printf(qcli_kpi_handle, "Test Completed \n");

    return;
}

