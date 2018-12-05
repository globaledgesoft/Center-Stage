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

/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include "aws_iot_error.h"
#include "aws_iot_log.h"
#include "timer_platform.h"
#include "network_interface.h"
#include "qapi_status.h"
#include "qapi_ssl_cert.h"
#include "network_platform.h"


IOT_SSL_INST *ssl = NULL;

extern unsigned atoh(char * buf);
#define htons(s) ((((s) >> 8) & 0xff) | \
                 (((s) << 8) & 0xff00))

/* This is the value used for ssl read timeout */
#define IOT_SSL_READ_TIMEOUT 10

void _iot_tls_set_connect_params(Network *pNetwork, char *pRootCALocation, char *pDeviceCertLocation,
                                 char *pDevicePrivateKeyLocation, char *pDestinationURL,
                                 uint16_t destinationPort, uint32_t timeout_ms, bool ServerVerificationFlag) {
    pNetwork->tlsConnectParams.DestinationPort = destinationPort;
    pNetwork->tlsConnectParams.pDestinationURL = pDestinationURL;
    pNetwork->tlsConnectParams.pDeviceCertLocation = pDeviceCertLocation;
    pNetwork->tlsConnectParams.pDevicePrivateKeyLocation = pDevicePrivateKeyLocation;
    pNetwork->tlsConnectParams.pRootCALocation = pRootCALocation;
    pNetwork->tlsConnectParams.timeout_ms = timeout_ms;
    pNetwork->tlsConnectParams.ServerVerificationFlag = ServerVerificationFlag;    
}

IoT_Error_t iot_tls_init(Network *pNetwork, char *pRootCALocation, char *pDeviceCertLocation,
                         char *pDevicePrivateKeyLocation, char *pDestinationURL,
                         uint16_t destinationPort, uint32_t timeout_ms, bool ServerVerificationFlag) {

    _iot_tls_set_connect_params(pNetwork, pRootCALocation, pDeviceCertLocation, pDevicePrivateKeyLocation,
                                 pDestinationURL, destinationPort, timeout_ms, ServerVerificationFlag);

    pNetwork->connect = iot_tls_connect;
    pNetwork->read = iot_tls_read;
    pNetwork->write = iot_tls_write;
    pNetwork->disconnect = iot_tls_disconnect;
    pNetwork->isConnected = iot_tls_is_connected;
    pNetwork->destroy = iot_tls_destroy;

    pNetwork->tlsDataParams.flags = 0;

    return SUCCESS;
}

IoT_Error_t iot_tls_is_connected(Network *pNetwork) {
    /* Use this to add implementation which can check for physical layer disconnect */
    return NETWORK_PHYSICAL_LAYER_CONNECTED;
}

    

int isValidIpv4Address(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result;
}

int is_Inet6Pton(char * src, void * dst)
{
   char *   cp;      /* char after previous colon */
   uint16_t *      dest;    /* word pointer to dst */
   int            colons;  /* number of colons in src */
   int            words;   /* count of words written to dest */

   /* count the number of colons in the address */
   cp = src;
   colons = 0;
   while(*cp)
   {
      if(*cp++ == ':') colons++;
   }

   if(colons < 2 || colons > 7)
   {
      /* printf("must have 2-7 colons");*/
      return 1;
   }

   /* loop through address text, parseing 16-bit chunks */
   cp = src;
   dest = dst;
   words = 0;

   if(*cp == ':') /* leading colon has implied zero, e.g. "::1" */
   {
      *dest++ = 0;
      words++;
      cp++;       /* bump past leading colon, eg ":!" */
   }

   while(*cp > ' ')
   {
      if(words >= 8)
      {
         IOT_DEBUG("inet_pton: logic error?\n");
         return 1;
      }
      if(*cp == ':')   /* found a double-colon? */
      {
         int i;
         for(i = (8 - colons); i > 0; i--)
         {
            *dest++ = 0;   /* add zeros to dest address */
            words++;       /* count total words */
         }
         cp++;             /* bump past double colon */
         if(*cp <= ' ')    /* last colon was last char? */
         {
            *dest++ = 0;   /* add one final zero */
            words++;       /* count total words */
         }
      }
      else
      {
         uint16_t wordval;
         uint16_t temp;
         wordval = atoh(cp);
         temp = wordval;
         wordval = htons(temp);    /* get next 16 bit word */
         if((wordval == 0) && (*cp != '0'))  /* check format */
         {
            IOT_DEBUG("must be hex numbers or colons \n");
            return 1;
         }
         *dest++ = wordval;
         words++;       /* count total words set in dest */
         cp = strchr((char *)cp, ':');   /* find next colon */
         if(cp)                  /* bump past colon */
            cp++;
         else                 /* no more colons? */
            break;            /* done with parsing */
      }
   }
   if(words != 8)
   {
      IOT_DEBUG("too short - missing colon?\n");
      return 1;
   }
   return 0;
}


int
iot_resolve_address(char *server, struct iot_address_t *dst) {

    struct ip46addr ipaddr;
    int32_t e = -1;
    char ip_str[48];

    int ip_len = 0;
    char ip_buf[16] = {0};
    int domain = AF_INET6;

    memset(&ipaddr, 0, sizeof(ipaddr));
    memset(ip_str, 0, sizeof(ip_str));
    

    if (!is_Inet6Pton((char*)server, ip_buf)){
          ip_len = 16;
          ipaddr.type = AF_INET6;
    } else {
      domain = AF_INET;
      ip_len = sizeof(unsigned long);
      ipaddr.type = AF_INET;
    } 

    IOT_DEBUG("try to resolve %s\n", server);

    if ((ipaddr.type == AF_INET6) || (0 == isValidIpv4Address(server)))
    {
        IOT_DEBUG("address is already IP %s\n", server);
        memcpy(ip_str, server, sizeof(ip_str));
    }
    else
    {
    
        if  (!qapi_Net_DNSc_Is_Started())
        {
            qapi_Net_DNSc_Command(QAPI_NET_DNS_START_E);
        }


        if  (!qapi_Net_DNSc_Is_Started())
        {
            IOT_DEBUG("DNS client is not started yet.\n");
            return -1;
        }


        e = qapi_Net_DNSc_Reshost(server, &ipaddr);
        if (e)
        {
            IOT_DEBUG("Unable to resolve %s\n", server);
            return -1;
        }
        else
        {
            inet_ntop(ipaddr.type, &ipaddr.a, ip_str, sizeof(ip_str));
            IOT_DEBUG("\n%s --> %s\n", server, ip_str);
        }
    }


    if (domain== AF_INET){
      ip_len = sizeof(struct in_addr);
      inet_pton(AF_INET, ip_str, &(dst->addr.sin.sin_addr));

    } else {
      ip_len = sizeof(struct in6_addr);
      inet_pton(AF_INET6, ip_str, &(dst->addr.sin6.sin_addr));
    }
    dst->addr.sin.sin_family = domain;
    dst->size = ip_len;

    return ip_len;
}


IoT_Error_t iot_tls_connect(Network *pNetwork, TLSConnectParams *params) {

    uint32_t tolen;
    struct sockaddr * to;
    int32_t result;
    iot_address_t dst;
    int32_t status = FAILURE;

    ssl = malloc(sizeof(IOT_SSL_INST));

    if(ssl == NULL)
        return status;

    memset(ssl, 0, sizeof(IOT_SSL_INST));

    ssl->role = QAPI_NET_SSL_CLIENT_E;
    ssl->sslCtx = qapi_Net_SSL_Obj_New(ssl->role);

    if(ssl->sslCtx == QAPI_NET_SSL_INVALID_HANDLE) {
        goto error;
    }
   
    memset(&ssl->config, 0, sizeof(qapi_Net_SSL_Config_t));
    ssl->config_set = FALSE;

    pNetwork->tlsDataParams.ssl = ssl->ssl;
    pNetwork->tlsDataParams.sslCtx = ssl->sslCtx;
    pNetwork->tlsDataParams.server_fd = AWS_INVALID_SOCKET_FD;

    IOT_DEBUG("iot_tls_connect\n");

    if (iot_resolve_address(pNetwork->tlsConnectParams.pDestinationURL, &dst) < 0)
    {
        IOT_DEBUG("ERROR fail to resolve address\n");
        status = TCP_SETUP_ERROR;
        goto error;
    }

    dst.addr.sin.sin_port = htons(pNetwork->tlsConnectParams.DestinationPort);

    if (dst.addr.sin.sin_family == AF_INET)
    {
        tolen = sizeof(struct sockaddr_in);        
        to = ((struct sockaddr *)&dst.addr.sin);
    }
    else
    {
        tolen = sizeof(struct sockaddr_in6);
        to = ((struct sockaddr *)&dst.addr.sin6);
    }

    /* Create socket */
    if ((pNetwork->tlsDataParams.server_fd = qapi_socket(dst.addr.sin.sin_family, SOCK_STREAM, 0)) == -1)
    {
        IOT_DEBUG("ERROR: Unable to create socket\n");
        status = TCP_SETUP_ERROR;
        goto error;
    }

    /* Connect to the server.*/
    IOT_DEBUG("Connecting\n");
    if (qapi_connect( pNetwork->tlsDataParams.server_fd, to, tolen) == -1)
    {
        IOT_DEBUG("Connection failed.\n");
        status = SSL_CONNECTION_ERROR;
        goto error;
    }


    if (pNetwork->tlsDataParams.ssl == QAPI_NET_SSL_INVALID_HANDLE)
    {

        //Load the certificate
        if(qapi_Net_SSL_Cert_Load(pNetwork->tlsDataParams.sslCtx, QAPI_NET_SSL_CERTIFICATE_E, pNetwork->tlsConnectParams.pDeviceCertLocation) != QAPI_OK) {
            IOT_DEBUG("ERROR: failed to load the certificate \n");
            status = NETWORK_SSL_UNKNOWN_ERROR;
            goto error;
        }

        //Load CA list if present
        if(pNetwork->tlsConnectParams.pRootCALocation != NULL) {
            IOT_DEBUG("Loading root ca = %s \n", pNetwork->tlsConnectParams.pRootCALocation);
            if(qapi_Net_SSL_Cert_Load(pNetwork->tlsDataParams.sslCtx, QAPI_NET_SSL_CA_LIST_E, pNetwork->tlsConnectParams.pRootCALocation) != QAPI_OK) {
                IOT_DEBUG("ERROR: failed to load the ca list = %s \n", pNetwork->tlsConnectParams.pRootCALocation);
                status = NETWORK_SSL_UNKNOWN_ERROR;
                goto error;
            }

            ssl->config.verify.domain = TRUE;
            ssl->config.verify.time_Validity = TRUE;
            qapi_Net_SSL_Cipher_Add(&ssl->config, QAPI_NET_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256);
            ssl->config_set = TRUE;
        }

        // Create SSL connection object
        pNetwork->tlsDataParams.ssl = qapi_Net_SSL_Con_New(pNetwork->tlsDataParams.sslCtx, QAPI_NET_SSL_TLS_E);
        if (pNetwork->tlsDataParams.ssl == QAPI_NET_SSL_INVALID_HANDLE)
        {
            IOT_DEBUG("ERROR: Unable to create SSL context\n");
            status = NULL_VALUE_ERROR;
            goto error;
        }

        if(ssl->config_set == TRUE) {
            //configure SSL
            if(qapi_Net_SSL_Configure(pNetwork->tlsDataParams.ssl, &ssl->config) != QAPI_OK) {
                IOT_DEBUG("SSL configure failed \n");
                status = NETWORK_SSL_UNKNOWN_ERROR;
                goto error;
            }
        }
    }

    // Add socket handle to SSL connection
    result = qapi_Net_SSL_Fd_Set(pNetwork->tlsDataParams.ssl, pNetwork->tlsDataParams.server_fd);
    if (result < 0)
    {
        IOT_DEBUG("ERROR: Unable to add socket handle to SSL (%d)\n", result);
        status = NETWORK_SSL_UNKNOWN_ERROR;
        goto error;
    }

    // SSL handshake with server
    result = qapi_Net_SSL_Connect(pNetwork->tlsDataParams.ssl);
    if (result < 0)
    {
        if (result == QAPI_SSL_OK_HS)
        {
            /** The peer's SSL certificate is trusted, CN matches the host name, time is valid */
            IOT_DEBUG("The certificate is trusted\n");
        }
        else if (result == QAPI_ERR_SSL_CERT_CN)
        {
            /** The peer's SSL certificate is trusted, CN matches the host name, time is expired */
            IOT_DEBUG("ERROR: The certificate is expired\n");
            status = NETWORK_X509_ROOT_CRT_PARSE_ERROR;
            goto error;
        }
        else if (result == QAPI_ERR_SSL_CERT_TIME)
        {
            /** The peer's SSL certificate is trusted, CN does NOT match the host name, time is valid */
            IOT_DEBUG("ERROR: The certificate is trusted, but the host name is not valid\n");
            status = NETWORK_X509_ROOT_CRT_PARSE_ERROR;
            goto error;
        }
        else if (result == QAPI_ERR_SSL_CERT_NONE)
        {
            /** The peer's SSL certificate is trusted, CN does NOT match host name, time is expired */
            IOT_DEBUG("ERROR: The certificate is expired and the host name is not valid\n");
            status = NETWORK_X509_ROOT_CRT_PARSE_ERROR;
            goto error;
        }
        else
        {
            IOT_DEBUG("ERROR: SSL connect failed (%d)\n", result);
            status = NETWORK_SSL_CONNECT_TIMEOUT_ERROR;
            goto error;
        }
    }

    return SUCCESS;

error:

   /*
    * Need this, since the AWS core code has a bug where by
    * disconnect & destroy functions are not called on a 
    * reconnection failure. Hence there is memory leak and
    * whole system fails. These calls are redundant for now
    * to fix this issue.
    */
   if(iot_tls_disconnect(pNetwork) != SUCCESS)
       status = FAILURE;

   if(iot_tls_destroy(pNetwork) != SUCCESS)
       status = FAILURE;

   return status;
}

IoT_Error_t iot_tls_write(Network *pNetwork, unsigned char *pMsg, size_t len, Timer *timer, size_t *written_len) {

    size_t written_so_far;
    bool isErrorFlag = false;
    int frags, ret = 0;
    TLSDataParams *tlsDataParams = &(pNetwork->tlsDataParams);    

    for(written_so_far = 0, frags = 0;
        written_so_far < len && !has_timer_expired(timer); written_so_far += ret, frags++) {
        while(!has_timer_expired(timer) &&
            (ret = qapi_Net_SSL_Write(tlsDataParams->ssl, pMsg + written_so_far, len - written_so_far)) <= 0) {
            if(ret != (len - written_so_far)) {
                IOT_ERROR(" failed\n  ! qapi_Net_SSL_Write returned -0x%x\n\n", -ret);
                /*  All other negative return values indicate connection needs to be reset.
                 *  Will be caught in ping request so ignored here 
                 */
                isErrorFlag = true;
                break;
            }
        }
        if(isErrorFlag) {
            break;
        }
    }

    *written_len = written_so_far;

    if(isErrorFlag) {
    	return NETWORK_SSL_WRITE_ERROR;
    } else if(has_timer_expired(timer) && written_so_far != len) {
    	return NETWORK_SSL_WRITE_TIMEOUT_ERROR;
    }

    return SUCCESS;   
}

IoT_Error_t iot_tls_read(Network *pNetwork, unsigned char *pMsg, size_t len, Timer *timer, size_t *read_len) {

    size_t rxLen = 0;
    int ret = 0;
    fd_set rset;

    while (len > 0) {

        FD_ZERO(&rset);

        FD_SET(pNetwork->tlsDataParams.server_fd, &rset);

        if (qapi_select(&rset, NULL, NULL, 200) > 0)
        { 

            // This read will timeout after IOT_SSL_READ_TIMEOUT if there's no data to be read
            ret = qapi_Net_SSL_Read(pNetwork->tlsDataParams.ssl, (void *)pMsg, len);

            if (ret > 0) {
                rxLen += ret;
                pMsg += ret;
                len -= ret;
                
            } else if (ret < 0) {
                return NETWORK_SSL_READ_ERROR;
            }
        }
        // Evaluate timeout after the read to make sure read is done at least once
        if (has_timer_expired(timer)) {
           break;
        }        
    }

    if (len == 0) {
        *read_len = rxLen;
        return SUCCESS;
    }

    if (rxLen == 0) {
        return NETWORK_SSL_NOTHING_TO_READ;
    } else {
        return NETWORK_SSL_READ_TIMEOUT_ERROR;
    }

}

IoT_Error_t iot_tls_disconnect(Network *pNetwork) {

    int32_t status = SUCCESS;

    if (pNetwork->tlsDataParams.ssl != QAPI_NET_SSL_INVALID_HANDLE)
    {
        if(qapi_Net_SSL_Shutdown(pNetwork->tlsDataParams.ssl) != QAPI_OK)
            status = FAILURE;

        pNetwork->tlsDataParams.ssl = QAPI_NET_SSL_INVALID_HANDLE;
    }

    if(ssl->sslCtx != QAPI_NET_SSL_INVALID_HANDLE)
    {
        if(qapi_Net_SSL_Obj_Free(ssl->sslCtx) != QAPI_OK)
            status = FAILURE;
    }

    if(pNetwork->tlsDataParams.server_fd != AWS_INVALID_SOCKET_FD)
    {
        if(qapi_socketclose(pNetwork->tlsDataParams.server_fd) != QAPI_OK)
            status = FAILURE;
    }

    return status;
}

IoT_Error_t iot_tls_destroy(Network *pNetwork) {
    if(ssl != NULL) {
       free(ssl);
       ssl = NULL;
    }

    return SUCCESS;   
}

#ifdef __cplusplus
}
#endif
