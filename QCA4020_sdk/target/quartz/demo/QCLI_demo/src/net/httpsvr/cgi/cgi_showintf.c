/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
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
#include <stdio.h>
#include <stdlib.h>     /* malloc */
#include <string.h>
#include "htmldata.h"
#include "qapi_socket.h"        /* AF_INET */
#include "qapi_netservices.h"   /* QAPI_NET_IPV4CFG_STATIC_IP_E */
#include "qapi_webs.h"
#include "qapi_httpsvr.h"
#include "netutils.h"

#ifdef CONFIG_NET_HTTPS_DEMO

static int32_t show_interface(void *hp, char *interface_name, qbool_t if_is_up)
{
    char buf[128], ip_str[20], *p;
    const uint8_t *physical_address;
    uint32_t physical_address_length;
    uint32_t addr, mask, gw;
    int i, len;

    snprintf(buf, sizeof(buf), "%s: %s<br>", interface_name, if_is_up ? "UP" : "DOWN");
    qapi_Net_Webs_Send_String(hp, buf);

    /* MAC */
    if (qapi_Net_Interface_Get_Physical_Address(
                    interface_name,
                    &physical_address,
                    &physical_address_length
                    ) == 0)
    {
        len = snprintf(buf, sizeof(buf), "&emsp;Phy Address: %02X", physical_address[0]);
        p = &buf[len];
        for (i = 1; i < physical_address_length; i++)
        {
            len = snprintf(p, sizeof(buf), ":%02X", physical_address[i]);
            p += len;
        }
        snprintf(p, sizeof(buf), "<br>");
    }
    qapi_Net_Webs_Send_String(hp, buf);

    /* IPv4 */
    if (qapi_Net_IPv4_Config(interface_name, QAPI_NET_IPV4CFG_QUERY_E, &addr, &mask, &gw) == 0)
    {
        p = buf;
        //addr = htonl(addr);
        len = snprintf(p, sizeof(buf), "&emsp;IPv4: %s<br>", inet_ntop(AF_INET, &addr, ip_str, sizeof(ip_str)));

        p += len;
        //mask = htonl(mask);
        len = snprintf(p, sizeof(buf), "&emsp;Subnet Mask: %s<br>", inet_ntop(AF_INET, &mask, ip_str, sizeof(ip_str)));

        p += len;
        //gw = htonl(gw);
        snprintf(p, sizeof(buf), "&emsp;Default Gateway: %s<br><br>", inet_ntop(AF_INET, &gw, ip_str, sizeof(ip_str)));
    }
    qapi_Net_Webs_Send_String(hp, buf);

    return 0;
}

int
cgi_setintf(void *hp, void *form, char **filetext)
{
    char interface_name[8] = {0};
    uint32_t interface_is_up;

    if (form != NULL)
    {
        qapi_Net_Webs_Get_Form_String(hp, "Intfname", interface_name, sizeof(interface_name));

        if (qapi_Net_Interface_Exist(interface_name, &interface_is_up) && !interface_is_up)
        {
            char buf[16];
            uint32_t addr, mask, gw;

            memset(buf, 0, sizeof(buf));
            qapi_Net_Webs_Get_Form_String(hp, "Ipv4addr", buf, sizeof(buf));
            inet_pton(AF_INET, buf, &addr);

            memset(buf, 0, sizeof(buf));
            qapi_Net_Webs_Get_Form_String(hp, "Subnetmask", buf, sizeof(buf));
            inet_pton(AF_INET, buf, &mask);

            memset(buf, 0, sizeof(buf));
            qapi_Net_Webs_Get_Form_String(hp, "Gateway", buf, sizeof(buf));
            inet_pton(AF_INET, buf, &gw);

            /* set it */
            if (qapi_Net_IPv4_Config(interface_name, QAPI_NET_IPV4CFG_STATIC_IP_E, &addr, &mask, &gw) == 0)
            {
                qapi_Net_Webs_Send_String(hp, "IP address is set!");
            }
            else
            {
                qapi_Net_Webs_Send_String(hp, "Failed to set IP address!");
            }
        }
        else    /* interface is up */
        {
            char buf[64];
            snprintf(buf, sizeof(buf), "Not allow to change IP address of %s", interface_name);
            qapi_Net_Webs_Send_String(hp, buf);
        }
    }

	return (FP_DONE); /* generic return */
}

/* cgi_showintf() - CGI routine.
Returns bitmask. */
/* The form has four fields:
 * - type="text" name="Intfname"
 * - type="text" name="Ipv4addr"
 * - type="text" name="Subnetmask"
 * - type="text" name="Gateway"
 */
int
cgi_showintf(void *hp, void *form, char **filetext)
{
    qapi_Net_Ifnameindex_t *p, *pp;
    int num, i;

    num = qapi_Net_Get_All_Ifnames(NULL);

    if (num > 0)
    {
        p = pp = malloc(num * sizeof(qapi_Net_Ifnameindex_t));
        if (p)
        {
            qapi_Net_Get_All_Ifnames(p);
            for (i = 0; i < num; ++i)
            {
                show_interface(hp, pp->interface_Name, pp->if_Is_Up);
                ++pp;
            }

            free(p);
        }
    }

    return (FP_DONE); /* generic return */
}

#endif
