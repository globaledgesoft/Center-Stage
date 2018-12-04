/* RAM_START  = 0x10000000; */
/* RAM_LENGTH = 0x000B8000; */
/* XIP_START  = 0x10100000; */
/* XIP_LENGTH = 0x00080000; */

MEMORY
{
   RAM_AON_PATCHABLE_RO_MEMORY                       (W)  : ORIGIN = 0x10001600, LENGTH = 0x000004C0
   RAM_AON_RO_MEMORY                                 (W)  : ORIGIN = 0x10001AC0, LENGTH = 0x00000140
   RAM_AON_PATCHABLE_RW_MEMORY                       (W)  : ORIGIN = 0x10001C00, LENGTH = 0x00000180
   RAM_AON_ROM2RAM_INDIRECTION_TABLE_RW_MEMORY       (W)  : ORIGIN = 0x10001DC0, LENGTH = 0x00000004
   RAM_AON_MOM_DATA_MEMORY                           (W)  : ORIGIN = 0x10001DC4, LENGTH = 0x000002DC

   RAM_SOM_BSP_ROM_PATCHTABLE_RW_MEMORY              (W)  : ORIGIN = 0x10002F00, LENGTH = 0x00000D00
   RAM_SOM_BSP_DATA_MEMORY                           (W)  : ORIGIN = 0x10003C00, LENGTH = 0x00000500

   RAM_SOM_BSP_ZI_MEMORY                             (W)  : ORIGIN = 0x10005160, LENGTH = 0x000000A0
   RAM_SOM_BSP_ROM2RAM_INDIRECTION_TABLE_RW_MEMORY   (W)  : ORIGIN = 0x10004100, LENGTH = 0x00000040
   RAM_SOM_DEVCFG_DATA_RW_MEMORY                     (W)  : ORIGIN = 0x10004140, LENGTH = 0x000006C0

   RAM_SOM_BSP_PATCHROM_RO_MEMORY                    (RX) : ORIGIN = 0x10004800, LENGTH = 0x00001400
   RAM_BSP_VENEER_RO_MEMORY                          (RX) : ORIGIN = 0x10005C00, LENGTH = 0x00000800
   RAM_SOM_BSP_RO_MEMORY                             (RX) : ORIGIN = 0x10006400, LENGTH = 0x00001200

   RAM_SOM_BSP_DIAG_BUF_MEMORY                       (W)  : ORIGIN = 0x1000E400, LENGTH = 0x00000280
   RAM_SOM_BSP_HEAP_ZI_MEMORY                        (W)  : ORIGIN = 0x1000E680, LENGTH = 0x00001200
   RAM_SOM_BSP_STACK_ZI_MEMORY                       (W)  : ORIGIN = 0x1000F880, LENGTH = 0x00000780

   RAM_SOM_APPS_RO_MEMORY                            (RX) : ORIGIN = 0x10007600, LENGTH = 0x00005A00
   RAM_SOM_APPS_DATA_MEMORY                          (W)  : ORIGIN = 0x1000D000, LENGTH = 0x00001400
   RAM_SOM_APPS_ZI_MEMORY                            (W)  : ORIGIN = 0x1000F800, LENGTH = 0x00002800

   RAM_FOM_BSP_ROM_PATCHTABLE_RW_MEMORY              (W)  : ORIGIN = 0x10013400, LENGTH = 0x00003000
   RAM_FOM_BSP_ROM2RAM_INDIRECTION_TABLE_RW_MEMORY_0 (W)  : ORIGIN = 0x10016400, LENGTH = 0x00000200
   RAM_FOM_BSP_ROM2RAM_INDIRECTION_TABLE_RW_MEMORY_1 (W)  : ORIGIN = 0x10016600, LENGTH = 0x00000200
   RAM_FOM_DEVCFG_DATA_RW_MEMORY                     (W)  : ORIGIN = 0x1001D400, LENGTH = 0x00000C00
   RAM_FOM_BSP_DATA_MEMORY                           (W)  : ORIGIN = 0x10016800, LENGTH = 0x00006C00

   RAM_FOM_BSP_HEAP_MEMORY                           (W)  : ORIGIN = 0x10090000, LENGTH = 0x0001F800
   RAM_FOM_BSP_STACK_ZI_MEMORY                       (W)  : ORIGIN = 0x100AF800, LENGTH = 0x00000800

   RAM_SBL_RAMDUMP_DATA_MEMORY                       (W)  : ORIGIN = 0x1001E000, LENGTH = 0x00002000

   RAM_FOM_BSP_PATCHROM_RO_MEMORY                    (RX) : ORIGIN = 0x10020000, LENGTH = 0x00005000
   RAM_FOM_BSP_RO_MEMORY                             (RX) : ORIGIN = 0x10025000, LENGTH = 0x0000C000
   RAM_FOM_APPS_RO_MEMORY                            (RX) : ORIGIN = 0x10031000, LENGTH = 0x00055000
   RAM_FOM_APPS_DATA_MEMORY                            (W)  : ORIGIN = 0x10086000, LENGTH = 0x0000A000
   RAM_FOM_APPS_ZI_MEMORY                            (W)  : ORIGIN = 0x10082000, LENGTH = 0x0000E000

   RAM_SBL_RO_MEMORY                                 (RX) : ORIGIN = 0x100A0000, LENGTH = 0x00002000
   RAM_SBL_RW_MEMORY                                 (W)  : ORIGIN = 0x100A2000, LENGTH = 0x00006000
   RAM_SBL_ZI_MEMORY                                 (W)  : ORIGIN = 0x100A8000, LENGTH = 0x00002000
   SBL_LOG_META_INFO_RW_MEMORY                       (W)  : ORIGIN = 0x100AA000, LENGTH = 0x00000040
   SBL_LOG_BUF_RW_MEMORY                             (W)  : ORIGIN = 0x100AA040, LENGTH = 0x00001000

   XIP_RO_MEMORY                                     (RX) : ORIGIN = 0x1000000, LENGTH = 0x00078000
   XIP_OEM_RO_MEMORY                                 (RX) : ORIGIN = 0x1078000, LENGTH = 0x00088000
   RAM_DIAG_STRING_RO_MEMORY                         (W)  : ORIGIN = 0x40000000,LENGTH = 0x00002000
}

PHDRS
{
   XIP_SEGMENT                         PT_LOAD;
   RAM_AON_PATCHABLE_RW_SEGMENT        PT_LOAD;
   RAM_AON_PATCHABLE_RO_SEGMENT        PT_LOAD;
   RAM_AON_RO_SEGMENT               PT_LOAD;
   RAM_AON_DATA_SEGMENT             PT_LOAD;
   RAM_SOM_PATCH_DATA_SEGMENT    PT_LOAD;
   RAM_SOM_BSP_CODE_SEGMENT      PT_LOAD;
   RAM_SOM_BSP_DATA_SEGMENT      PT_LOAD;
   RAM_SOM_PATCHROM_CODE_SEGMENT PT_LOAD;
   RAM_SOM_APPS_CODE_SEGMENT     PT_LOAD;
   RAM_SOM_APPS_DATA_SEGMENT     PT_LOAD;
   RAM_FOM_PATCH_DATA_SEGMENT    PT_LOAD;
   RAM_FOM_BSP_DATA_SEGMENT      PT_LOAD;
   RAM_FOM_BSP_CODE_SEGMENT      PT_LOAD;
   RAM_FOM_PATCHROM_CODE_SEGMENT PT_LOAD;
   RAM_BSP_VENEER_SEGMENT        PT_LOAD;
   RAM_FOM_APPS_CODE_SEGMENT     PT_LOAD;
   RAM_FOM_APPS_DATA_SEGMENT     PT_LOAD;
   RAM_SBL_RAMDUMP_SEGMENT       PT_LOAD;
   RAM_SBL_DATA_SEGMENT          PT_LOAD;
   RAM_SBL_CODE_SEGMENT          PT_LOAD;
   RAM_DIAG_STRING_SEGMENT       PT_LOAD;
   EXIDX_SEGMENT                 0x70000001 FLAGS(4);
}

SECTIONS
{
   XIP_OEM_RO_REGION :
   {
      *:pka.o(.text)
      *:ecc_lock.o(.text)
      *:elppka.o(.text)
      *:elppka.o(.constdata)
      *:ed25519.o(.text)
      *:ed25519.o(.constdata)
      *:ed25519.o(.conststring)
      *:ed25519.o(.rodata)
      *:ecjpake_hw.o(.text)
      *:ecjpake_hw.o(.constdata)
      *:ecjpake_hw.o(.conststring)
      *:ecjpake_hw.o(.rodata)      
      *:SharkSslCon_hs.o(.text)
      *:SharkSslCon_hs.o(.conststring)
      *:SharkSslCon_hs.o(.constdata)
      *:SharkSslCert.o(.text)
      *:SharkSslCert.o(.conststring)
      *:SharkSslBigInt.o(.text)
      *:SharkSslBigInt.o(.conststring)
      *:SharkSslECC.o(.text)
      *:SharkSslECC.o(.conststring)
      *:SharkSslPublic.o(.text)
      *:SharkSslPublic.o(.conststring)
      *:SharkSslPublic.o(.constdata)
      *:SharkSslCon_DTLS_hs.o(.text)
      *:SharkSslCon_DTLS_hs.o(.constdata)
      *:cmac_hw.o(.text)
      *:cmac_hw.o(.constdata)
      *:cmac_hw.o(.conststring)
      *:cmac_hw.o(.rodata)
      *:securefs.o(.text)
      *:securefs.o(.constdata)
      *:securefs.o(.conststring)
      *:securefs.o(.rodata)
      *:securefs_cipher.o(.text)
      *:securefs_cipher.o(.constdata)
      *:securefs_cipher.o(.conststring)
      *:securefs_cipher.o(.rodata)
      *:securefs_hmac.o(.text)
      *:securefs_hmac.o(.constdata)
      *:securefs_hmac.o(.conststring)
      *:securefs_hmac.o(.rodata)
      *:tee_securefs.o(.text)
      *:tee_securefs.o(.constdata)
      *:tee_securefs.o(.conststring)
      *:tee_securefs.o(.rodata)
      *:qapi_firmware_upgrade.o(.text)
      *:qapi_ota.o(.text)
      *:ota.o(.text)
      *:time.o(.text)
      *crypto_demo.o(.text)
      *crypto_demo.o(.conststring)
      *crypto_demo.o(.constdata)
      *crypto_demo.o(.rodata)
      *fs_demo.o(.text)
      *fs_demo.o(.conststring)
      *fs_demo.o(.constdata)
      *fs_demo.o(.rodata)
      *securefs_demo.o(.text)
      *securefs_demo.o(.conststring)
      *securefs_demo.o(.constdata)
      *securefs_demo.o(.rodata)
      *pal.o(.text)
      *qcli.o(.text)
      *pal.o(.rodata)
      *qcli.o(.rodata)
      *qcli_util.o(.text)
      *qcli_util.o(.rodata)
      *hmi_demo.o(.text)
      *hmi_demo.o(.rodata)
      *ble_demo.o(.text)
      *ble_demo.o(.rodata)
      *sbrk.o(.text)
      *(XIP_OEM_RO_SECTION)
   } >XIP_OEM_RO_MEMORY : XIP_SEGMENT

   /* RAM_AON_CONFIG_SEGMENT */
   RAM_AON_PATCHABLE_RW_REGION :
   {
      *mom_patch_table*.o(.data)
      *(RAM_AON_BSP_RW_SECTION)
   } >RAM_AON_PATCHABLE_RW_MEMORY : RAM_AON_PATCHABLE_RW_SEGMENT

   RAM_AON_PATCHABLE_RO_REGION :
   {
      *(RAM_AON_BSP_RO_SECTION)
   } >RAM_AON_PATCHABLE_RO_MEMORY : RAM_AON_PATCHABLE_RO_SEGMENT

   RAM_AON_RO_REGION :
   {
      *:platform_ramdump_entry.o(.text)
      *:ram_function_table_mom.o(.text)
      *:platform_oem_mom.o(.text)
      *wakeup_handler.o(.text)
      *:mom_aon.o(.text)
      *:mom_aon.o(.emb_text)
      *(RAM_AON_BSP_RO_SECTION)
   } >RAM_AON_RO_MEMORY : RAM_AON_RO_SEGMENT


   RAM_AON_ROM2RAM_INDIRECTION_TABLE_RW_REGION :
   {
      *(RAM_AON_ROM2RAM_INDIRECTION_TABLE_RW_SECTION)
   } >RAM_AON_ROM2RAM_INDIRECTION_TABLE_RW_MEMORY : RAM_AON_DATA_SEGMENT

   RAM_AON_MOM_RW_REGION :
   {
      *:mom_aon.o(.data)
      *:platform_ramdump_entry.o(.data)
      *:ram_function_table_mom.o(.data)
      *wakeup_handler.o(.data)
      *:platform_omt.o(.data)
      *platform_oem_mom.o(.data)
      *(RAM_AON_MOM_RW_SECTION)
   } >RAM_AON_MOM_DATA_MEMORY : RAM_AON_DATA_SEGMENT


 /* RAM_AON_CONFIG_SEGMENT */
   RAM_AON_MOM_ZI_REGION :
   {
      *wakeup_handler.o(.bss)
      *:platform_omt.o(.bss)
      *mom_lp_test.o(.bss)
      *:mom_aon.o(.bss)
      *:ram_function_table_mom.o(.bss)
      *platform_oem_mom.o(.bss)
      *:platform_ramdump_entry.o(.bss)
      *(RAM_AON_MOM_ZI_SECTION)
   } >RAM_AON_MOM_DATA_MEMORY : RAM_AON_DATA_SEGMENT

   /* RAM_SOM_PATCH_DATA_SEGMENT */
   RAM_SOM_BSP_ROM_PATCHTABLE_RW_REGION :
   {
      *som_patch_table_*.o(.data)

      *(RAM_SOM_BSP_ROM_PATCHTABLE_RW_SECTION)
   } >RAM_SOM_BSP_ROM_PATCHTABLE_RW_MEMORY : RAM_SOM_PATCH_DATA_SEGMENT

   /* RAM_SOM_BSP_DATA_SEGMENT */
   VECTOR_TABLE_RW_REGION :
   {
      *(VECTOR_TABLE_RW_SECTION)
   } >RAM_SOM_BSP_DATA_MEMORY : RAM_SOM_BSP_DATA_SEGMENT

   RAM_SOM_BSP_ROM2RAM_INDIRECTION_TABLE_RW_REGION :
   {
      *(RAM_SOM_BSP_ROM2RAM_INDIRECTION_TABLE_RW_SECTION)
   } >RAM_SOM_BSP_ROM2RAM_INDIRECTION_TABLE_RW_MEMORY : RAM_SOM_BSP_DATA_SEGMENT

   RAM_SOM_BSP_RW_REGION :
   {
      *:som_init_internal.o(.data)
      *diag_msg_ram.o(.data)
      *:malloc.o(.data)
      *:qapi_dsr.o(.data)
      *qurt_interrupt_asm.o(.data)
      *:platform_ram_data_init.o(.data)
      *diag_ram.o(.data)
      *:pmu_init.o(.data)
      *:timer_log.o(.data)
      *:qurt_interrupt.o(.data)
      *:InterruptController_Internal_m4.o(.data)
      *:qurt_power_shutdown.o(.data)
      *:platform_sleep_control.o(.data)
      *platform_ram_data.o(.data)
      *qurt_power_shutdown_asm.o(.data)
      *ram_function_table_som.o(.data)
      *(RAM_SOM_BSP_RW_SECTION)
   } >RAM_SOM_BSP_DATA_MEMORY : RAM_SOM_BSP_DATA_SEGMENT

   RAM_SOM_BSP_ZI_REGION :
   {
      *:timer_log.o(.bss)
      *:malloc.o(.bss)
      *:qapi_dsr.o(.bss)
      *:platform_sleep_control.o(.bss)
      *diag_msg_ram.o(.bss)
      *:pmu_init.o(.bss)
      *qurt_power_shutdown_asm.o(.bss)
      *:qurt_interrupt.o(.bss)
      *qurt_interrupt_asm.o(.bss)
      *:som_init_internal.o(.bss)
      *platform_ram_data.o(.bss)
      *:qurt_power_shutdown.o(.bss)
      *:InterruptController_Internal_m4.o(.bss)
      *diag_ram.o(.bss)
      *:platform_ram_data_init.o(.bss)
      *ram_function_table_som.o(.bss)
      *(.bss.RAM_SOM_BSP_ZI_SECTION)
   } >RAM_SOM_BSP_DATA_MEMORY : RAM_SOM_BSP_DATA_SEGMENT

   RAM_SOM_DEVCFG_DATA_RW_REGION :
   {
      *devcfg_devcfg_data.o(.rodata)
      *devcfg_devcfg_data.o(.text)
      *DALConfig_devcfg.o(.rodata)
      *DALConfig_devcfg.o(.text)
      *DALConfig_devcfg.o(.data)
      *devcfg_devcfg_cfg1.o(.constdata)
      *devcfg_devcfg_cfg1.o(.text)
      *devcfg_devcfg_cfg1.o(.data)
      *devcfg_devcfg_cfg1.o(.bss)
      *devcfg_devcfg_IPCInt_Config.o(.text)
      *devcfg_devcfg_IPCInt_Config.o(.data)
      *devcfg_devcfg_IPCInt_Config.o(.bss)
      *devcfg_devcfg_AdcBsp.o(.constdata)
      *devcfg_devcfg_AdcBsp.o(.text)
      *devcfg_devcfg_AdcBsp.o(.data)
      *devcfg_devcfg_AdcBsp.o(.bss)
      *devcfg_devcfg_InterruptController_Config.o(.text)
      *devcfg_devcfg_InterruptController_Config.o(.data)
      *devcfg_devcfg_InterruptController_Config.o(.bss)
      *devcfg_devcfg_GPIOInt_Config.o(.text)
      *devcfg_devcfg_GPIOInt_Config.o(.data)
      *devcfg_devcfg_GPIOInt_Config.o(.bss)
      *devcfg_devcfg_spi1_config.o(.constdata)
      *devcfg_devcfg_spi1_config.o(.text)
      *devcfg_devcfg_spi1_config.o(.data)
      *devcfg_devcfg_spi1_config.o(.bss)
      *devcfg_devcfg_TsensBsp.o(.constdata)
      *devcfg_devcfg_TsensBsp.o(.text)
      *devcfg_devcfg_TsensBsp.o(.data)
      *devcfg_devcfg_TsensBsp.o(.bss)
      *(RAM_SOM_DEVCFG_DATA_RW_SECTION)
   } >RAM_SOM_DEVCFG_DATA_RW_MEMORY : RAM_SOM_BSP_DATA_SEGMENT

   RAM_SOM_BSP_HEAP_ZI_REGION :
   {
      *(RAM_SOM_BSP_HEAP_ZI_SECTION)
   } >RAM_SOM_BSP_HEAP_ZI_MEMORY : RAM_SOM_BSP_DATA_SEGMENT

   RAM_SOM_BSP_STACK_ZI_REGION :
   {
      *(.bss.RAM_SOM_BSP_STACK_ZI_SECTION)
   } >RAM_SOM_BSP_STACK_ZI_MEMORY : RAM_SOM_BSP_DATA_SEGMENT

/* RAM_SOM_BSP_CODE_SEGMENT */
   RAM_SOM_BSP_RO_REGION :
   {
      *:platform_ram_data_init.o(.text)
      *diag_ram.o(.text)
      *:platform_ram_data.o(.text)
      *:platform_ram_data.o(PLATFROM_AON_DATA)
      *:pmu_init.o(.text)
      *diag_msg_ram.o(.text)
      *:qapi_mom.o(.text)
      *:malloc.o(.text)
      *qurt_interrupt.o(.text)
      *:qurt_power_shutdown.o(.text)
      *:qurt_power_shutdown.o(.constdata)
      *:qurt_power_shutdown.o(.conststring)
      *:platform_sleep_control.o(.text)
      *:platform_sleep_control.o(.constdata)
      *:platform_sleep_control.o(.conststring)
      *qurt_power_shutdown_asm.o(qurt_power_shutdown)
      *:som_init_internal.o(.text)
      *:som_init_internal.o(.emb_text)
      *:som_init_internal.o(.constdata)
      *:som_init_internal.o(.conststring)
      *qurt_interrupt_asm.o(.text)
      *qurt_interrupt_asm.o(IntCont)
      *:timer_log.o(.text)
      *:InterruptController_Internal_m4.o(.text)
      *ram_function_table_som.o(.text)
      *(RAM_SOM_BSP_RO_SECTION)
   } >RAM_SOM_BSP_RO_MEMORY : RAM_SOM_BSP_CODE_SEGMENT

   /* RAM_SOM_PATCHROM_CODE_SEGMENT */
   RAM_SOM_BSP_PATCHROM_RO_REGION :
   {
      *patch_platform.o(.text)
      *(RAM_SOM_BSP_PATCHROM_RO_SECTION)
   } >RAM_SOM_BSP_PATCHROM_RO_MEMORY : RAM_SOM_PATCHROM_CODE_SEGMENT

   /* RAM_BSP_VENEER_SEGMENT */
   RAM_BSP_VENEER_RO_REGION :
   {
      *:mom.o(.text)
      *:mom.o(.constdata)
      *:mom.o(.conststring)
   } >RAM_BSP_VENEER_RO_MEMORY : RAM_BSP_VENEER_SEGMENT

   /* RAM_SOM_APPS_CODE_SEGMENT */
   RAM_SOM_APPS_RO_REGION :
   {
      *\uSleep.o(.text)
      *\pal_som.o(.text)
      *\hmi_demo_som.o(.text)
      *(RAM_SOM_APPS_RO_SECTION)
   } >RAM_SOM_APPS_RO_MEMORY : RAM_SOM_APPS_CODE_SEGMENT

   /* RAM_SOM_APPS_DATA_SEGMENT */
   RAM_SOM_APPS_RW_REGION :
   {
      *\uSleep.o(.data)
      *\pal_som.o(.data)
      *\hmi_demo_som.o(.data)
     *(RAM_SOM_APPS_RW_SECTION)
   } >RAM_SOM_APPS_DATA_MEMORY : RAM_SOM_APPS_DATA_SEGMENT

   RAM_SOM_APPS_ZI_REGION :
   {
      *\uSleep.o(.bss)
      *\pal_som.o(.bss)
      *\hmi_demo_som.o(.bss)
     *(RAM_SOM_APPS_ZI_SECTION)
   } >RAM_SOM_APPS_DATA_MEMORY : RAM_SOM_APPS_DATA_SEGMENT

   /* RAM_FOM_PATCH_DATA_SEGMENT */
   RAM_FOM_BSP_ROM_PATCHTABLE_RW_REGION :
   {
      *fom_patch_table*.o(.data)
      *(RAM_FOM_BSP_ROM_PATCHTABLE_RW_SECTION)
   } >RAM_FOM_BSP_ROM_PATCHTABLE_RW_MEMORY : RAM_FOM_PATCH_DATA_SEGMENT

   /* RAM_FOM_BSP_DATA_SEGMENT */
   RAM_FOM_BSP_ROM2RAM_INDIRECTION_TABLE_RW_REGION_0 :
   {
      *(RAM_FOM_BSP_ROM2RAM_INDIRECTION_TABLE_RW_SECTION_0)
   } >RAM_FOM_BSP_ROM2RAM_INDIRECTION_TABLE_RW_MEMORY_0 : RAM_FOM_BSP_DATA_SEGMENT

   RAM_FOM_BSP_ROM2RAM_INDIRECTION_TABLE_RW_REGION_1 :
   {
      *(RAM_FOM_BSP_ROM2RAM_INDIRECTION_TABLE_RW_SECTION_1)
   } >RAM_FOM_BSP_ROM2RAM_INDIRECTION_TABLE_RW_MEMORY_1 : RAM_FOM_BSP_DATA_SEGMENT



   RAM_FOM_DEVCFG_DATA_RW_REGION :
   {
      *devcfg_fom_data.o(.rodata)
      *devcfg_fom_data.o(.text)
      *devcfg_fom_data.o(.data)
      *fom_devcfg_platform_props.o(.constdata)
      *fom_devcfg_platform_props.o(.text)
      *fom_devcfg_platform_props.o(.data)
      *DALConfig_fom.o(.rodata)
      *DALConfig_fom.o(.text)
      *DALConfig_fom.o(.data)
      *fom_devcfg_cfg2.o(.constdata)
      *fom_devcfg_cfg2.o(.text)
      *fom_devcfg_cfg2.o(.data)     
      *fom_devcfg_uart_quartz_config.o(.constdata)
      *fom_devcfg_uart_quartz_config.o(.text)
      *fom_devcfg_uart_quartz_config.o(.data)
      *fom_devcfg_spi2_config.o(.constdata)
      *fom_devcfg_spi2_config.o(.text)
      *fom_devcfg_spi2_config.o(.data)
      *(RAM_FOM_DEVCFG_DATA_RW_SECTION)
   } >RAM_FOM_DEVCFG_DATA_RW_MEMORY : RAM_FOM_BSP_DATA_SEGMENT

   /* RAM_FOM_BSP_STACK_SEGMENT */
   RAM_FOM_BSP_STACK_ZI_REGION :
   {
   } >RAM_FOM_BSP_STACK_ZI_MEMORY : RAM_FOM_BSP_STACK_SEGMENT

   /* RAM_FOM_PATCHROM_CODE_SEGMENT */
   RAM_FOM_BSP_PATCHROM_RO_REGION :
   {
      *patch_api_txrx.o(.text)
      *patch_driver_netbuf.o(.text)
      *patch_driver_txrx.o(.text)
      *patch_hcd_common.o(.text)
      *patch_sdio_hcd.o(.text)
      *patch_htc.o(.text)
      *patch_hw_api.o(.text)
      *patch_api_stack_offload.o(.text)
      *patch_cust_hw_sdio.o(.text)
      *patch_osal.o(.text)
      *patch_cust_api_stack_txrx.o(.text)
      *patch_cust_hw_api.o(.text)
      *patch_driver_main.o(.text)
      *patch_util.o(.text)
      *patch_cust_driver_main.o(.text)
      *patch_api_txrx.o(.rodata)
      *patch_driver_netbuf.o(.rodata)
      *patch_driver_txrx.o(.rodata)
      *patch_hcd_common.o(.rodata)
      *patch_sdio_hcd.o(.rodata)
      *patch_htc.o(.rodata)
      *patch_hw_api.o(.rodata)
      *patch_api_stack_offload.o(.rodata)
      *patch_cust_hw_sdio.o(.rodata)
      *patch_osal.o(.rodata)
      *patch_cust_api_stack_txrx.o(.rodata)
      *patch_cust_hw_api.o(.rodata)
      *patch_driver_main.o(.rodata)
      *patch_util.o(.rodata)
      *patch_cust_driver_main.o(.rodata)
   } >RAM_FOM_BSP_PATCHROM_RO_MEMORY : RAM_FOM_PATCHROM_CODE_SEGMENT



   /* RAM_SBL_RAMDUMP_SEGMENT */
   RAM_SBL_RAMDUMP_RW_REGION :
   {
      *:boot_dload_debug.o(.data)
      *:qusb_al_bulk.o(.data)
      *:boot_extern_qusb_interface.o(.data)
      *:boot_sahara.o(.data)
      *:boot_ramdump.o(.data)
      *:usb_sbl_mem_util.o(.data)
      *:sbl1_sahara.o(.data)
      *:boot_sahara_impl.o(.data)
      *:boot_sahara_impl.o(.bss.RAMDUMP_USB_BUFFER)
      *(RAM_SBL_RAMDUMP_RW_SECTION)
   } >RAM_SBL_RAMDUMP_DATA_MEMORY : RAM_SBL_RAMDUMP_SEGMENT

   RAM_SBL_RAMDUMP_ZI_REGION :
   {
      *:qusb_al_bulk.o(.bss)
      *:sbl1_sahara.o(.bss)
      *:boot_dload_debug.o(.bss)
      *:boot_sahara.o(.bss)
      *:boot_extern_qusb_interface.o(.bss)
      *:boot_ramdump.o(.bss)
      *:usb_sbl_mem_util.o(.bss)
      *:boot_sahara_impl.o(.bss)
      *(RAM_SBL_RAMDUMP_ZI_SECTION)
   } >RAM_SBL_RAMDUMP_DATA_MEMORY : RAM_SBL_RAMDUMP_SEGMENT

   /* RAM_SBL_CODE_SEGMENT */
   RAM_SBL_RO_REGION :
   {
      *:sbl.o(SBL_VECTOR_TABLE)
      *:boot_logger.o(.text)
      *(QUARTZ_SBL_ENTRY)
      *:boot_logger_ram.o(.text)
      *:boot_logger_ram.o(.conststring)
      *:boot_logger_timer.o(.text)
      *:sbl.o(.text)
      *:boot_mpu.o(.text)
      *:boot_mpu.o(.conststring)
      *:boot_mpu.o(.constdata)
      *:boot_pbl.o(.text)
      *:boot_pbl.o(.conststring)
      *:sbl1_mc.o(.text)
      *:boot_stack.o(.text)
      *:boot_stack.o(i.boot_init_stack_chk_canary)
      *:sbl1_xip.o(.text)
      *:sbl1_xip.o(.constdata)
      *flash_init.o(.text)
      *flash_init_config.o(.text)
      *:sbl1_hw_4020.o(.text)
      *:sbl1_error_handler.o(.text)
      *:sbl1_error_handler.o(.conststring)
      *:boot_extern_crypto_interface.o(.text)
      *:boot_extern_crypto_interface.o(.constdata)
      *:sbl1_timer.o(.text)
     *(RAM_SBL_RO_SECTION)
   } >RAM_SBL_RO_MEMORY : RAM_SBL_CODE_SEGMENT

   /* RAM_DIAG_STRING_SEGMENT */
   RAM_DIAG_STRING_RO_REGION :
   {
      *(.diagmsg.format)
   } >RAM_DIAG_STRING_RO_MEMORY : RAM_DIAG_STRING_SEGMENT

   /* RAM_SBL_DATA_SEGMENT */
   RAM_SBL_RW_REGION :
   {
     *:boot_pbl.o(.data)
     *:boot_logger_ram.o(.data)
     *:boot_util.o(.data)
     *:sbl1_mc.o(.data)
     *:boot_logger.o(.data)
     *:sbl.o(.data)
     *:boot_extern_pmu_interface.o(.data)
     *:boot_extern_crypto_interface.o(.data)
     *:sbl1_xip.o(.data)
     *:boot_authenticator.o(.data)
     *:qfprom_test.o(.data)
     *:boot_logger_timer.o(.data)
     *:boot_error_handler.o(.data)
     *:boot_rollback_version.o(.data)
     *:boot_config.o(.bss.BOOT_HASH_SEGMENT)
     *:sbl1_config.o(.data)
     *:qfprom.o(.data)
     *:qfprom_target.o(.data)
     *:boot_mpu.o(.data)
     *:qfprom_fuse_region_4020.o(.data)
     *:boot_extern_sec_img_interface.o(.data)
     *:boot_stack.o(.data)
     *:HALqfprom.o(.data)
     *:boot_extern_secboot_interface.o(.data)
     *:boot_extern_apt_interface.o(.data)
     *:boot_config.o(.data)
     *flash_init.o(.data)
     *flash_init_config.o(.data)
     *:boot_stack.o(.data)
     *:sbl1_hw_4020.o(.data)
     *:sbl1_error_handler.o(.data)
     *:boot_timer.o(.data)
     *:boot_timer_impl.o(.data)
     *:boot_rollback_version.o(.data)
     *:boot_rollback_version_impl.o(.data)
     *:sbl1_error_handler.o(.bss.ERR_DATA)
     *:sbl1_timer.o(.data)
           *(.bss.BOOT_HASH_SEGMENT)
      *(RAM_SBL_DATA_SEGMENT_RW_SECTION)
   } >RAM_SBL_RW_MEMORY : RAM_SBL_DATA_SEGMENT

   RAM_SBL_ZI_REGION :
   {
      *:boot_logger.o(.bss)
      *:qfprom_fuse_region_4020.o(.bss)
      *:sbl1_config.o(.bss)
      *:boot_rollback_version.o(.bss)
      *:boot_mpu.o(.bss)
      *:boot_extern_apt_interface.o(.bss)
      *:boot_extern_crypto_interface.o(.bss)
      *:boot_error_handler.o(.data)
      *:boot_config.o(.bss)
      *:HALqfprom.o(.bss)
      *:boot_extern_pmu_interface.o(.bss)
      *:boot_pbl.o(.bss)
      *:boot_logger_timer.o(.bss)
      *:boot_extern_sec_img_interface.o(.bss)
      *:boot_logger_ram.o(.bss)
      *:boot_authenticator.o(.bss)
      *:sbl1_mc.o(.bss)
      *:boot_extern_secboot_interface.o(.bss)
      *:qfprom.o(.bss)
      *:sbl.o(.bss)
      *:boot_stack.o(.bss)
      *:qfprom_test.o(.bss)
      *:boot_util.o(.bss)
      *:sbl1_xip.o(.bss)
      *:qfprom_target.o(.bss)
      *flash_init.o(.bss)
      *flash_init_config.o(.bss)
      *:boot_stack.o(.bss)
      *:sbl1_hw_4020.o(.bss)
      *:sbl1_error_handler.o(.bss)
      *:boot_timer.o(.bss)
      *:boot_timer_impl.o(.bss)
      *:boot_rollback_version.o(.bss)
      *:boot_rollback_version_impl.o(.bss)
      *:sbl1_timer.o(.bss)
      *(RAM_SBL_ZI_SECTION)

   } >RAM_SBL_ZI_MEMORY : RAM_SBL_DATA_SEGMENT

   SBL_LOG_META_INFO_RW_SECTION :
   {
   } >SBL_LOG_META_INFO_RW_MEMORY : RAM_SBL_DATA_SEGMENT

   SBL_LOG_BUF_RW_SECTION :
   {
   } >SBL_LOG_META_INFO_RW_MEMORY : RAM_SBL_DATA_SEGMENT

/* XIP_SEGMENT */
   XIP_BSP_RO_REGION :
   {
      *:dhcpv6_loop.o(.text)
      *:sbl1_config.o(.text)
      *:elppka.o(.text)
      *:qusb_log.o(.text)
      *:dhcpv6_bld_send_msg.o(.text)
      *:usb_os_util.o(.text)
      *:boot_config.o(.text)
      *:boot_config.o(.conststring)
      *:dhcpv6_process_msg.o(.text)
      *:qapi_kpd.o(.text)
      *:qapi_ns_utils.o(.text)
      *:qusb_dci_4020.o(.text)
      *:qfprom_test.o(.text)
      *:fs_ffs_sb.o(.text)
      *:boot_authenticator.o(.text)
      *:boot_authenticator.o(.conststring)
      *:qapi_ns_gen_v6.o(.text)
      *:usb_mem_util.o(.text)
      *:qfprom_fuse_region_4020.o(.text)
      *:qfprom_fuse_region_4020.o(.constdata)
      *:qusb_fd.o(.text)
      *:ns_mod.o(.text)
      *:usb_common_os_util.o(.text)
      *:dhcpv6_mod.o(.text)
      *:usb_al_hlos_bulk.o(.text)
      *:boot_elf_loader.o(.text)
      *:boot_extern_secboot_interface.o(.text)
      *:sntpclnt.o(.text)
      *:dnssrvr.o(.text)
      *:ns_v4.o(.text)
      *:ns_utils.o(.text)
      *:boot_extern_sec_img_interface.o(.text)
      *:boot_extern_qusb_interface.o(.text)
      *:dhcpv6_utils.o(.text)
      *:dhcpv6_nt.o(.text)
      *:qusb_clock.o(.text)
      *:qapi_ns_gen_v4.o(.text)
      *:boot_rollback_version.o(.text)
      *:boot_rollback_version.o(.conststring)
      *:qfprom_target.o(.text)
      *:keypad.o(.text)
      *:sbl1_sahara.o(.text)
      *:sbl1_sahara.o(.conststring)
      *:qapi_socket.o(.text)
      *:qusb_dci_4020_hw_apis_sbl1.o(.text)
      *:qapi_sntpc.o(.text)
      *:boot_ramdump.o(.text)
      *:boot_extern_apt_interface.o(.text)
      *:boot_extern_crypto_interface.o(.text)
      *:boot_dload_debug.o(.text)
      *:boot_dload_debug.o(.conststring)
      *:boot_dload_debug.o(.constdata)
      *:boot_loader_mpu.o(.text)
      *:ecc_lock.o(.text)
      *:create_sockets.o(.text)
      *:qapi_dhcpv6c.o(.text)
      *:qusb_dci.o(.text)
      *:qapi_dhcpv4c.o(.text)
      *:dhcpv6_bld_msg.o(.text)
      *:boot_loader_xip.o(.text)
      *:boot_loader_xip.o(.constdata)
      *:boot_loader_m0.o(.text)
      *:usb_sbl_mem_util.o(.text)
      *:boot_util.o(.text)
      *:pka.o(.text)
      *:qapi_dnsc.o(.text)
      *:qapi_dnss.o(.text)
      *:qapi_httpsvr.o(.text)
      *:qapi_webs.o(.text)
      *:sl_v4.o(.text)
      *:net_devcfg.o(.text)
      *:net_devcfg.o(.rodata)
      *:net_devcfg.o(.conststring)
      *:net_devcfg.o(.constdata)
      *:nsport.o(.text)
      *:ce_http.o(.text)
      *:htauth.o(.text)
      *:htutils.o(.text)
      *:htmllib.o(.text)
      *:htpers.o(.text)
      *:http.o(.text)
      *:http_mod.o(.text)
      *:httpconn.o(.text)
      *:httpport.o(.text)
      *:wbs_mod.o(.text)
      *:wbscgi.o(.text)
      *:wbsupload.o(.text)
      *:webserver.o(.text)
      *htmldata.o(.text)
      *htmldata.o(.constdata)
      *htmldata.o(.conststring)
      *htmldata.o(.rodata)
      *cgi_showintf.o(.text)
      *cgi_showintf.o(.constdata)
      *cgi_showintf.o(.conststring)
      *cgi_showintf.o(.rodata)
      *:vfsfiles.o(.text)
      *:userpass.o(.text)
      *:HALqfprom.o(.text)
      *:qusb_dcd_ch9.o(.text)
      *:qusb_al_bulk.o(.text)
      *:sl_v6.o(.text)
      *:ed25519.o(.text)
      *:qfprom.o(.text)
      *:boot_rollback_img_set.o(.text)
      *:boot_sahara.o(.text)
      *:qapi_dhcpv4s.o(.text)
      *:dhcpv6_port.o(.text)
      *:boot_extern_pmu_interface.o(.text)
      *:boot_extern_flash_interface.o(.text)
      *:sl_cmn.o(.text)
      *:usb_compile_wrapper.o(.text)
      *:usb_wrapper_api.o(.text)
      *:fs_ffs_freemap.o(.text)
      *:fs_ffs.o(.text)
      *:fs_vnode.o(.text)
      *:fs_global_lock.o(.text)
      *:fs_osal_heap.o(.text)
      *:fs_util.o(.text)
      *:fs_ftl.o(.text)
      *:fs_ftl.o(.constdata)
      *:fs_ffs_db.o(.text)
      *:fs_ffs_db.o(.constdata)
      *:fs_ffs_inode.o(.text)
      *:fs_ffs_inode.o(.constdata)
      *:fs.o(.text)
      *:fs.o(.constdata)
      *:fs_ffs_buf.o(.text)
      *:fs_ffs_buf.o(.constdata)
      *:fs_bdev_spinor.o(.text)
      *:fs_bdev_sd.o(.text)
      *:fs_ffs_iter.o(.text)
      *:fs_bdev.o(.text)
      *:fs_desc.o(.text)
      *:fs_ftl_flash.o(.text)
      *:fs_mount.o(.text)
      *:fs_namei.o(.text)
      *:fs_api.o(.text)
      *:fs_osal.o(.text)
      *:fs_bdev_spinor.o(.text)
      *:fs_ftl.o(.text)
      *:fs*.o(.conststring)
      *:qapi_fs.o(.text)
      *:spi_hcd.o(.text)
      *:cust_api_ioctl.o(.text)
      *:cust_platform_api.o(.text)
      *:cust_dset_nvram.o(.text)
      *:cust_profiler.o(.text)
      *:cust_ipstack_install.o(.text)
      *:cust_hw_spi.o(.text)
      *:qapi_wlan_base.o(.text)
      *:qapi_wlan_p2p.o(.text)
      *:qapi_wlan_prof.o(.text)
      *:wlan_qapi_helper.o(.text)
      *:cust_qapi.o(.text)
      *:cust_elf_loader.o(.text)
      *:cust_hw_api_ext.o(.text)
      *:fe_ge.o(.text)
      *:srp.o(.text)
      *:op_mgmt.o(.text)
      *:obj_mgmt.o(.text)
      *:qapi_ssl_cert.o(.text)
      *:shared_buffer.o(.text)
      *:shared_buffer.o(.constdata)
      *:shared_buffer.o(.conststring)
      *:shared_buffer.o(.rodata)
      *:ssl_cert_store.o(.text)
      *:ssl_store_common.o(.text)
      *:ssl_store_common.o(.constdata)
      *:ssl_store_common.o(.conststring)
      *:ssl_store_common.o(.rodata)
      *:strrcl_dnsc.o(.text)
      *:strrcl_dnss.o(.text)
      *:strrcl_sntpclt.o(.text)
      *:ns_bridge.o(.text)
      *:boot_timer.o(.text)
      *:boot_timer_impl.o(.text)
      *:boot_rollback_version.o(.text)
      *:boot_rollback_version_impl.o(.text)
      *:boot_loader_impl.o(.text)
      *:boot_loader.o(.text)
      *qapi_securefs.o(.text)
      *qapi_securefs.o(.rodata)
      *qapi_securefs.o(.conststring)
      *qapi_securefs.o(.constdata)
      *tlv_transport.o(.text)
      *tlv_transport.o(.rodata)
      *tlv_transport.o(.conststring)
      *tlv_transport.o(.constdata)
      *:crypto_devcfg.o(.text)
      *:crypto_devcfg.o(.rodata)
      *:crypto_devcfg.o(.conststring)
      *:crypto_devcfg.o(.constdata)
      *pka_host.o(.text)
      *pka_host.o(.rodata)
      *pka_host.o(.conststring)
      *pka_host.o(.constdata)
      *:fs_helper_m4.o(.text)
      *:fs_helper_m4.o(.rodata)
      *:fs_helper_m4.o(.conststring)
      *:fs_helper_m4.o(.constdata)
      *:securefs_m4.o(.text)
      *:securefs_m4.o(.rodata)
      *:securefs_m4.o(.conststring)
      *:securefs_m4.o(.constdata)
      *tee_master.o(.text)
      *tee_master.o(.rodata)
      *tee_master.o(.conststring)
      *tee_master.o(.constdata)
      *qapi_ed25519.o(.text)
      *qapi_ed25519.o(.rodata)
      *qapi_ed25519.o(.conststring)
      *qapi_ed25519.o(.constdata)
      *:qapi_wlan_bridge.o(.text)
      *:qRTUS.o(.text)
      *:diag_cli.o(.text)
      *:diag_cli.o(.conststring)
      *:diag_msg.o(.text)
      *:GLS.o(.text)
      *:AIOS.o(.text) 
      *:HTS.o(.text)
      *:diag.o(.text)
      *:usb_isr.o(.text)
      *:tf_interface.o(.text)
      *:IPSP.o(.text)
      *:SCPS.o(.text)
      *:tf_list.o(.text)
      *:qANCS.o(.text)
      *:tf_sample.o(.text)
      *:BLS.o(.text)
      *:diag_transport_glink.o(.text)
      *:diag_transport_usb.o(.text)
      *:qHTS.o(.text)
      *:qCTS.o(.text)
      *:PASS.o(.text)
      *:LLS.o(.text)
      *:qBMS.o(.text)
      *:CGMS.o(.text)
      *:qBCS.o(.text)
      *:BMS.o(.text)
      *:ANCS.o(.text)
      *:tf_uart.o(.text)
      *:qBLS.o(.text)
      *:qSCPS.o(.text)
      *:qCPS.o(.text)
      *:qANS.o(.text)
      *:IAS.o(.text)
      *:qGLS.o(.text)
      *:qHRS.o(.text)
      *:qAIOS.o(.text)
      *:qRSCS.o(.text)
      *:BCS.o(.text)
      *:qNDCS.o(.text)
      *:DIS.o(.text)
      *:qCGMS.o(.text)
      *:GAPS.o(.text)
      *:TPS.o(.text)
      *:CPS.o(.text)
      *:qLLS.o(.text)
      *:diag_transport_uart.o(.text)
      *:RTUS.o(.text)
      *:HRS.o(.text)
      *:qBAS.o(.text)
      *:BAS.o(.text)
      *:qHIDS.o(.text)
      *:diagpkt.o(.text)
      *:qLNS.o(.text)
      *:diagtools.o(.text)
      *:diagtools.o(.constdata)
      *:NDCS.o(.text)
      *:nsport.o(.text)
      *:boot_sahara_impl.o(.text)
      *:boot_sahara_impl.o(.constdata)
      *:qIAS.o(.text)
      *:diag_transport.o(.text)
      *:qapi_diag.o(.text)
      *:qDIS.o(.text)
      *:diag_buffer_read.o(.text)
      *:diag_binary_log.o(.text)
      *:ANS.o(.text)
      *:qGAPS.o(.text)
      *:qIPSP.o(.text)
      *:qPASS.o(.text)
      *:HIDS.o(.text)
      *:LNS.o(.text)
      *:diag_test.o(.text)
      *:qTPS.o(.text)
      *:tf_main.o(.text)
      *:usb_client_loopback_diag.o(.text)
      *:tf_glink.o(.text)
      *:CTS.o(.text)
      *:RSCS.o(.text)
      *:tf_handle.o(.text)      
      *\rfs.lib:*(.text)
      *:srp_hw.o(.text)
      *zigbee.lib:*(.text)
      *zigbee.lib:*(.rodata)
      *\thread.lib:*(i.*)
      *qapi_thread.lib:*(.text)
      *(XIP_BSP_RO_SECTION)
   } >XIP_RO_MEMORY : XIP_SEGMENT


 /* RAM_FOM_APPS_CODE_SEGMENT */
   RAM_FOM_APPS_RO_REGION :
   {
      *(.conststring)
      *(.constdata)
      *(.rodata)
      *(RAM_FOM_APPS_RO_SECTION)
   } >RAM_FOM_APPS_RO_MEMORY : RAM_FOM_APPS_CODE_SEGMENT

   /* RAM_FOM_APPS_DATA_SEGMENT */
   RAM_FOM_APPS_RW_REGION :
   {
      *:usb_mem_util.o(.data)
      *:qusb_clock.o(.data)
      *:usb_os_util.o(.data)
      *:usb_al_hlos_bulk.o(.data)
      *:usb_wrapper_api.o(.data)
      *pal.o(.data)
      *qcli.o(.data)
      *qcli_util.o(.data)
      *hmi_demo.o(.data)
      *(RAM_FOM_APPS_RW_SECTION)
   } >RAM_FOM_APPS_DATA_MEMORY : RAM_FOM_APPS_DATA_SEGMENT

   RAM_FOM_APPS_ZI_REGION :
   {
      *:usb_mem_util.o(.bss)
      *:usb_wrapper_api.o(.bss)
      *:usb_os_util.o(.bss)
      *:usb_al_hlos_bulk.o(.bss)
      *:qusb_clock.o(.bss)
      *pal.o(.bss)
      *qcli.o(.bss)
      *qcli_util.o(.bss)
      *hmi_demo.o(.bss)
      *(COMMON)
      *(RAM_FOM_APPS_ZI_SECTION)
   } >RAM_FOM_APPS_DATA_MEMORY : RAM_FOM_APPS_DATA_SEGMENT

 /* RAM_FOM_BSP_CODE_SEGMENT */
   RAM_FOM_BSP_RO_REGION :
   {
      *(.text)

      /* Needed for C++ static constructors. See __initialize_cpp__aeabi_ */
      PROVIDE_HIDDEN(__init_array_begin = .);
      *(.init_array*)
      PROVIDE_HIDDEN(__init_array_end = .);

      *(RAM_FOM_BSP_RO_SECTION)
   } >RAM_FOM_BSP_RO_MEMORY : RAM_FOM_BSP_CODE_SEGMENT

   RAM_FOM_BSP_ZI_REGION :
   {
      *(.bss)

      *(RAM_FOM_BSP_ZI_SECTION)
   } >RAM_FOM_BSP_DATA_MEMORY : RAM_FOM_BSP_DATA_SEGMENT

   RAM_FOM_BSP_RW_REGION :
   {
      *(.data)
      *(RAM_FOM_BSP_RW_SECTION)
   } >RAM_FOM_BSP_DATA_MEMORY : RAM_FOM_BSP_DATA_SEGMENT

   .ARM.exidx :
   {
      *(.ARM.exidx*)
   } >XIP_RO_MEMORY : XIP_SEGMENT
}

/* Export symbols. */
Image$$RAM_SOM_BSP_ZI_REGION$$ZI$$Base        = ORIGIN(RAM_SOM_BSP_ZI_MEMORY);
Image$$RAM_SOM_BSP_ZI_REGION$$ZI$$Length      = LENGTH(RAM_SOM_BSP_ZI_MEMORY);


Image$$RAM_FOM_BSP_ZI_REGION$$ZI$$Base            = ORIGIN(RAM_FOM_BSP_DATA_MEMORY);
Image$$RAM_FOM_BSP_ZI_REGION$$ZI$$Length          = LENGTH(RAM_FOM_BSP_DATA_MEMORY);

Image$$SBL_LOG_META_INFO_RW_SECTION$$Base     = ORIGIN(SBL_LOG_META_INFO_RW_MEMORY);
Image$$SBL_LOG_BUF_RW_SECTION$$Base           = ORIGIN(SBL_LOG_BUF_RW_MEMORY);

Image$$RAM_SBL_RO_REGION$$RW$$Base            = ORIGIN(RAM_SBL_RO_MEMORY);
Image$$RAM_SBL_RO_REGION$$RW$$Limit           = LENGTH(RAM_SBL_RO_MEMORY);

Image$$RAM_SOM_BSP_HEAP_REGION$$ZI$$Base         = ORIGIN(RAM_SOM_BSP_HEAP_ZI_MEMORY);
Image$$RAM_SOM_BSP_HEAP_REGION$$ZI$$Length       = LENGTH(RAM_SOM_BSP_HEAP_ZI_MEMORY);

Image$$RAM_SOM_BSP_HEAP_ZI_REGION$$ZI$$Base      = ORIGIN(RAM_SOM_BSP_HEAP_ZI_MEMORY);
Image$$RAM_SOM_BSP_HEAP_ZI_REGION$$ZI$$Length    = LENGTH(RAM_SOM_BSP_HEAP_ZI_MEMORY);


Image$$RAM_SOM_APPS_ZI_REGION$$ZI$$Base          = ORIGIN(RAM_SOM_APPS_ZI_MEMORY);
Image$$RAM_SOM_APPS_ZI_REGION$$ZI$$Length        = LENGTH(RAM_SOM_APPS_ZI_MEMORY);

Image$$RAM_FOM_APPS_ZI_REGION$$ZI$$Base          = ORIGIN(RAM_FOM_APPS_ZI_MEMORY);
Image$$RAM_FOM_APPS_ZI_REGION$$ZI$$Length        = LENGTH(RAM_FOM_APPS_ZI_MEMORY);

Image$$RAM_AON_MOM_ZI_REGION$$ZI$$Base           = ORIGIN(RAM_AON_MOM_DATA_MEMORY);
Image$$RAM_AON_MOM_ZI_REGION$$ZI$$Length         = LENGTH(RAM_AON_MOM_DATA_MEMORY);

Load$$RAM_SOM_BSP_ZI_REGION$$ZI$$Base            = ORIGIN(RAM_SOM_BSP_ZI_MEMORY);
Load$$RAM_SOM_BSP_HEAP_ZI_REGION$$ZI$$Base       = ORIGIN(RAM_SOM_BSP_HEAP_ZI_MEMORY);
Load$$RAM_SOM_APPS_ZI_REGION$$ZI$$Base           = ORIGIN(RAM_SOM_APPS_ZI_MEMORY);
Load$$RAM_FOM_BSP_ZI_REGION$$ZI$$Base            = ORIGIN(RAM_FOM_BSP_DATA_MEMORY);
Load$$RAM_FOM_APPS_ZI_REGION$$ZI$$Base           = ORIGIN(RAM_FOM_APPS_DATA_MEMORY);

Image$$RAM_SOM_BSP_DIAG_BUF_REGION$$ZI$$Base     = ORIGIN(RAM_SOM_BSP_DIAG_BUF_MEMORY);
Image$$RAM_SOM_BSP_DIAG_BUF_REGION$$ZI$$Length   = LENGTH(RAM_SOM_BSP_DIAG_BUF_MEMORY);

Image$$RAM_SOM_BSP_STACK_REGION$$ZI$$Base        = ORIGIN(RAM_SOM_BSP_STACK_ZI_MEMORY);
Image$$RAM_SOM_BSP_STACK_REGION$$ZI$$Limit       = ORIGIN(RAM_SOM_BSP_STACK_ZI_MEMORY) + LENGTH(RAM_SOM_BSP_STACK_ZI_MEMORY);;
Image$$RAM_FOM_BSP_HEAP_REGION$$ZI$$Base         = ORIGIN(RAM_FOM_BSP_HEAP_MEMORY);
Image$$RAM_FOM_BSP_STACK_REGION$$ZI$$Base        = ORIGIN(RAM_FOM_BSP_STACK_ZI_MEMORY);
Image$$RAM_FOM_BSP_STACK_REGION$$ZI$$Limit       = ORIGIN(RAM_FOM_BSP_STACK_ZI_MEMORY) + LENGTH(RAM_FOM_BSP_STACK_ZI_MEMORY);