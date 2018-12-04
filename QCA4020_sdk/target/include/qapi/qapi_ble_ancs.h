/*
 * Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
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

/**
 * @file qapi_ble_ancs.h
 *
 * @brief
 * QCA QAPI for Bluetopia Bluetooth Apple Notification Center Service
 * (GATT based) API Type Definitions, Constants, and
 * Prototypes.
 *
 * @details
 * The Apple Notification Center Service (ANCS)programming interface
 * defines the protocols and procedures to be used to
 * implement Apple Notification Center Service capabilities.
 */

#ifndef __QAPI_BLE_ANCS_H__
#define __QAPI_BLE_ANCS_H__

#include "./qapi_ble_btapityp.h"  /* Bluetooth API Type Definitions.          */
#include "./qapi_ble_bttypes.h"   /* Bluetooth Type Definitions/Constants.    */
#include "./qapi_ble_gatt.h"      /* qapi GATT prototypes.                    */
#include "./qapi_ble_ancstypes.h" /* QAPI ANCS prototypes.                    */

/** @addtogroup qapi_ble_services
@{
*/

   /** @name Error Return Codes

    Error codes that are smaller than these (less than -1000) are
    related to the Bluetooth Protocol Stack itself (see
    qapi_ble_errors.h).
    @{ */
#define QAPI_BLE_ANCS_ERROR_INVALID_PARAMETER            (-1000)
/**< Invalid parameter. */
#define QAPI_BLE_ANCS_ERROR_INSUFFICIENT_RESOURCES       (-1001)
/**< Insufficient resources. */
#define QAPI_BLE_ANCS_ERROR_INVALID_REQUEST_DATA         (-1002)
/**< Invalid request data. */
#define QAPI_BLE_ANCS_ERROR_INVALID_ATTRIBUTE_DATA       (-1003)
/**< Invalid attribute data. */
#define QAPI_BLE_ANCS_ERROR_UNKNOWN_ERROR                (-1004)
/**< Unknown error. */
/** @} */ /* end_namegroup */

/**
 * Enumeration of the different category IDs that can be
 * used.
 */
typedef enum
{
   QAPI_BLE_ID_OTHER_E              = QAPI_BLE_ANCS_CATEGORY_ID_OTHER,
   /**< Other. */
   QAPI_BLE_ID_INCOMING_CALL_E      = QAPI_BLE_ANCS_CATEGORY_ID_INCOMING_CALL,
   /**< Incoming call. */
   QAPI_BLE_ID_MISSED_CALL_E        = QAPI_BLE_ANCS_CATEGORY_ID_MISSED_CALL,
   /**< Missed call. */
   QAPI_BLE_ID_VOICEMAIL_E          = QAPI_BLE_ANCS_CATEGORY_ID_VOICEMAIL,
   /**< Voicemail. */
   QAPI_BLE_ID_SOCIAL_E             = QAPI_BLE_ANCS_CATEGORY_ID_SOCIAL,
   /**< Social. */
   QAPI_BLE_ID_SCHEDULE_E           = QAPI_BLE_ANCS_CATEGORY_ID_SCHEDULE,
   /**< Schedule. */
   QAPI_BLE_ID_EMAIL_E              = QAPI_BLE_ANCS_CATEGORY_ID_EMAIL,
   /**< Email. */
   QAPI_BLE_ID_NEWS_E               = QAPI_BLE_ANCS_CATEGORY_ID_NEWS,
   /**< News. */
   QAPI_BLE_ID_HEALTH_AND_FITNESS_E   = QAPI_BLE_ANCS_CATEGORY_ID_HEALTH_AND_FITNESS,
   /**< Health and Fitness. */
   QAPI_BLE_ID_BUSINESS_AND_FINANCE_E = QAPI_BLE_ANCS_CATEGORY_ID_BUSINESS_AND_FINANCE,
   /**< Business and Finance. */
   QAPI_BLE_ID_LOCATION_E           = QAPI_BLE_ANCS_CATEGORY_ID_LOCATION,
   /**< Location. */
   QAPI_BLE_ID_ENTERTAINMENT_E      = QAPI_BLE_ANCS_CATEGORY_ID_ENTERTAINMENT
   /**< Entertainment. */
} qapi_BLE_ANCS_Category_ID_t;

/**
 * Enumerations of the different event IDs that can be used.
 */
typedef enum
{
   QAPI_BLE_ID_NOTIFICATION_ADDED_E    = QAPI_BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED,
   /**< Notification added. */
   QAPI_BLE_ID_NOTIFICATION_MODIFIED_E = QAPI_BLE_ANCS_EVENT_ID_NOTIFICATION_MODIFIED,
   /**< Notification modified. */
   QAPI_BLE_ID_NOTIFICATION_REMOVED_E  = QAPI_BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED
   /**< Notification removed. */
} qapi_BLE_ANCS_Event_ID_t;

/**
 * Enumerations of the different command IDs that can be
 * used.
 */
typedef enum
{
   QAPI_BLE_ID_GET_NOTIFICATION_ATTRIBUTES_E = QAPI_BLE_ANCS_COMMAND_ID_GET_NOTIFICATION_ATTRIBUTES,
   /**< Get notification attributes. */
   QAPI_BLE_ID_GET_APP_ATTRIBUTES_E          = QAPI_BLE_ANCS_COMMAND_ID_GET_APP_ATTRIBUTES
   /**< Get application attributes. */
} qapi_BLE_ANCS_Command_ID_t;

/**
 * Enumerations of the different notification attribute IDs
 * that can be used.
 */
typedef enum
{
   QAPI_BLE_ID_APP_IDENTIFIER_E = QAPI_BLE_ANCS_NOTIFICATION_ATTRIBUTE_ID_APP_IDENTIFIER,
   /**< Application identifier. */
   QAPI_BLE_ID_TITLE_E          = QAPI_BLE_ANCS_NOTIFICATION_ATTRIBUTE_ID_TITLE,
   /**< Title. */
   QAPI_BLE_ID_SUBTITLE_E       = QAPI_BLE_ANCS_NOTIFICATION_ATTRIBUTE_ID_SUBTITLE,
   /**< Subtitile. */
   QAPI_BLE_ID_MESSAGE_E        = QAPI_BLE_ANCS_NOTIFICATION_ATTRIBUTE_ID_MESSAGE,
   /**< Message. */
   QAPI_BLE_ID_MESSAGE_SIZE_E   = QAPI_BLE_ANCS_NOTIFICATION_ATTRIBUTE_ID_MESSAGE_SIZE,
   /**< Message size. */
   QAPI_BLE_ID_DATE_E           = QAPI_BLE_ANCS_NOTIFICATION_ATTRIBUTE_ID_DATE
   /**< Date. */
} qapi_BLE_ANCS_Notification_Attribute_ID_t;

/**
 * Enumeration of the different application attribute IDs
 * that can be used.
 */
typedef enum
{
   QAPI_BLE_ID_DISPLAY_NAME_E = QAPI_BLE_ANCS_APP_ATTRIBUTE_ID_DISPLAY_NAME
   /**< Display name. */
} qapi_BLE_ANCS_App_Attribute_ID_t;

/**
 * Enumerations of the different types of attribute data
 * that can exist.
 */
typedef enum
{
   QAPI_BLE_DT_NOTIFICATION_E, /**< Notification. */
   QAPI_BLE_DT_APPLICATION_E /**< Application. */
} qapi_BLE_ANCS_Attribute_Data_Type_t;

/**
 * Structure that defines the format of the data returned in a
 * notification from the Nofication Source Apple Notification 
 * Center Service (ANCS) characteristic.
 */
typedef struct qapi_BLE_ANCS_Notification_Received_Data_s
{
   /**
    * Event ID.
    */
   qapi_BLE_ANCS_Event_ID_t    EventID;

   /**
    * Event Flags.
    */
   uint8_t                     EventFlags;

    /**
    * Category ID.
    */
   qapi_BLE_ANCS_Category_ID_t CategoryID;

   /**
    * Category Count.
    */
   uint8_t                     CategoryCount;

   /**
    * UID for the notification.
    */
   uint32_t                    NotificationUID;
} qapi_BLE_ANCS_Notification_Received_Data_t;

#define QAPI_BLE_ANCS_NOTIFICATION_RECEIVED_DATA_SIZE    (sizeof(qapi_BLE_ANCS_Notification_Received_Data_t))
/**<
 * Size of the #qapi_BLE_ANCS_Notification_Received_Data_t structure.
 */

/**
 * Structure that defines the format of the data required for each
 * Notification Attribute being requested.
 */
typedef struct qapi_BLE_ANCS_Notification_Attribute_Request_Data_s
{
   /**
    * Notification attribute ID.
    */
   qapi_BLE_ANCS_Notification_Attribute_ID_t NotificationAttributeID;

   /**
    * Attribute maximum length.
    *
    * The member AttributeMaxLength is only required when requesting
    * specific Notification Attributes.
    *
    * The attributes that require this maximum length parameter are
    * specified in the Apple Notification Center Service Specification.
    */
   uint16_t                                  AttributeMaxLength;
} qapi_BLE_ANCS_Notification_Attribute_Request_Data_t;

#define QAPI_BLE_ANCS_NOTIFCATION_ATTRIBUTE_REQUEST_DATA_SIZE  (sizeof(qapi_BLE_ANCS_Notification_Attribute_Request_Data_t))
/**<
 * Size of the #qapi_BLE_ANCS_Notification_Attribute_Request_Data_t
 * structure.
 */

/**
 * Structure that defines the format of the data required for each Application
 * Attribute being requested.
 */
typedef struct qapi_BLE_ANCS_App_Attribute_Request_Data_s
{
   /**
    * Application attribute ID.
    */
   qapi_BLE_ANCS_App_Attribute_ID_t AppAttributeID;

   /**
    * Attribute maximum length.
    *
    * The member AttributeMaxLength is only required when requesting
    * specific Application Attributes. The attributes that require this maximum
    * length parameter are specified in the Apple Notification Center
    * Service Specification.
    *
    * As of ANCS Specification v1.0, no Application Attribute types
    * require a maximum length parameter. This functionality is put in
    * place in case future versions of the specification require it.
    */
   uint16_t                         AttributeMaxLength;
} qapi_BLE_ANCS_App_Attribute_Request_Data_t;

#define QAPI_BLE_ANCS_APP_ATTRIBUTE_REQUEST_DATA_SIZE    (sizeof(qapi_BLE_ANCS_App_Attribute_Request_Data_t))
/**<
 * Size of the #qapi_BLE_ANCS_App_Attribute_Request_Data_t structure.
 */

/**
 * Structure that is a container structure that holds the information
 * related to returned application attribute data.
 */
typedef struct qapi_BLE_ANCS_App_Attribute_Data_s
{
   /**
    * Application attribute ID.
    */
   qapi_BLE_ANCS_App_Attribute_ID_t  AttributeID;

   /**
    * Pointer to the attribute data.
    *
    * The AttributeData member is a NULL-terminated string. It is
    * possible that an Apple device has no information for a requested
    * attribute. In this case, the AttributeData field will be set to
    * NULL.
    */
   uint8_t                          *AttributeData;
} qapi_BLE_ANCS_App_Attribute_Data_t;

#define QAPI_BLE_ANCS_APP_ATTRIBUTE_DATA_SIZE(_x)            ((sizeof(qapi_BLE_ANCS_App_Attribute_Data_t)) + (_x))
/**<
 * This macro is a utility macro that exists to aid in calculating the
 * length of the #qapi_BLE_ANCS_App_Attribute_Data_t struct based on the
 * length of its Attribute Data. The first parameter to this macro is
 * the length of the member AttributeData in bytes.
 */

/**
 * Structure that is a container structure that holds the information
 * related to returned application attribute data.
 */
typedef struct qapi_BLE_ANCS_Attribute_Data_s
{
   /**
    * Identifies the type of attribute data.
    */
   qapi_BLE_ANCS_Attribute_Data_Type_t Type;

   union
   {
      /**
       * Notification attribute ID.
       */
      qapi_BLE_ANCS_Notification_Attribute_ID_t NotificationAttributeID;

      /**
       * Application attribute ID.
       */
      qapi_BLE_ANCS_App_Attribute_ID_t          AppAttributeID;
   }
   /**
    * Holds the ID for the attribute data. The ID depends on the value of
    * the Type field.
    */
    AttributeID;

   /**
    * Pointer to the attribute data.
    *
    * The AttributeData member is a NULL-terminated string. It is
    * possible that an Apple device has no information for a requested
    * attribute. In this case, the AttributeData field will be set to
    * NULL.
    */
   uint8_t                            *AttributeData;
} qapi_BLE_ANCS_Attribute_Data_t;

#define QAPI_BLE_ANCS_ATTRIBUTE_DATA_SIZE                (sizeof(qapi_BLE_ANCS_Attribute_Data_t))
/**< Size of the #qapi_BLE_ANCS_Attribute_Data_t structure. */

/**
 * Structure that holds parsed attribute data for either Notification
 * Attributes or Application Attributes.
 *
 * This data structure is used when calling the
 * qapi_BLE_ANCS_Decode_Attribute_Data() function.
 */
typedef struct qapi_BLE_ANCS_Parsed_Attribute_Data_s
{
   /**
    * Identifies the type of attribute data.
    */
   qapi_BLE_ANCS_Attribute_Data_Type_t Type;
   union
   {
      /**
       * Notification attribute ID.
       */
      uint32_t  NotificationUID;

      /**
       * Application attribute ID.
       */
      uint8_t  *AppIdentifier;
   }
    /**
    * Holds the ID for the request. The ID depends on the value of
    * the Type field.
    */
   RequestIdentifier;

   /**
    * Number of attributes for the AttributeData field.
    */
   uint32_t                            NumberOfAttributes;

   /**
    * Pointer to the attribute data.
    */
   qapi_BLE_ANCS_Attribute_Data_t     *AttributeData;
} qapi_BLE_ANCS_Parsed_Attribute_Data_t;

#define QAPI_BLE_ANCS_PARSED_ATTRIBUTE_DATA_SIZE         (sizeof(qapi_BLE_ANCS_Parsed_Attribute_Data_t))
/**< Size of the #qapi_BLE_ANCS_Parsed_Attribute_Data_t structure. */

/**
 * @brief
 * Utility function that exists to decode data received from
 * the Notification Source ANCS characteristic.
 *
 * @param[in]   BufferLength    Length of the data buffer.
 *
 * @param[in]   Buffer          Buffer that contains the data that
 *                              will be decoded.
 *
 * @param[out]  NotificationReceivedData    Pointer to a structure that
 *                                          will hold the notification
 *                                          received data if this
 *                                          function is successful.
 *
 * @return       Zero if successful.
 *
 * @return       An error code if negative; one of the following values:
 *               @par
 *                  QAPI_BLE_ANCS_ERROR_INVALID_PARAMETER
 */
QAPI_BLE_DECLARATION int QAPI_BLE_BTPSAPI qapi_BLE_ANCS_Decode_Notification_Received_Data(uint32_t BufferLength, uint8_t *Buffer, qapi_BLE_ANCS_Notification_Received_Data_t *NotificationReceivedData);

/**
 * @brief
 * Utility function that exists to encode a Notification
 * Attribute request into a raw data buffer.
 *
 * @details
 * The caller is responsible for freeing this buffer once they are finished
 * with it with a call to qapi_BLE_ANCS_Free_Notification_Attribute_Request().
 *
 * @param[in]   NotificationUID    Unique ID of the notification for
 *                                 which the attributes are being
 *                                 requested.
 *
 * @param[in]   NumberAttributes   Number of attributes to be
 *                                 encoded.
 *
 * @param[in]   NotificationAttributes    Pointer to a structure that
 *                                        will hold the notification
 *                                        attribute data that will be
 *                                        encoded if this function is
 *                                        successful.
 *
 * @param[out]   Buffer         Pointer to the buffer that will be
 *                              allocated by this function and will contain
 *                              the encoded data if this function is
 *                              successful.
 *
 * @param[out]   TotalLength   Length of the encoded data buffer.
 *
 * @return       Zero if successful.
 *
 * @return       An error code if negative; one of the following values:
 *               @par
 *                  QAPI_BLE_ANCS_ERROR_INVALID_PARAMETER
 */
QAPI_BLE_DECLARATION int QAPI_BLE_BTPSAPI qapi_BLE_ANCS_Encode_Notification_Attribute_Request(uint32_t NotificationUID, uint32_t NumberAttributes, qapi_BLE_ANCS_Notification_Attribute_Request_Data_t *NotificationAttributes, uint8_t **Buffer, uint32_t *TotalLength);

#define QAPI_BLE_ANCS_Free_Notification_Attribute_Request(_x)  BTPS_FreeMemory(_x)
/**<
 * This macro is provided as a mechanism to free an allocated buffer from
 * a successful call to
 * qapi_BLE_ANCS_Encode_Notification_Attribute_Request().
 */

/**
 * @brief
 * Utility function that exists to encode an Application
 * Attribute request into a raw data buffer.
 *
 * @details
 * The caller is responsible for freeing this buffer once they are finished
 * with it with a call to qapi_BLE_ANCS_Free_Notification_Attribute_Request().
 *
 * @param[in]   AppIdentifier    NULL-terminated UTF-8 string, which
 *                               specifes the application for which attributes are
 *                               being requested.
 *
 * @param[in]   NumberAttributes    Number of attributes to be
 *                                  encoded.
 *
 * @param[in]   AppAttributes       Pointer to a structure that will
 *                                  hold the notification attribute data
 *                                  that will be encoded if this function
 *                                  is successful.
 *
 * @param[out]   Buffer         Pointer to the buffer that will be
 *                              allocated by this function and will contain
 *                              the encoded data if this function is
 *                              successful.
 *
 * @param[out]   TotalLength   Length of the encoded data buffer.
 *
 * @return       Zero if successful.
 *
 * @return       An error code if negative; one of the following values:
 *               @par
 *                  QAPI_BLE_ANCS_ERROR_INVALID_PARAMETER
 */
QAPI_BLE_DECLARATION int QAPI_BLE_BTPSAPI qapi_BLE_ANCS_Encode_App_Attribute_Request(char *AppIdentifier, uint32_t NumberAttributes, qapi_BLE_ANCS_App_Attribute_Request_Data_t *AppAttributes, uint8_t **Buffer, uint32_t *TotalLength);

#define QAPI_BLE_ANCS_Free_App_Attribute_Request(_x)     BTPS_FreeMemory(_x)
/**<
 * This macro is provided as a mechanism to free an allocated buffer from
 * a successful call to qapi_BLE_ANCS_Encode_App_Attribute_Request().
 */

/**
 * @brief
 * Utility function that exists to determine if a specified
 * buffer of data from an ANCS Attribute Data Notification is a complete
 * response.
 *
 * @param[in]   BufferLength     NULL-terminated UTF-8 string, which
 *                               specifes the application for which attributes are
 *                               being requested.
 *
 * @param[in]   Buffer          Pointer to the buffer that contains the
 *                              encoded data.
 *
 * @param[in]   NumberAttributesRequested   Number of attributes requested.
 *
 * @return       TRUE -- The data is completed and can be sent to the
 *               qapi_BLE_ANCS_Convert_Raw_Attribute_Data_To_Parsed_Attribute_Data()
 *               function. \n
 *               FALSE -- The caller should wait for more notifications from the
 *               remote device and add the data to the current input buffer.
 */
QAPI_BLE_DECLARATION boolean_t QAPI_BLE_BTPSAPI qapi_BLE_ANCS_Is_Buffer_Complete(uint32_t BufferLength, uint8_t *Buffer, uint32_t NumberAttributesRequested);

/**
 * @brief
 * Utility function that exists to parse the specified Raw
 * Attribute Data Stream into Parsed Attribute Data in the form of
 * #qapi_BLE_ANCS_Parsed_Attribute_Data_t.
 *
 * @param[in]   NumberAttributes    Number of attributes to decode.
 *
 * @param[in]   RawDataLength       Length of the raw stream (must be
 *                                  greater than zero).
 *
 * @param[in]   RawAttributeData    Pointer to the raw data stream.
 *
 * @param[out]  ParsedAttributeData    Pointer to the structure that
 *                                     will hold the Parsed Attribute
 *                                     Data if this function is
 *                                     succssful.
 *
 * @return       Zero if successful.
 *
 * @return       An error code if negative; one of the following values:
 *               @par
 *                  QAPI_BLE_ANCS_ERROR_INVALID_PARAMETER
 */
QAPI_BLE_DECLARATION int QAPI_BLE_BTPSAPI qapi_BLE_ANCS_Decode_Attribute_Data(uint32_t NumberAttributes, uint32_t RawDataLength, uint8_t *RawAttributeData, qapi_BLE_ANCS_Parsed_Attribute_Data_t *ParsedAttributeData);

/**
 * @brief
 * Allows a mechanism to free all resources that were
 * allocated to parse a Raw Attribute Data Stream into Parsed Attribute Data. See
 * the qapi_BLE_ANCS_Decode_Attribute_Data() function for more information.
 *
 * @param[in]   ParsedAttributeData  Pointer to the parsed attribute
 *                                   data that will be freed.
 *
 * @return      None.
 */
QAPI_BLE_DECLARATION void QAPI_BLE_BTPSAPI qapi_BLE_ANCS_Free_Parsed_Attribute_Data(qapi_BLE_ANCS_Parsed_Attribute_Data_t * ParsedAttributeData);

/** @}
 */

#endif

