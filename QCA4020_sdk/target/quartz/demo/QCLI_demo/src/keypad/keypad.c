/*
 * Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
 * 2015-2016 Qualcomm Atheros, Inc.
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

#include <stdio.h>
#include <qurt_signal.h>
#include <qapi/qurt_thread.h>
#include <qapi/qapi_status.h>
#include <qapi/qapi_timer.h>
#include <stdint.h>
#include <qcli.h>
#include <qcli_api.h>
#include <qapi_kpd.h>
#include <qurt_timer.h>

#include "keypad_demo.h"

#define DEFAULT_KEYPAD_MATRIX_ROW_MASK	  0xE7
#define DEFAULT_KEYPAD_MATRIX_COL_MASK	  0xEF

#define	KEYPAD_USR_KEY_PRESS_SIG_MASK     0x01
#define	KEYBOARD_USR_KEY_PRESS_SIG_MASK   0x02

#define	USR_KEY_PRESS_SIG_MASK   (KEYPAD_USR_KEY_PRESS_SIG_MASK | KEYBOARD_USR_KEY_PRESS_SIG_MASK)

extern QCLI_Group_Handle_t qcli_keybrd_group;              /* Handle for our QCLI Command Group. */

qurt_signal_t   key_press_signal;

qapi_KPD_Handle_t   keybrd_handle;

qapi_KPD_Config_t   keybrd_Config = 
{
   {
      0xE7,
   /**< Bitmap of rows that need to be enabled
    *  (e.g., 0x03 = Rows 0 and 1 are enabled).
    */

      0xEF,
   /**< Bitmap of columns that need to be enabled
    * (e.g., 0x03 = Columns 0 and 1 are enabled ).
    */

       0,
   /**< GPIO set to be used. The platform supports mutiple sets of GPIOs
     *  to interface with an external keypad. Consult the hardware schematic
     *  configuration database for the GPIO set to be used.
     *  0 = set 0 is selected; 1 = set 1 is selected.
     */
   },
   /**< Keypad matrix configuration. */

   {
       20,
   /**< First press debounce time in ms (default range: 0 to 60).
    *   The typical value for first press debounce is 20 ms.
    */

       1000,
   /**< Long press debounce time in ms (default range: 0 to 3937).
    *   The long press timer starts after the initial debounce has completed.
    *   The typical value for long press debounce is 1000 ms.
    */

        500,
   /**< Repeated press debounce time in ms (default range: 0 to 3937).
    *   Repeated press debounce starts after long press debounce has completed.
    *   The typical value for repeated press debounce is 500 ms.
    */
   },
   /**< Keypad debounce configuration. */

   {
		1,   /**< First press interrupt enable. */
		1,    /**< Long press interrupt enable. */
		1,  /**< Repeated press interrupt enable. */
		1,   /**< Key release interrupt enable. @newpagetable */
   },
   /**< Keypad interrupt configuration. */
};

qapi_KPD_KeyPress_t  keypress;
qapi_KPD_Interrupt_Status_t  key_status;

void keyboard_cb(const qapi_KPD_Interrupt_Status_t *pKpd_IntStatus,
                 const qapi_KPD_KeyPress_t *pKpd_KeyPressState, void *callback_Ctxt)
{
	keypress.keyMatrix_Hi = pKpd_KeyPressState->keyMatrix_Hi;
    keypress.keyMatrix_Lo = pKpd_KeyPressState->keyMatrix_Lo;

	key_status.firstPress = pKpd_IntStatus->firstPress ;   /**< First press interrupt status. */
    key_status.longPress = pKpd_IntStatus->longPress;      /**< Long press interrupt status. */
    key_status.repeatPress = pKpd_IntStatus->repeatPress;  /**< Repeated press interrupt status. */
    key_status.keyRelease = pKpd_IntStatus->keyRelease;    /**< Key release interrupt status. */

	qurt_signal_set(&key_press_signal, KEYPAD_USR_KEY_PRESS_SIG_MASK);	
}

int32_t keypad_test_quit( uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List )
{
	qurt_signal_set(&key_press_signal, KEYBOARD_USR_KEY_PRESS_SIG_MASK);	
	return 0;
}

int32_t keyboard_driver_qapi_test( uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List )
{	
    qapi_Status_t status;
	uint32_t	val32, i, row, col, signals;

    keybrd_Config.kpdMatrix.row_Mask = DEFAULT_KEYPAD_MATRIX_ROW_MASK;
	keybrd_Config.kpdMatrix.col_Mask = DEFAULT_KEYPAD_MATRIX_COL_MASK;
	
	if (Parameter_Count >= 1)
	{
		if (Parameter_List->Integer_Value != 0)
			keybrd_Config.kpdMatrix.row_Mask = Parameter_List->Integer_Value;
		
		Parameter_List++;
	}

	if (Parameter_Count >= 2)
	{
		if (Parameter_List->Integer_Value != 0)
			keybrd_Config.kpdMatrix.col_Mask = Parameter_List->Integer_Value;
		Parameter_List++;
	}
    QCLI_Printf(qcli_keybrd_group, "Keypad Matrix Mask=[row:%02X col:%02X]\n", 
								keybrd_Config.kpdMatrix.row_Mask, keybrd_Config.kpdMatrix.col_Mask);	

    qurt_signal_init(&key_press_signal);
	
	status = qapi_KPD_Init(&keybrd_handle);
	if (status != QAPI_OK)
		return  -1;
	
	status = qapi_KPD_Set_Config(keybrd_handle, &keybrd_Config, keyboard_cb, NULL);
	if (status != QAPI_OK)
	{
		qapi_KPD_DeInit(keybrd_handle);
		return  -1;		
	}

	status = qapi_KPD_Enable(keybrd_handle, TRUE);

    do
    {
        signals = qurt_signal_wait(&key_press_signal, USR_KEY_PRESS_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);
		
		if (signals & KEYBOARD_USR_KEY_PRESS_SIG_MASK)
			break;
		
		if (key_status.keyRelease == 0)
		{			
			if (keypress.keyMatrix_Hi != 0 || keypress.keyMatrix_Lo != 0)
			{
				if (keypress.keyMatrix_Hi != 0)
				{
					val32 = keypress.keyMatrix_Hi;
					
					for (i=0; i < 32; i++)
					{
						if (val32 & 1)
						{
							row = i / 8;
							col = i % 8;
				            QCLI_Printf(qcli_keybrd_group, "[row:%d col:%d]\n", row + 4, col);	
						}
						val32 = val32 >> 1;
					}						
				}
				
				if (keypress.keyMatrix_Lo != 0)
				{
					val32 = keypress.keyMatrix_Lo;
					
					for (i=0; i < 32; i++)
					{
						if (val32 & 1)
						{
							row = i / 8;
							col = i % 8;
				            QCLI_Printf(qcli_keybrd_group, "[row:%d col:%d]\n", row, col);	
						}
						val32 = val32 >> 1;
					}					
				}				
			}
		}
    } while (1);
	
	status = qapi_KPD_Enable(keybrd_handle, FALSE);
	status = qapi_KPD_DeInit(keybrd_handle);

	qurt_signal_destroy(&key_press_signal);
	
	return 0;
}
