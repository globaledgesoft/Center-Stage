/*
 * Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
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

#include <stdint.h>

#include "QPKTZR.h"

   /* ***************************************************************** */
   /* **                                                             ** */
   /* **                 INTERNAL MODULE MACRO'S                     ** */
   /* **                                                             ** */
   /* ***************************************************************** */

   /* The following MACRO's simply map to the required platforms memory */
   /* allocation/free routines.                                         */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define _COPY_PACKETIZER_MEMORY(_x, _y, _z)                 memcpy(_x, _y, _z)
#define _ALLOCATE_PACKETIZER_MEMORY(_x)                     malloc(_x)
#define _FREE_PACKETIZER_MEMORY(_x)                         free(_x)

   /* Convert an unaligned, little endian, 16-bit integer to an aligned */
   /* host order value.                                                 */
   /*                                                                   */
   /* Parameters:                                                       */
   /*    __src__ address of the input value                             */
   /*                                                                   */
   /* Returns:                                                          */
   /*    aligned, host order value                                      */
#define READ_UNALIGNED_LITTLE_ENDIAN_UINT16(__src__)                                \
   ((uint16_t)((((uint32_t)(((uint8_t *)(__src__))[1])) << 8) |                     \
                ((uint32_t)(((uint8_t *)(__src__))[0]))))

   /* Convert an unaligned, little endian, 32-bit integer to an aligned */
   /* host order value.                                                 */
   /*                                                                   */
   /* Parameters:                                                       */
   /*    __src__ address of the input value                             */
   /*                                                                   */
   /* Returns:                                                          */
   /*    aligned, host order value                                      */
#define READ_UNALIGNED_LITTLE_ENDIAN_UINT32(__src__)                                \
   ((uint32_t)((((uint32_t)(((uint8_t *)(__src__))[3])) << 24) |                    \
               (((uint32_t)(((uint8_t *)(__src__))[2])) << 16) |                    \
               (((uint32_t)(((uint8_t *)(__src__))[1])) << 8) |                     \
                ((uint32_t)(((uint8_t *)(__src__))[0]))))

   /* Convert a host-order 32-bit integer to an unaligned, little       */
   /* endian 32-bit integer.                                            */
   /*                                                                   */
   /* Parameters:                                                       */
   /*    __dest__ address for the output                                */
   /*    __src__  input value                                           */
#define WRITE_UNALIGNED_LITTLE_ENDIAN_UINT32(__dest__, __src__)                     \
{                                                                                   \
  ((uint8_t *)(__dest__))[0] = ((uint8_t)(((uint32_t)(__src__)) & 0xFF));           \
  ((uint8_t *)(__dest__))[1] = ((uint8_t)((((uint32_t)(__src__)) >> 8) & 0xFF));    \
  ((uint8_t *)(__dest__))[2] = ((uint8_t)((((uint32_t)(__src__)) >> 16) & 0xFF));   \
  ((uint8_t *)(__dest__))[3] = ((uint8_t)((((uint32_t)(__src__)) >> 24) & 0xFF));   \
}

   /* ***************************************************************** */
   /* **                                                             ** */
   /* **                   TABI INFORMATION                          ** */
   /* **                                                             ** */
   /* ***************************************************************** */

   /* The following represents the format of a TABI packet (Version 1). */
   /*    - uint32_t:    Group ID                                        */
   /*    - uint32_t:    Length                                          */
   /*    - uint8_t:     Payload (variable)                              */
   /* * NOTE * Group ID and Length are in Little Endian format.         */
#define TABI_VERSION_1_HEADER_SIZE                          8

#define TABI_VERSION_1_HEADER_GROUP_ID_OFFSET               0
#define TABI_VERSION_1_HEADER_LENGTH_OFFSET                 (TABI_VERSION_1_HEADER_GROUP_ID_OFFSET + sizeof(uint32_t))

   /* The following MACRO tests to see if this is a valid start of a    */
   /* TABI Version 1 packet.                                            */
   /* * NOTE * Any byte value that is larger than the 15.4 TABI Group   */
   /*          ID will be considered a valid start of aTABI packet.     */
#define TABI_VERSION_1_CHECK_START_BYTE(_x)                 (((uint8_t)(_x) >= 0x10) && (((uint8_t)(_x)) < 0x51))

   /* The following represents the format of a TABI packet (Version 2). */
   /*    - unint8_t:    Start Packet Delimeter (0x51)                   */
   /*    - uint32_t:    Group ID                                        */
   /*    - uint32_t:    Length                                          */
   /*    - uint8_t:     Checksum                                        */
   /*    - uint8_t:     Payload (variable)                              */
   /* * NOTE * Group ID and Length are in Little Endian format.         */
   /* * NOTE * Checksum is simply a Checksum of the Group ID and the    */
   /*          Length (in the order it came through - Little Endian).   */
#define TABI_VERSION_2_HEADER_SIZE                          10

#define TABI_VERSION_2_HEADER_START_PACKET_DELIM_OFFSET     0
#define TABI_VERSION_2_HEADER_GROUP_ID_OFFSET               (TABI_VERSION_2_HEADER_START_PACKET_DELIM_OFFSET + sizeof(uint8_t))
#define TABI_VERSION_2_HEADER_LENGTH_OFFSET                 (TABI_VERSION_2_HEADER_GROUP_ID_OFFSET + sizeof(uint32_t))
#define TABI_VERSION_2_HEADER_CHECKSUM_OFFSET               (TABI_VERSION_2_HEADER_LENGTH_OFFSET + sizeof(uint32_t))

   /* The following constants represent the starting offset of the TABI */
   /* Checksum and the number of bytes that are included in the         */
   /* checksum.                                                         */
#define TABI_VERSION_2_HEADER_CHECKSUM_START_OFFSET         (TABI_VERSION_2_HEADER_GROUP_ID_OFFSET)
#define TABI_VERSION_2_HEADER_CHECKSUM_LENGTH               (sizeof(uint32_t) * 2)

   /* The following constant represents the start delimeter for a TABI  */
   /* packet (Version 2).                                               */
#define TABI_VERSION_2_START_BYTE                           0x51

   /* The following represents the Group ID of the 15.4 TABI packets.   */
#define TABI_GROUP_ID_15_4                                  0x00000010
#define TABI_GROUP_ID_UART                                  0x00000020
#define TABI_GROUP_ID_COEX                                  0x00000040

   /* ***************************************************************** */
   /* **                                                             ** */
   /* **               BLUETOOTH PACKET INFORMATION                  ** */
   /* **                                                             ** */
   /* ***************************************************************** */

   /* The following container structure holds all pertinent header      */
   /* information about an HCI packet header.  This information is:     */
   /*   - Length of the entire header (in bytes)                        */
   /*   - Starting byte offset of the Length (in bytes)                 */
   /*   - Whether or not the length is 1 or 2 bytes                     */
typedef struct _tagHCIHeaderInformation_t
{
   unsigned int HeaderLength;
   unsigned int LengthOffset;
   unsigned int Length16;
} HCIHeaderInformation_t;

   /* The following table represents the header information for each    */
   /* defined HCI Packet type.                                          */
   /* * NOTE * This array is simply indexed with the actual Packetizer  */
   /*          Packet Type to get the correct HCI Header.               */
static const HCIHeaderInformation_t HCIHeaderInformation[] =
{
   { 0, 0, 0 }, /* Unused.                                                    */
   { 3, 2, 0 }, /* QPKTZR_PACKET_TYPE_HCI_COMMAND   - Opcode[2], Length[1]    */
   { 4, 2, 1 }, /* QPKTZR_PACKET_TYPE_HCI_ACL_DATA  - Handle[2], Length [2]   */
   { 3, 2, 0 }, /* QPKTZR_PACKET_TYPE_HCI_SCO_DATA  - Handle[2], Length[1]    */
   { 2, 1, 0 }  /* QPKTZR_PACKET_TYPE_HCI_EVENT     - EventCode[1], Length[1] */
} ;

   /* ***************************************************************** */
   /* **                                                             ** */
   /* **               PACKETIZER STATE INFORMATION                  ** */
   /* **                                                             ** */
   /* ***************************************************************** */

   /* Enumerated States of the Bluetooth/TABI Packet Builder.           */
typedef enum
{
   psLOOKING,
   psWAIT_HEADER,
   psCOMPILING
} ParseState_t;

   /* ***************************************************************** */
   /* **                                                             ** */
   /* **                 INTERNAL MODULE VARIABLES                   ** */
   /* **                                                             ** */
   /* ***************************************************************** */

   /* The following variable flags whether or not the module has been   */
   /* initialized or not.                                               */
static int              Initialized;

   /* The following variable flags whether or not we are processing all */
   /* TABI packets as Version 1 (0) or Version 2 (non-zero).            */
static int              TABIVersion2;

   /* The following variables represent the packeting state variables.  */
static ParseState_t     ParseState;
static unsigned int     PacketIndex;
static unsigned int     PacketType;
static unsigned char    PacketHeader[16];
static unsigned int     PacketLength;
static unsigned char   *PacketBuffer;
static unsigned int     UnknownTABIGroupID;
static QPKTZR_Packet_t *Packet;

   /* ***************************************************************** */
   /* **                                                             ** */
   /* **                 INTERNAL MODULE FUNCTIONS                   ** */
   /* **                                                             ** */
   /* ***************************************************************** */

static int ParseStream(unsigned int StreamBufferSize, unsigned char *StreamBuffer, QPKTZR_Packet_t **ParsedPacket);
static uint8_t CalculateTABIChecksum(unsigned int Length, unsigned char *Data);
static QPKTZR_Packet_t *VerifyParsedPacket(void);

   /* The following function is responsible for parsing the specified   */
   /* input stream data and trying to parse it into a packetized packet.*/
static int ParseStream(unsigned int StreamBufferSize, unsigned char *StreamBuffer, QPKTZR_Packet_t **ParsedPacket)
{
   int          ret_val = 0;
   unsigned int Length;
   unsigned int DataLength;
   unsigned int TabiGroup;

   /* First, verify that the input parameters appear to be semi-valid.  */
   if((StreamBufferSize) && (StreamBuffer) && (ParsedPacket))
   {
      /* Flag that we have not found a valid packet.                    */
      *ParsedPacket = NULL;

      /* Loop through the data and parse the specified stream data.     */
      DataLength    = StreamBufferSize;

      while((DataLength) && (!ret_val))
      {
         if(ParseState == psLOOKING)
         {
            /* While we are looking, check for a valid Packet ID.  When */
            /* a Packet ID is found, we must save the Packet type.  We  */
            /* can Check to see if this is a TABI Version 1 packet by   */
            /* simply looking at the byte value we encounter.           */
            /* * NOTE * We are doing this because:                      */
            /*             - All TABI group ID's are outside the range  */
            /*               of Bluetooth HCI packets.                  */
            /*             - TABI packets are in little endian format   */
            /*               (so the first byte will be the least       */
            /*               significant byte).                         */
            /* * NOTE * For this to work TABI has guarantee that the    */
            /*          least significant byte of any TABI Group ID     */
            /*          will never match a Bluetooth HCI defined        */
            /*          packet.  This is noot really a problem because  */
            /*          TABI is aware of this limitation (and it is an  */
            /*          internal protocol that is completely controlled */
            /*          by Qualcomm).                                   */
            if((*StreamBuffer == QPKTZR_PACKET_TYPE_HCI_COMMAND) || (*StreamBuffer == QPKTZR_PACKET_TYPE_HCI_ACL_DATA) || (*StreamBuffer == QPKTZR_PACKET_TYPE_HCI_SCO_DATA) || (*StreamBuffer == QPKTZR_PACKET_TYPE_HCI_EVENT) || TABI_VERSION_1_CHECK_START_BYTE(*StreamBuffer))
            {
               ParseState         = psWAIT_HEADER;
               PacketIndex        = 0;
               PacketBuffer       = PacketHeader;
               PacketType         = *StreamBuffer;
               UnknownTABIGroupID = 0;

               /* Process TABI packets specially due to V1/V2           */
               /* differences.                                          */
               if(TABI_VERSION_1_CHECK_START_BYTE(*StreamBuffer))
               {
                  /* Note we will treat all packets as 15.4 Group ID so */
                  /* as to not confuse the packetizer (we will not      */
                  /* dispatch the built packet, however, so as to not   */
                  /* confuse the caller.                                */
                  PacketType   = QPKTZR_PACKET_TYPE_15_4;

                  PacketLength = TABI_VERSION_1_HEADER_SIZE;

                  if(TABIVersion2)
                     PacketLength++;

                  /* We need to include this byte in the Packet Header. */
                  PacketHeader[PacketIndex++] = *StreamBuffer;
               }
               else
               {
                  /* HCI Packet, note the Header Length.                */
                  PacketLength = HCIHeaderInformation[*StreamBuffer].HeaderLength;
               }
            }
            else
            {
               /* Not an HCI, 15.4, or Coex TABI packet, check for TABI */
               /* Version 2.                                            */
               if(*StreamBuffer == TABI_VERSION_2_START_BYTE)
               {
                  /* Flag that we have found a TABI Version 2 packet.   */
                  /* * NOTE * We do not change the state because we     */
                  /*          we can deal with the extra Checksum when  */
                  /*          we get to it by processing the rest as a  */
                  /*          TABI Version 1 packet.                    */
                  TABIVersion2 = 1;
               }
            }

            StreamBuffer++;
            DataLength--;
         }
         else
         {
            /* Go ahead and place the next byte in our current packet   */
            /* buffer.                                                  */
            PacketBuffer[PacketIndex++] = *StreamBuffer;

            StreamBuffer++;
            DataLength--;

            /* Compile as much of the header that is needed for each    */
            /* packet type.                                             */
            if(ParseState == psWAIT_HEADER)
            {
               /* Check to see if the Event Packet can be processed.    */
               /* * NOTE * We will key off the length of data that we   */
               /*          have received (take note that we have already*/
               /*          incremented the packet index).               */
               /* * NOTE * The length of each packet header (for each   */
               /*          packet type) is:                             */
               /*             - HCI Commmand - 3 bytes                  */
               /*             - HCI Event    - 2 bytes                  */
               /*             - HCI ACL Data - 4 bytes                  */
               /*             - HCI SCO Data - 3 bytes                  */
               /*             - TABI V. 1    - 8 bytes                  */
               /*             - TABI V. 2    - 9 bytes                  */
               if(PacketIndex == PacketLength)
               {
                  /* Header has been completely read, process the       */
                  /* packet:                                            */
                  /*    - Verify checksum (TABI V2 only)                */
                  /*    - Allocate space to hold the packet data        */
                  /*    - Copy required header information to the       */
                  /*      newly allocated packet                        */
                  if((PacketType == QPKTZR_PACKET_TYPE_15_4) || (PacketType == QPKTZR_PACKET_TYPE_COEX) || (PacketType == QPKTZR_PACKET_TYPE_UART))
                  {
                     /* TABI packet.                                    */
                     if(TABIVersion2)
                     {
                        /* Verify the Checksum.                         */
                        /* * NOTE * We cannot include the Checksum      */
                        /*          byte in our testing of the          */
                        /*          Checksum.                           */
                        if(PacketHeader[TABI_VERSION_1_HEADER_SIZE] != CalculateTABIChecksum(TABI_VERSION_1_HEADER_SIZE, PacketHeader))
                        {
                           /* Checksum doesn't match, go ahead and toss */
                           /* it.                                       */
                           PacketLength = 0;
                        }
                     }

                     /* Note the Packet Length of the actual data.      */
                     PacketLength = READ_UNALIGNED_LITTLE_ENDIAN_UINT32(&(PacketHeader[TABI_VERSION_1_HEADER_LENGTH_OFFSET]));

                     /* Verify the Group ID and pull out the Length.    */
                     /* * NOTE * TABI Version 1 and Version 2 both      */
                     /*          start with the same format:            */
                     /*             - Group ID                          */
                     /*             - Length                            */
                     TabiGroup = READ_UNALIGNED_LITTLE_ENDIAN_UINT32(PacketHeader);
                     if(TabiGroup == TABI_GROUP_ID_15_4)
                        PacketType = QPKTZR_PACKET_TYPE_15_4;
                     else if(TabiGroup == TABI_GROUP_ID_COEX)
                        PacketType = QPKTZR_PACKET_TYPE_COEX;
                     else if(TabiGroup == TABI_GROUP_ID_UART)
                        PacketType = QPKTZR_PACKET_TYPE_UART;
                     else
                        UnknownTABIGroupID = 1;
                  }
                  else
                  {
                     /* HCI Packet, go ahead and determine the header   */
                     /* length.                                         */
                     if(HCIHeaderInformation[PacketType].Length16)
                        PacketLength = READ_UNALIGNED_LITTLE_ENDIAN_UINT16(&(PacketHeader[HCIHeaderInformation[PacketType].LengthOffset]));
                     else
                        PacketLength = PacketHeader[HCIHeaderInformation[PacketType].LengthOffset];

                     /* We need to adjust the length of the data to     */
                     /* account for the bytes that occurred BEFORE the  */
                     /* Length in the HCI Packets.                      */
                     if(PacketLength)
                        PacketLength += HCIHeaderInformation[PacketType].HeaderLength;
                  }

                  /* If we have determined that the packet is valid then*/
                  /* the length of the packet will be contained in      */
                  /* PacketLength.                                      */
                  /* * NOTE * There is no way for an HCI packet to have */
                  /*          a non-zero packet length because there    */
                  /*          will always be an HCI header.  We cannot  */
                  /*          accept a zero length TABI packet, though. */
                  if(((PacketLength) && ((PacketType == QPKTZR_PACKET_TYPE_15_4) || (PacketType == QPKTZR_PACKET_TYPE_COEX) || (PacketType == QPKTZR_PACKET_TYPE_UART))) || ((PacketType != QPKTZR_PACKET_TYPE_15_4) && (PacketType != QPKTZR_PACKET_TYPE_COEX) && (PacketType != QPKTZR_PACKET_TYPE_UART)))
                  {
                     /* Allocate the memory to hold the packet.         */
                     if((Packet = _ALLOCATE_PACKETIZER_MEMORY(sizeof(QPKTZR_Packet_t) + PacketLength)) != NULL)
                     {
                        /* Flag that we are now in the compiling packet */
                        /* state.                                       */
                        ParseState           = psCOMPILING;

                        /* Initialize the packet information.           */
                        Packet->PacketType   = PacketType;
                        Packet->PacketLength = 0;
                        Packet->PacketData   = ((unsigned char *)Packet) + sizeof(QPKTZR_Packet_t);

                        PacketBuffer         = Packet->PacketData;

                        /* Now, go ahead and copy the header over and   */
                        /* fix up the remaining length of expected data.*/
                        if((PacketType == QPKTZR_PACKET_TYPE_15_4) || (PacketType == QPKTZR_PACKET_TYPE_COEX) || (PacketType == QPKTZR_PACKET_TYPE_UART))
                        {
                           /* Simply set the Packet Index to zero, the  */
                           /* Packet Length will be the length that was */
                           /* already read (i.e.  no need to copy any   */
                           /* header).                                  */
                           PacketIndex  = 0;
                        }
                        else
                        {
                           /* Bluetooth HCI Packet, we need to copy over*/
                           /* the existing header and fix up the packet */
                           /* buffer.                                   */
                           /* * NOTE * Keep in mind that we are actually*/
                           /*          copying the header size PLUS the */
                           /*          next byte we already consumed.   */
                           _COPY_PACKETIZER_MEMORY(PacketBuffer, PacketHeader, HCIHeaderInformation[PacketType].HeaderLength);

                           /* Check to see if the packet has been       */
                           /* completely received.                      */
                           if(!PacketLength)
                           {
                              if((*ParsedPacket = VerifyParsedPacket()) != NULL)
                              {
                                 /* Flag that the remaining data has NOT*/
                                 /* been consumed (i.e.  we processed a */
                                 /* packet).                            */
                                 ret_val = DataLength;
                              }
                           }
                        }
                     }
                     else
                     {
                        /* Unable to allocate memory, go ahead and      */
                        /* silently fail and try to recover.            */
                        ParseState = psLOOKING;
                     }
                  }
                  else
                     ParseState = psLOOKING;
               }
            }
            else
            {
               /* Keep saving the data until all data is received.      */
               if(ParseState == psCOMPILING)
               {
                  /* Optimization to bulk copy any remaining data that  */
                  /* is required and might be present in the stream     */
                  /* buffer.                                            */
                  if(PacketIndex < PacketLength)
                  {
                     /* Calculate how many bytes remain in this packet. */
                     Length = (PacketLength - PacketIndex);
                     if(Length > DataLength)
                        Length = DataLength;

                     _COPY_PACKETIZER_MEMORY(&(PacketBuffer[PacketIndex]), StreamBuffer, Length);

                     PacketIndex  += Length;
                     StreamBuffer += Length;
                     DataLength   -= Length;
                  }

                  /* Check to see if the packet has been completely     */
                  /* received.                                          */
                  if(PacketIndex == PacketLength)
                  {
                     if((*ParsedPacket = VerifyParsedPacket()) != NULL)
                     {
                        /* Flag that the remaining data has NOT been    */
                        /* consumed (i.e.  we processed a packet).      */
                        ret_val = DataLength;
                     }
                  }
               }
            }
         }
      }
   }
   else
      ret_val = -10;

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* calculate the Checksum for a TABI packet given the specified data.*/
static uint8_t CalculateTABIChecksum(unsigned int Length, unsigned char *Data)
{
   uint8_t ret_val = 0;

   /* Loop through data and calculate the TABI checksum.                */
   while(Length--)
   {
      ret_val -= *Data;

      Data++;
   }

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is an internal function that exists to     */
   /* verify if a parsed packet is valid and update the current parsing */
   /* state based on this information.  This function returns a zero if */
   /* the packet was deemed to be invalid, or a non-zero value if it was*/
   /* determined the packet is valid.  Regardless of the case, the      */
   /* parsing state is reset to the looking state.                      */
static QPKTZR_Packet_t *VerifyParsedPacket(void)
{
   QPKTZR_Packet_t *ret_val = NULL;

   /* Go ahead and reset the state to looking (regardless if the packet */
   /* is valid).                                                        */
   ParseState = psLOOKING;

   if(Packet)
   {
      /* Fix up the Packet Length, and break out of the loop.           */
      Packet->PacketLength = PacketIndex;
      ret_val              = Packet;
      Packet               = NULL;

      /* If we do not know the TABI Group ID, then go ahead and eat the */
      /* packet so as to not confuse the calling application since we   */
      /* claim we only support 802.15.4 TABI Group ID's.                */
      if((PacketType == QPKTZR_PACKET_TYPE_15_4) && (UnknownTABIGroupID))
      {
         /* Unknown TABI Group ID so just toss the packet.              */
         _FREE_PACKETIZER_MEMORY(ret_val);

         /* Flag that the packet has not been processed.                */
         ret_val = NULL;
      }
   }

   /* Return result to the caller.                                      */
   return(ret_val);
}

   /* ***************************************************************** */
   /* **                                                             ** */
   /* **                   PUBLIC MODULE FUNCTIONS                   ** */
   /* **                                                             ** */
   /* ***************************************************************** */

   /* The following function is used to initialize the Packetizer       */
   /* module.  This function returns zero if successful or a negative   */
   /* return error code if there was an error.                          */
int QPKTZR_Initialize(void)
{
   int ret_val;

   /* First check to see if this module has already been initialized.   */
   if(!Initialized)
   {
      /* Initialize required state variables.                           */
      ParseState   = psLOOKING;
      Packet       = NULL;
      PacketIndex  = 0;
      PacketBuffer = PacketHeader;
      TABIVersion2 = 0;

      /* Flag that this module has been initialized.                    */
      Initialized  = 1;

      /* Flag success to the caller.                                    */
      ret_val      = 0;
   }
   else
      ret_val = -1;

   /* Simply return the result to the caller.                           */
   return(ret_val);
}

   /* The following function is used to de-initialize the Packetizer    */
   /* module.  This function returns zero if successful or a negative   */
   /* return error code if there was an error.                          */
int QPKTZR_Cleanup(void)
{
   int ret_val;

   /* First check to see if this module has already been initialized.   */
   if(Initialized)
   {
      /* If we have begun to build a packet, then free the memory that  */
      /* was allocated.                                                 */
      if(Packet)
         _FREE_PACKETIZER_MEMORY(Packet);

      /* Flag that this module has not been initialized.                */
      Initialized = 0;

      /* Flag success to the caller.                                    */
      ret_val     = 0;
   }
   else
      ret_val = -1;

   /* Simply return the result to the caller.                           */
   return(ret_val);
}

   /* The following function is called to process received stream data. */
   /* If the stream data contains a valid packet that was packetized, it*/
   /* is returned as the parsed packet.  The return value from this     */
   /* function represents the number of bytes that are still left in the*/
   /* stream (zero means all bytes were processed).  This function      */
   /* returns a negative return code if there was an error parsing the  */
   /* stream.                                                           */
   /* * NOTE * If this function returns a parsed packet then the        */
   /*          caller *MUST* free this memory by calling the            */
   /*          QPKTZR_Free_Packet_Memory() function.                    */
   /* * NOTE * If this function returns a positive value, then the      */
   /*          caller should call this function again with the remaining*/
   /*          stream buffer bytes (and size).                          */
int QPKTZR_Packetize_Stream(unsigned int StreamBufferSize, unsigned char *StreamBuffer, QPKTZR_Packet_t **ParsedPacket)
{
   int ret_val;

   /* First check to see if this module has already been initialized.   */
   if(Initialized)
   {
      /* Next make sure the input parameters appear to be semi-valid.   */
      if((StreamBufferSize) && (StreamBuffer) && (ParsedPacket))
      {
         /* Initialize that we have not parsed a packet.                */
         *ParsedPacket = NULL;

         /* Now let's loop through each byte and attempt to packetize   */
         /* the data.                                                   */
         ret_val = ParseStream(StreamBufferSize, StreamBuffer, ParsedPacket);
      }
      else
         ret_val = -2;
   }
   else
      ret_val = -1;

   /* Simply return the result to the caller.                           */
   return(ret_val);
}

   /* The folling function accepts an already packetized packet and     */
   /* calculates the correct stream header that is to be sent (in front */
   /* of the packet data).  This function requires the caller to specify*/
   /* the stream buffer and stream buffer size.  This function can be   */
   /* called with NULL as the stream buffer AND zero as the stream      */
   /* length and it will calculate the required stream header length and*/
   /* return it as the return value from this function.  This function  */
   /* returns a positive value that represents the size of the required */
   /* stream header if successful, or a negative return value if there  */
   /* was an error.                                                     */
int QPKTZR_Build_Stream_Header(QPKTZR_Packet_t *Packet, unsigned int StreamBufferSize, unsigned char *StreamBuffer)
{
   int ret_val;

   /* First check to see if this module has already been initialized.   */
   if(Initialized)
   {
      /* Next make sure the input parameters appear to be semi-valid.   */
      if((Packet) && (Packet->PacketLength) && (Packet->PacketData) && (((!StreamBufferSize) && (!StreamBuffer)) || ((StreamBufferSize) && (StreamBuffer))))
      {
         switch(Packet->PacketType)
         {
            case QPKTZR_PACKET_TYPE_HCI_COMMAND:
            case QPKTZR_PACKET_TYPE_HCI_ACL_DATA:
            case QPKTZR_PACKET_TYPE_HCI_SCO_DATA:
            case QPKTZR_PACKET_TYPE_HCI_EVENT:
               /* Flag the length of the Header (always 1 byte for      */
               /* Bluetooth).                                           */
               ret_val = 1;

               /* Check to see if a buffer was specified, if it was     */
               /* simply map the header to be the HCI Packet Type (which*/
               /* maps directly to the Parsed Packet Type).             */
               if(StreamBuffer)
                  StreamBuffer[0] = (unsigned char)Packet->PacketType;
               break;
            case QPKTZR_PACKET_TYPE_15_4:
            case QPKTZR_PACKET_TYPE_COEX:
            case QPKTZR_PACKET_TYPE_UART:
               /* Determine what TABI version we are using.             */
               if(TABIVersion2)
               {
                  /* Format TABI Version 2 Header.                      */
                  ret_val = TABI_VERSION_2_HEADER_SIZE;

                  /* Check to see if a buffer was specified to hold the */
                  /* header.                                            */
                  if(StreamBuffer)
                  {
                     /* Buffer specified, make sure it is large enough. */
                     if(StreamBufferSize >= (unsigned int)ret_val)
                     {
                        /* Format the TABI Header for this packet.      */
                        StreamBuffer[TABI_VERSION_2_HEADER_START_PACKET_DELIM_OFFSET] = TABI_VERSION_2_START_BYTE;

                        if(Packet->PacketType == QPKTZR_PACKET_TYPE_15_4)
                        {
                           WRITE_UNALIGNED_LITTLE_ENDIAN_UINT32(&(StreamBuffer[TABI_VERSION_2_HEADER_GROUP_ID_OFFSET]), TABI_GROUP_ID_15_4);
                        }
                        else if(Packet->PacketType == QPKTZR_PACKET_TYPE_COEX)
                        {
                           WRITE_UNALIGNED_LITTLE_ENDIAN_UINT32(&(StreamBuffer[TABI_VERSION_2_HEADER_GROUP_ID_OFFSET]), TABI_GROUP_ID_COEX);
                        }
                        else
                        {
                           WRITE_UNALIGNED_LITTLE_ENDIAN_UINT32(&(StreamBuffer[TABI_VERSION_2_HEADER_GROUP_ID_OFFSET]), TABI_GROUP_ID_UART);
                        }

                        WRITE_UNALIGNED_LITTLE_ENDIAN_UINT32(&(StreamBuffer[TABI_VERSION_2_HEADER_LENGTH_OFFSET]), Packet->PacketLength);

                        /* Header built, so go ahead and calculate the  */
                        /* checksum.                                    */
                        StreamBuffer[TABI_VERSION_2_HEADER_CHECKSUM_OFFSET] = CalculateTABIChecksum(TABI_VERSION_2_HEADER_CHECKSUM_LENGTH, &(StreamBuffer[TABI_VERSION_2_HEADER_CHECKSUM_START_OFFSET]));
                     }
                     else
                        ret_val = -4;
                  }
               }
               else
               {
                  /* Format TABI Version 1 Header.                      */
                  ret_val = TABI_VERSION_1_HEADER_SIZE;

                  /* Check to see if a buffer was specified to hold the */
                  /* header.                                            */
                  if(StreamBuffer)
                  {
                     /* Buffer specified, make sure it is large enough. */
                     if(StreamBufferSize >= (unsigned int)ret_val)
                     {
                        /* Format the TABI Header for this packet.      */
                        if(Packet->PacketType == QPKTZR_PACKET_TYPE_15_4)
                        {
                           WRITE_UNALIGNED_LITTLE_ENDIAN_UINT32(&(StreamBuffer[TABI_VERSION_1_HEADER_GROUP_ID_OFFSET]), TABI_GROUP_ID_15_4);
                        }
                        else if(Packet->PacketType == QPKTZR_PACKET_TYPE_COEX)
                        {
                           WRITE_UNALIGNED_LITTLE_ENDIAN_UINT32(&(StreamBuffer[TABI_VERSION_1_HEADER_GROUP_ID_OFFSET]), TABI_GROUP_ID_COEX);
                        }
                        else
                        {
                           WRITE_UNALIGNED_LITTLE_ENDIAN_UINT32(&(StreamBuffer[TABI_VERSION_1_HEADER_GROUP_ID_OFFSET]), TABI_GROUP_ID_UART);
                        }

                        WRITE_UNALIGNED_LITTLE_ENDIAN_UINT32(&(StreamBuffer[TABI_VERSION_1_HEADER_LENGTH_OFFSET]), Packet->PacketLength);
                     }
                     else
                        ret_val = -4;
                  }
               }
               break;
            default:
               /* Invalid packet type passed, so flag an error.         */
               ret_val = -3;
               break;
         }
      }
      else
         ret_val = -2;
   }
   else
      ret_val = -1;

   /* Simply return the result to the caller.                           */
   return(ret_val);
}

   /* The following function is used to free a packet that was          */
   /* successfully parsed from a stream (and returned via a successful  */
   /* call to the QPKTZR_Packetize_Stream() function.                   */
void QPKTZR_Free_Parsed_Packet(QPKTZR_Packet_t *ParsedPacket)
{
   /* First, verify that the packet appears to have been allocated from */
   /* this module.                                                      */
   if((ParsedPacket) && (ParsedPacket->PacketLength) && (ParsedPacket->PacketData == ((unsigned char *)ParsedPacket + sizeof(QPKTZR_Packet_t))))
   {
      /* Simply free the allocated memory.                              */
      _FREE_PACKETIZER_MEMORY(ParsedPacket);
   }
}

