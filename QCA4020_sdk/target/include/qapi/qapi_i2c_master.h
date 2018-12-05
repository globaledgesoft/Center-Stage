#ifndef __QAPI_I2C_MASTER_H__
#define __QAPI_I2C_MASTER_H__

/*
 * Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
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


/*==================================================================================

                             EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file. Notice that
changes are listed in reverse chronological order.

$Header: //components/rel/core.ioe/1.0/v2/rom/release/api/buses/i2cm/qapi_i2c_master.h#4 $

when       who     what, where, why
--------   ---     -----------------------------------------------------------------
2/10/17    leo     (Tech Comm) Edited/added Doxygen comments and markup.
12/03/15   leo     (Tech Comm) Edited/added Doxygen comments and markup.
10/15/15   unr     Initial version
==================================================================================*/

/**
 * 
 * @file qapi_i2c_master.h
 *
 * @addtogroup qapi_i2c_master
 * @{
 *
 * @brief Inter-Integrated Circuit (I2C)
 *
 * @details I2C is a 2-wire bus used to connect low speed peripherals to a
 *          processor or a microcontroller. Common I2C peripherals include
 *          touch screen controllers, accelerometers, gyros, and ambient light
 *          and temperature sensors.
 *
 *          The 2-wire bus comprises a data line, a clock line, and basic START,
 *          STOP, and acknowledge signals to drive transfers on the bus. An I2C
 *          peripheral is also referred to as an I2C slave. The processor or
 *          microcontroller implements the I2C master as defined in the I2C
 *          specification. This documentation provides the software interface to
 *          access the I2C master implementation.

 @code {.c}
   
 // 
 // The code sample below demonstrates the use of this interface.
 //

 void sample (void)
 {
   void *client_handle = NULL;
   uint32_t transferred1, transferred2;
   uint8_t buffer[4] = { 1, 2, 3, 4 };

   qapi_Status_t res = QAPI_OK;
   qapi_I2CM_Config_t config;
   qapi_I2CM_Descriptor_t desc[2];

   // Obtain a client specific connection handle to the i2c bus instance 1
   res = qapi_I2CM_Open (QAPI_I2CM_INSTANCE_001_E, &client_handle);

   // Configure the bus speed and slave address
   config.bus_Frequency_KHz = 400; 
   config.slave_Address     = 0x36;
   config.SMBUS_Mode        = FALSE;

   // <S>  - START bit
   // <P>  - STOP  bit
   // <Sr> - Repeat Start bit
   // <A>  - Acknowledge bit
   // <N>  - Not-Acknowledge bit
   // <R>  - Read Transfer
   // <W>  - Write Transfer

   // Single write transfer of N bytes
   // <S><slave_address><W><A><data1><A><data2><A>...<dataN><A><P>
   desc[0].buffer      = buffer;
   desc[0].length      = 4;
   desc[0].transferred = &transferred1;
   desc[0].flags       = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_WRITE | QAPI_I2C_FLAG_STOP;
   res = qapi_I2CM_Transfer (client_handle, &config, &desc[0], 1, client_callback, NULL);

   // Single read transfer of N bytes
   // <S><slave_address><R><A><data1><A><data2><A>...<dataN><N><P>
   desc[0].buffer      = buffer;
   desc[0].length      = 4;
   desc[0].transferred = &transferred1;
   desc[0].flags       = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_READ  | QAPI_I2C_FLAG_STOP;
   res = qapi_I2CM_Transfer (client_handle, &config, &desc[0], 1, client_callback, NULL);

   // Read N bytes from a register 0x01 on a sensor device
   // <S><slave_address><W><A><0x01><A><S><slave_address><R><A><data1><A><data2><A>...<dataN><N><P>
   uint8_t reg = 0x01;
   desc[0].buffer      = &reg;
   desc[0].length      = 1;
   desc[0].transferred = &transferred1;
   desc[0].flags       = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_WRITE;

   desc[1].buffer      = buffer;
   desc[1].length      = 4;
   desc[1].transferred = &transferred2;
   desc[1].flags       = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_READ  | QAPI_I2C_FLAG_STOP;
   res = qapi_I2CM_Transfer (client_handle, &config, &desc[0], 2, client_callback, NULL);

   // Read N bytes from eeprom address 0x0102
   // <S><slave_address><W><A><0x01><A><0x02><A><S><slave_address><R><A><data1><A><data2><A>...<dataN><N><P>
   uint8_t reg[2] = { 0x01, 0x02 };
   desc[0].buffer      = reg;
   desc[0].length      = 2;
   desc[0].transferred = &transferred1;
   desc[0].flags       = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_WRITE;

   desc[1].buffer      = buffer;
   desc[1].length      = 4;
   desc[1].transferred = &transferred2;
   desc[1].flags       = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_READ  | QAPI_I2C_FLAG_STOP;
   res = qapi_I2CM_Transfer (client_handle, &config, &desc[0], 2, client_callback, NULL);

   // Close the connection handle to the i2c bus instance
   res = qapi_I2CM_Close (client_handle);
 }

 void client_callback (uint32_t status, void *ctxt)
 {
   // Transfer completed
 }

 @endcode

 @}
*/


/*-------------------------------------------------------------------------
 * Include Files
 * ----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * ----------------------------------------------------------------------*/

/** @addtogroup qapi_i2c_master
@{ */

/** @name Preprocessor Definitions and Constants
@{ */

#define QAPI_I2CM_ERR_INVALID_PARAMETER             __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 1)    /**< Parameter passed to the API is invalid. */
#define QAPI_I2CM_ERR_UNSUPPORTED_CORE_INSTANCE     __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 2)    /**< Core instance passed in qapi_I2CM_Open() is not supported. */
#define QAPI_I2CM_ERR_API_INVALID_EXECUTION_LEVEL   __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 3)    /**< API was called from an interrupt execution level. */
#define QAPI_I2CM_ERR_HANDLE_ALLOCATION             __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 4)    /**< Client handle allocation failed. */
#define QAPI_I2CM_ERR_HW_INFO_ALLOCATION            __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 5)    /**< Core instance resource allocation failed. */
#define QAPI_I2CM_ERR_BUS_NOT_IDLE                  __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 6)    /**< I2C bus is not in Idle (default high) state. */
#define QAPI_I2CM_ERR_TRANSFER_TIMEOUT              __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 7)    /**< Transfer timed out. */
#define QAPI_I2CM_ERR_INPUT_FIFO_OVER_RUN           __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 8)    /**< Input FIFO is full. */
#define QAPI_I2CM_ERR_OUTPUT_FIFO_UNDER_RUN         __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 9)    /**< Output FIFO is empty. */
#define QAPI_I2CM_ERR_INPUT_FIFO_UNDER_RUN          __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 10)   /**< Input FIFO is empty. */
#define QAPI_I2CM_ERR_OUTPUT_FIFO_OVER_RUN          __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 11)   /**< Output FIFO is full. */
#define QAPI_I2CM_ERR_INVALID_BUS_HIGH_TIME_VALUE   __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 12)   /**< Invalid SCL high time configuration. */
#define QAPI_I2CM_ERR_QSTATE_INVALID                __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 13)   /**< The hardware state is invalid. */
#define QAPI_I2CM_ERR_START_STOP_UNEXPECTED         __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 14)   /**< An unexpected START or STOP condition occurred on the bus. */
#define QAPI_I2CM_ERR_DATA_NACK                     __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 15)   /**< Data was NACKed by the slave. */
#define QAPI_I2CM_ERR_ADDR_NACK                     __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 16)   /**< Slave address was NACKed. */
#define QAPI_I2CM_ERR_ARBITRATION_LOST              __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 17)   /**< The controller lost arbitration. */
#define QAPI_I2CM_ERR_INVALID_WRITE                 __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 18)   /**< Invalid data was written to the Output FIFO. */
#define QAPI_I2CM_ERR_INVALID_TAG                   __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 19)   /**< Invalid tag was written to the Output FIFO. */
#define QAPI_I2CM_ERR_PLATFORM_INIT_FAIL            __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 20)   /**< Platform initialize failed. */
#define QAPI_I2CM_ERR_PLATFORM_DEINIT_FAIL          __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 21)   /**< Platform de-initialize failed. */
#define QAPI_I2CM_ERR_PLATFORM_REG_INT_FAIL         __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 22)   /**< Interrupt could not be registered. */
#define QAPI_I2CM_ERR_PLATFORM_DE_REG_INT_FAIL      __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 23)   /**< Interrupt could not be unregistered. */
#define QAPI_I2CM_ERR_PLATFORM_CLOCK_ENABLE_FAIL    __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 24)   /**< Clock enable failed. */
#define QAPI_I2CM_ERR_PLATFORM_GPIO_ENABLE_FAIL     __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 25)   /**< Clock disable failed. */
#define QAPI_I2CM_ERR_PLATFORM_CLOCK_DISABLE_FAIL   __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 26)   /**< GPIO enable failed. */
#define QAPI_I2CM_ERR_PLATFORM_GPIO_DISABLE_FAIL    __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 27)   /**< GPIO disable failed. */
#define QAPI_I2CM_TRANSFER_CANCELED                 __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 28)   /**< Transfer was cancelled. */
#define QAPI_I2CM_TRANSFER_FORCE_TERMINATED         __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 29)   /**< Transfer was forecefully terminated. */
#define QAPI_I2CM_TRANSFER_COMPLETED                __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 30)   /**< Transfer completed. */
#define QAPI_I2CM_TRANSFER_INVALID                  __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 31)   /**< Transfer is invalid and was never queued to hadware. */
#define QAPI_I2CM_ERROR_HANDLE_ALREADY_IN_QUEUE     __QAPI_ERROR(QAPI_MOD_BSP_I2C_MASTER, 32)   /**< The client already has one pending transfer in the queue. */

#define QAPI_I2C_FLAG_START         0x00000001  /**< Specifies that the transfer begins with a START bit - S. */
#define QAPI_I2C_FLAG_STOP          0x00000002  /**< Specifies that the transfer ends with a STOP bit - P. */
#define QAPI_I2C_FLAG_WRITE         0x00000004  /**< Must be set to indicate a WRITE transfer. */
#define QAPI_I2C_FLAG_READ          0x00000008  /**< Must be set to indicate a READ transfer. */

#define QAPI_I2C_TRANSFER_MASK      (QAPI_I2C_FLAG_WRITE | QAPI_I2C_FLAG_READ) /**< Transfer types. */
#define QAPI_VALID_FLAGS(x) \
    (((x & QAPI_I2C_TRANSFER_MASK) == QAPI_I2C_FLAG_READ) || ((x & QAPI_I2C_TRANSFER_MASK) == QAPI_I2C_FLAG_WRITE)) /**< Verifies the validity of flags. */

/** @} */ /* end_namegroup */

/** @} */ /* end_addtogroup qapi_i2c_master */

/** @addtogroup qapi_i2c_master
@{ */

/*-------------------------------------------------------------------------
 * Type Declarations
 * ----------------------------------------------------------------------*/

/**
  Instance of the I2C core that the client wants to use. This instance is
  passed in qapi_I2CM_Open().
*/
typedef enum
{
    QAPI_I2CM_INSTANCE_001_E = 1, /**< I2C core 01. */
    QAPI_I2CM_INSTANCE_002_E,     /**< I2C core 02. */
    QAPI_I2CM_INSTANCE_003_E,     /**< I2C core 03. */
    QAPI_I2CM_INSTANCE_004_E,     /**< I2C core 04. */
    QAPI_I2CM_INSTANCE_005_E,     /**< I2C core 05. */
    QAPI_I2CM_INSTANCE_006_E,     /**< I2C core 06. */
    QAPI_I2CM_INSTANCE_007_E,     /**< I2C core 07. */
    QAPI_I2CM_INSTANCE_008_E,     /**< I2C core 08. */
    QAPI_I2CM_INSTANCE_009_E,     /**< I2C core 09. */
    QAPI_I2CM_INSTANCE_010_E,     /**< I2C core 10. */
    QAPI_I2CM_INSTANCE_011_E,     /**< I2C core 11. */
    QAPI_I2CM_INSTANCE_012_E,     /**< I2C core 12. */

    QAPI_I2CM_INSTANCE_MAX_E,

} qapi_I2CM_Instance_t;

/** @} */ /* end_addtogroup qapi_i2c_master */

/** @addtogroup qapi_i2c_master
@{ */

/**
  I2C client configuration parameters that the client uses to communicate
  to an I2C slave.
*/
typedef struct
{
    uint32_t  bus_Frequency_KHz;            /**< I2C bus speed in kHz. */
    uint32_t  slave_Address;                /**< 7-bit I2C slave address. */
    qbool_t   SMBUS_Mode;                   /**< SMBUS mode transfers. Set to TRUE for SMBUS mode. */
    uint32_t  slave_Max_Clock_Stretch_Us;   /**< Maximum slave clock stretch in us that a slave might perform. */
    uint32_t  core_Configuration1;          /**< Core Specific Configuration. Recommended 0. */
    uint32_t  core_Configuration2;          /**< Core Specific Configuration. Recommended 0. */

} qapi_I2CM_Config_t;

/**
  I2C transfer descriptor.
*/
typedef struct
{
    uint8_t   *buffer;        /**< Buffer for the data transfer. */
    uint32_t  length;         /**< Length of the data to be transferred in bytes. */
    uint32_t  transferred;    /**< Number of bytes actually transferred. */
    uint32_t  flags;          /**< I2C flags for for the transfer. */

} qapi_I2CM_Descriptor_t;

/**
  Transfer callback.

  Declares the type of callback function that needs to be defined by the client.
  The callback is called when the data is completely transferred on the bus or
  the transfer ends due to an error or cancellation.

  Clients pass the callback function pointer and the callback context to the
  driver in the qapi_I2CM_Transfer() API.

  @note1hang
  The callback is called in the interrupt context. Calling any of the APIs
  defined here in the callback will result in the error
  QAPI_I2CM_ERR_API_INVALID_EXECUTION_LEVEL. Processing in the callback
  function must be kept to a minimum to avoid latencies in the system.

  @param[out] status        Completion status of the transfer. A call to
                            qapi_I2CM_Get_QStatus_Code() will convert this
                            status to QAPI status codes.

  @param[out] CB_Parameter  CP_Parameter context that was passed in the call
                            to qapi_I2CM_Transfer().

*/
typedef
void
(*qapi_I2CM_Transfer_CB_t)
(
    const uint32_t status,
    void *CB_Parameter
);

/** @} */ /* end_addtogroup qapi_i2c_master */

/** @addtogroup qapi_i2c_master
@{ */

/*-------------------------------------------------------------------------
 * Function Declarations and Documentation
 * ----------------------------------------------------------------------*/

/** Called by the client code to initialize the respective I2C
    instance. On success, i2c_Handle points to the handle for the I2C instance.
    The API allocates resources for use by the client handle and the I2C
    instance. These resources are release in the qapi_I2CM_Close() call.
    The API also enables power to the I2C HW instance. To disable the power to
    the instance, a corresponding call to ::qapi_I2CM_Close must be made.

    @param[in]  instance    I2C instance that the client intends to
                            initialize; see #qapi_I2CM_Instance_t for 
                            details.
    @param[out] i2c_Handle  Pointer location to be filled by the
                            driver with a handle to the instance.

    @return
    QAPI_OK -- Function was successful. \n
    QAPI_I2CM_ERR_INVALID_PARAMETER -- Invalid parameter. \n
    QAPI_I2CM_ERR_API_INVALID_EXECUTION_LEVEL -- Invalid execution level. \n
    QAPI_I2CM_ERR_UNSUPPORTED_CORE_INSTANCE --  Unsupported core instance. \n
    QAPI_I2CM_ERR_HANDLE_ALLOCATION -- Handle allocation error. \n
    QAPI_I2CM_ERR_HW_INFO_ALLOCATION -- Hardware information allocation error. \n
    QAPI_I2CM_ERR_PLATFORM_INIT_FAIL -- Platform initialization failure. \n
    QAPI_I2CM_ERR_PLATFORM_REG_INT_FAIL -- Platform registration internal failure. \n
    QAPI_I2CM_ERR_PLATFORM_CLOCK_ENABLE_FAIL -- Platform clock enable failure. \n
    QAPI_I2CM_ERR_PLATFORM_GPIO_ENABLE_FAIL -- Platform GPIO enable failure.
*/
qapi_Status_t
qapi_I2CM_Open
(
    qapi_I2CM_Instance_t instance,
    void **i2c_Handle
);

/** 
    Deinitializes the I2C instance and releases any resources allocated by the
    qapi_I2CM_Open() API.

    @param[in] i2c_Handle  Handle to the I2C instance.

    @return
    QAPI_OK -- Function was successful. \n
    QAPI_I2CM_ERR_INVALID_PARAMETER -- Invalid parameter. \n
    QAPI_I2CM_ERR_API_INVALID_EXECUTION_LEVEL -- Invalid execution level. \n
    QAPI_I2CM_ERR_PLATFORM_DEINIT_FAIL -- Platform de-initialization failure. \n
    QAPI_I2CM_ERR_PLATFORM_DE_REG_INT_FAIL -- Platform de-registration internal failure. \n
    QAPI_I2CM_ERR_PLATFORM_CLOCK_DISABLE_FAIL -- Platform clock disable failure. \n
    QAPI_I2CM_ERR_PLATFORM_GPIO_DISABLE_FAIL -- Platform GPIO disable failure.
*/
qapi_Status_t
qapi_I2CM_Close
(
    void *i2c_Handle
);

/** 
    Performs an I2C transfer. In case a transfer is already in progress by
    another client, this call queues the transfer. If the transfer returns a
    failure, the transfer has not been queued and no callback will occur.
    If the transfer returns QAPI_OK, the transfer has been queued and a
    further status of the transfer can only be obtained when the callback is
    called.

    @note1hang
    After a client calls this API, it must wait for the completion callback to
    occur before it can call the API again. If the client wishes to queue
    mutliple transfers, it must use an array of descriptors of type
    qapi_I2CM_Descriptor_t instead of calling the API multiple times.

    @param[in]  i2c_Handle       Handle to the I2C instance.
    @param[in]  config           Slave configuration. See #qapi_I2CM_Config_t
                                 for details.
    @param[in]  desc             I2C transfer descriptor. See
                                 #qapi_I2CM_Descriptor_t for details. This
                                 can be an array of descriptors.
    @param[in]  num_Descriptors  Number of descriptors in the descriptor array.
    @param[in]  CB_Function      The callback function that is called at the
                                 completion of the transfer occurs in the
                                 interrupt context. The call must do
                                 minimal processing and must not call any API
                                 defined here.
    @param[in]  CB_Parameter     The context that the client passes here is
                                 returned as is in the callback function.

    @return
    QAPI_OK -- Function was successful. \n
    QAPI_I2CM_ERR_INVALID_PARAMETER -- Invalid parameter. \n
    QAPI_I2CM_ERR_API_INVALID_EXECUTION_LEVEL -- Invalid execution level. \n
    QAPI_I2CM_ERR_TRANSFER_TIMEOUT -- Transfer timed out. \n
    QAPI_I2CM_ERR_QSTATE_INVALID -- QState is invalid.
    QAPI_I2CM_ERROR_HANDLE_ALREADY_IN_QUEUE -- Client IO is pending.
*/
qapi_Status_t
qapi_I2CM_Transfer
(
    void *i2c_Handle,
    qapi_I2CM_Config_t *config,
    qapi_I2CM_Descriptor_t *desc,
    uint16_t num_Descriptors,
    qapi_I2CM_Transfer_CB_t CB_Function,
    void *CB_Parameter
);

/** 
    Returns QAPI status codes corresponding to the controller codes returned in
    ::qapi_I2CM_Transfer_CB_t.

    @param[in] status  Controller status code.

    @return
    QAPI_OK -- Function was successful. \n
    QAPI_I2CM_ERR_BUS_NOT_IDLE -- Bus was not idle. \n
    QAPI_I2CM_ERR_INPUT_FIFO_OVER_RUN -- Input FIFO overrun. \n
    QAPI_I2CM_ERR_OUTPUT_FIFO_UNDER_RUN -- Output FIFO underrun. \n
    QAPI_I2CM_ERR_INPUT_FIFO_UNDER_RUN -- Input FIFO underrun. \n
    QAPI_I2CM_ERR_OUTPUT_FIFO_OVER_RUN -- Output FIFO overrun. \n
    QAPI_I2CM_ERR_START_STOP_UNEXPECTED -- Unexpected bus START or STOP. \n
    QAPI_I2CM_ERR_DATA_NACK -- Data was NACKed. \n
    QAPI_I2CM_ERR_ADDR_NACK -- Address was NACKed. \n
    QAPI_I2CM_ERR_ARBITRATION_LOST -- Arbitration was lost. \n
    QAPI_I2CM_ERR_INVALID_WRITE -- Invalid write. \n
    QAPI_I2CM_ERR_INVALID_TAG -- Invalid tag.
*/
qapi_Status_t
qapi_I2CM_Get_QStatus_Code
(
    uint32_t status
);


/** 
    Cancels a transfer. A transfer that has been initiated successfully
    by calling qapi_I2CM_Transfer() may be canceled. Based on the internal state 
    of the transfer, this function will either immediately cancel the transfer or end
    the transfer at a later time.

    If the function returns QAPI_I2CM_TRANSFER_CANCELED,
    the transfer was canceled successfully and the client callback will
    not be called. If the function returns QAPI_I2CM_TRANSFER_FORCE_TERMINATED,
    the client must wait for the callback to occur in order to make sure that the
    transfer has ended. If the function returns QAPI_I2CM_TRANSFER_COMPLETED, the
    transfer has already completed and the callback has already been called.
    If the function returns QAPI_I2CM_TRANSFER_INVALID, the client is trying to cancel a
    transfer that is not queued to the hardware with an earlier call to
    qapi_I2CM_Transfer().

    @param[in] i2c_Handle   Handle to the I2C instance.

    @return
    QAPI_I2CM_ERR_INVALID_PARAMETER -- Invalid parameter. \n
    QAPI_I2CM_ERR_API_INVALID_EXECUTION_LEVEL -- Invalid execution level. \n
    QAPI_I2CM_TRANSFER_INVALID -- Invalid transfer. \n
    QAPI_I2CM_TRANSFER_CANCELED -- Transfer is canceled successfully. \n
    QAPI_I2CM_TRANSFER_COMPLETED -- Transfer completed before canceling. \n
    QAPI_I2CM_TRANSFER_FORCE_TERMINATED -- Transfer aborted on the bus. \n

*/
qapi_Status_t
qapi_I2CM_Cancel_Transfer
(
    void *i2c_Handle
);

/** @} */ /* end_addtogroup qapi_i2c_master */

#endif /* __QAPI_I2C_MASTER_H__ */
