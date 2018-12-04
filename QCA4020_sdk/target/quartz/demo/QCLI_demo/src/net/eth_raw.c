/*
* Copyright (c) 2017 Qualcomm Technologies, Inc.
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

#include <stdlib.h>     /* malloc() */
#include <string.h>     /* memcpy */
#include "bench.h"
#include "qapi_status.h"
#include "qapi_socket.h"
#include "qapi_ns_utils.h"

#ifdef CONFIG_NET_RAW_SOCKET_DEMO
//#define ETHRAW_DEBUG 

extern QCLI_Group_Handle_t qcli_net_handle;
#define PRINTF(fmt, args...)                QCLI_Printf(qcli_net_handle, fmt, ## args)
#define HEXDUMP(inbuf, inlen, ascii, addr)  app_hexdump(inbuf, inlen, ascii, addr)

#define ETH_P_PAE       0x888e
#define RX_BUFFER_SIZE  512
static uint8_t eth_rx_quit;

/*****************************************************************************
 *****************************************************************************/
static void eth_help(void)
{
    PRINTF("eth tx <ifname> <dest mac addr> [-d <data bytes>] [-p <etherType>]\n"); 
    PRINTF("eth rx [-i <ifname>] [-p <etherType>] [-q]\n"); 
    PRINTF("Examples:\n");
    PRINTF(" eth tx wlan1 00:11:22:33:44:55 -p 0x888e -d \"01 00 00 05 01 ab 00 05 01\"\n");
    PRINTF(" eth rx -i wlan1 -p 0x1234\n");
    PRINTF(" eth rx -q\n");
}

#ifdef ETHRAW_DEBUG 
#pragma GCC push_options
#pragma GCC optimize ("O0")
#endif
/************************************************************************
 *     [0]  [1]       [2]               [3]   [4]         [5]  [6]
 * eth tx  <ifname>  <dest MAC addr>    -d  <data bytes>  -p  <etherType>
 * eth tx   wlan1    02:03:7F:20:01:42  -d  "01 02 03 04" -p  0x1234
 ************************************************************************/
static
QCLI_Command_Status_t eth_tx(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *ifname = NULL;
    char *payload = NULL;
    int sock = QAPI_ERROR;
    int rc = QAPI_ERROR;
    int i;
    struct sockaddr_ll foreign_addr;
    struct ethhdr *eth;
    char *da;
    uint32_t mac_addr_len;
    uint8_t *srcmac;
    uint8_t eapol[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* DA */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* SA */
        0x88, 0x8e,                         /* EtherType */
        0x01,           /* EAPOL protocol version */
        0x00,           /* EAPOL type: EAPOL-Packet */ 
        0x00, 0x05,     /* EAPOL payload length */
        0x01,           /* EAP code: request */
        0xaa,           /* EAP id */
        0x00, 0x05,     /* EAP length */
        0x01            /* EAP type: identity */
    };
    /* default to send EAPOL frame */
    uint8_t *pkt = eapol;
    int len = sizeof(eapol);
    uint16_t protocol = ETH_P_PAE;

    if (Parameter_Count < 3)
    {
        eth_help();
        goto end;
    }

    ifname  = Parameter_List[1].String_Value;
    da      = Parameter_List[2].String_Value;

    for (i = 3; i < Parameter_Count; i++)
    {
        if (Parameter_List[i].String_Value[0] == '-')
        {
            switch (Parameter_List[i].String_Value[1])
            {
                case 'i':   /* -i wlan1 */
                    i++;
                    ifname = Parameter_List[i].String_Value;
                    break;

                case 'd':   /* -d "00 01 02 03" */
                    i++;
                    payload = Parameter_List[i].String_Value;
                    break;

                case 'p':   /* -p 0x888e */
                    i++;
                    if (!Parameter_List[i].Integer_Is_Valid ||
                        (protocol = Parameter_List[i].Integer_Value) < 0x600)
                    {
                        PRINTF("ERROR: Invalid etherType: %s\n", Parameter_List[i].String_Value);
                        goto end;
                    }
                    break;

                default:
                    PRINTF("ERROR: Unknown option: %s\n", Parameter_List[i].String_Value);
                    goto end;
            }
        }
        else
        {
            PRINTF("ERROR: Unknown option: %s\n", Parameter_List[i].String_Value);
            goto end;
        }

        if (i == Parameter_Count)
        {
            PRINTF("What is value of %s?\n", Parameter_List[i-1].String_Value);
            goto end;
        }
    }   /* for */

    if (payload)
    {
        len = strlen(payload) + sizeof(struct ethhdr);
        pkt = malloc(len);
        if (pkt == NULL)
        {
            PRINTF("ERROR: No memory\n");
            goto end;
        }
        
        rc = hexstr2bin(payload, pkt + sizeof(struct ethhdr), strlen(payload));
        if (rc < 0)
        {
            PRINTF("ERROR: Invalid data string: %s\n", payload);
            goto end;
        }
        len = rc + sizeof(struct ethhdr);
    }

    /* Open an socket */
    sock = qapi_socket(AF_PACKET, SOCK_RAW, htons(protocol));
    if (sock == QAPI_ERROR)
    {
        PRINTF("ERROR: Failed to create socket\n");
        goto end;
    }

    eth = (struct ethhdr *)pkt;

    /* DA */
    rc = hwaddr_pton(da, eth->h_dest, sizeof(eth->h_dest));  
    if (rc != QAPI_OK)
    {
        PRINTF("ERROR: Invalid MAC address\n");
        goto end;
    }

    /* SA */
    rc = qapi_Net_Interface_Get_Physical_Address(
                    (const char *)ifname,
                    (const uint8_t **)&srcmac,
                    &mac_addr_len);
    if (rc != QAPI_OK)
    {
        PRINTF("ERROR: Failed to get source MAC address\n");
        goto end;
    }
    memcpy(eth->h_source, srcmac, ETH_ALEN);

    /* EtherType */
    eth->h_proto = htons(protocol);

    /* socket address */
    memset(&foreign_addr, 0, sizeof(foreign_addr));
    foreign_addr.sll_family = AF_PACKET;
    foreign_addr.sll_protocol = htons(protocol);
    foreign_addr.sll_ifindex = rc = qapi_Net_Interface_Get_Ifindex(ifname);
    if (rc < 0)
    {
        PRINTF("ERROR: Failed to get ifIndex\n");
        goto end;
    }
    foreign_addr.sll_halen = ETH_ALEN;
    memcpy(foreign_addr.sll_addr, eth->h_dest, ETH_ALEN);

    /* send it */
    rc = qapi_sendto(sock, (char *)pkt, len, 0,
                     (struct sockaddr *)&foreign_addr, sizeof(struct sockaddr_ll));
    if (rc < 0)
    {
        PRINTF("ERROR: Failed to send (%d)\n", rc);
        goto end;
    }

    PRINTF("\nSent %d bytes to %s\n", rc, da); 
    HEXDUMP((char *)pkt, len, true, false);
    rc = QAPI_OK;

end:
    if (pkt && pkt != eapol)
    {
        free(pkt);
    }

    if (sock != QAPI_ERROR)
    {
        qapi_socketclose(sock);
    }

    if (rc != QAPI_OK)
    {
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

/************************************************************************
 *     [0] [1] [2]       [3]  [4]         [5]
 * eth rx  [-i <ifname>] [-p <etherType>] [-q]
 * eth rx  -i  wlan1     -p   0x888e
 * eth rx  -q
 ************************************************************************/
static
QCLI_Command_Status_t eth_rx(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    int i;
    struct sockaddr_ll local_addr;
    int32_t received;
    struct sockaddr_ll from_addr;
    int32_t fromlen;
    struct sockaddr *from;
    char *ifname = NULL;
    char *buf = NULL;
    int sock = QAPI_ERROR;
    int rc = QAPI_ERROR;
    /* default to receive EAPOL frame */
    uint16_t protocol = ETH_P_PAE;

    for (i = 1; i < Parameter_Count; i++)
    {
        if (Parameter_List[i].String_Value[0] == '-')
        {
            switch (Parameter_List[i].String_Value[1])
            {
                case 'i':   /* -i wlan1 */
                    i++;
                    ifname = Parameter_List[i].String_Value;
                    break;

                case 'p':   /* -p 0x888e */
                    i++;
                    if (!Parameter_List[i].Integer_Is_Valid ||
                        (protocol = Parameter_List[i].Integer_Value) < 0x600)
                    {
                        PRINTF("ERROR: Invalid etherType: %s\n", Parameter_List[i].String_Value);
                        goto end;
                    }
                    break;

                case 'q':   /* -q */
                    eth_rx_quit = true;
                    rc = QAPI_OK;
                    goto end;

                default:
                    PRINTF("ERROR: Unknown option: %s\n", Parameter_List[i].String_Value);
                    goto end;
            }
        }
        else
        {
            PRINTF("ERROR: Unknown option: %s\n", Parameter_List[i].String_Value);
            goto end;
        }

        if (i == Parameter_Count)
        {
            PRINTF("What is value of %s?\n", Parameter_List[i-1].String_Value);
            goto end;
        }
    }   /* for */

    buf = malloc(RX_BUFFER_SIZE);
    if (buf == NULL)
    {
        PRINTF("ERROR: No memory\n");
        goto end;
    }

    /* Open an socket */
    sock = qapi_socket(AF_PACKET, SOCK_RAW, (int)htons(protocol));
    if (sock == QAPI_ERROR)
    {
        PRINTF("ERROR: Failed to create socket\n");
        goto end;
    }

    /* Bind */
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sll_family = AF_PACKET;
    local_addr.sll_protocol = htons(protocol);
    if (ifname)
    {
        local_addr.sll_ifindex = rc = qapi_Net_Interface_Get_Ifindex(ifname);
        if (rc < 0)
        {
            PRINTF("ERROR: Failed to get ifIndex\n");
            goto end;
        }
    }

    rc = qapi_bind(sock, (struct sockaddr *)&local_addr, sizeof(struct sockaddr_ll));
    if (rc != QAPI_OK)
    {
        PRINTF("ERROR: Socket bind error.\n");
        goto end;
    }

    /* ------ Start test.----------- */
    PRINTF("****************************************************\n");
    PRINTF("Ethernet RX Test\n");
    PRINTF("****************************************************\n");
    PRINTF(" Bind interface: %s  ifIndex: %d\n", ifname ? ifname : "none", local_addr.sll_ifindex);
    PRINTF(" EtherType: 0x%04x\n", protocol);
    PRINTF("Type \"eth rx -q\" to termintate test.\n");
    PRINTF("****************************************************\n");

    memset(&from_addr, 0, sizeof(from_addr));
    from = (struct sockaddr *)&from_addr;
    fromlen = sizeof(struct sockaddr_ll);

    /* Receive loop */
    while (!eth_rx_quit)
    {
        fd_set rset;
        int conn_sock;

        rc = QAPI_OK;
        PRINTF("Waiting\n");

        do
        {
            if (eth_rx_quit)
            {
                goto end;
            }

            /* block for 500msec or until a packet is received */
            FD_ZERO(&rset);
            FD_SET(sock, &rset);

            conn_sock = qapi_select(&rset, NULL, NULL, 500);
            if (conn_sock == QAPI_ERROR)
            {
                rc = QAPI_ERROR;
                goto end;
            }
        } while (conn_sock == 0);

        /* Receive */
        received = qapi_recvfrom(sock, (char *)buf, RX_BUFFER_SIZE, 0, from, &fromlen);
        if (received > 0)
        {
            struct ethhdr *eth = (struct ethhdr *)buf;

            PRINTF("Received %d bytes from %02x:%02x:%02x:%02x:%02x:%02x\n", received,
                    eth->h_source[0],
                    eth->h_source[1],
                    eth->h_source[2],
                    eth->h_source[3],
                    eth->h_source[4],
                    eth->h_source[5]);
            HEXDUMP((char *)buf, received, true, false);
        }
        else
        {
            rc = QAPI_ERROR;
            PRINTF("ERROR: received %d bytes\n", received);
            break;
        }
        PRINTF("\n");
    } /* receive_loop */

end:
    if (buf)
    {
        free(buf);
    }

    if (sock != QAPI_ERROR)
    {
        qapi_socketclose(sock);
    }

    if (rc != QAPI_OK)
    {
        return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;
}

/************************************************************************
 eth tx  <ifname> <dest mac addr> [-p <etherType>] [-d <payload>]
 eth rx  [-i <ifname> -p <etherType>] [-q]
 ************************************************************************/
QCLI_Command_Status_t eth(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    QCLI_Command_Status_t status;

    if (Parameter_Count < 1)
    {
        eth_help();
        return QCLI_STATUS_SUCCESS_E;
    }

    eth_rx_quit = 0;

    if (strncmp(Parameter_List[0].String_Value, "tx", 1) == 0)
    {
        status = eth_tx(Parameter_Count, Parameter_List);
    }
    else
    if (strncmp(Parameter_List[0].String_Value, "rx", 1) == 0)
    {
        status = eth_rx(Parameter_Count, Parameter_List);
    }
    else
    {
        PRINTF("ERROR: Unknown command: %s\n", Parameter_List[0].String_Value);
        status = QCLI_STATUS_ERROR_E;
    }

    return status;
}
#ifdef ETHRAW_DEBUG 
#pragma GCC pop_options
#endif

#endif
