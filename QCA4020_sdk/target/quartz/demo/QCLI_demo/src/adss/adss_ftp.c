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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "qurt_signal.h"
#include "qurt_mutex.h"
#include "qurt_thread.h"
#include "qapi/qapi_types.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_socket.h"
#include "qapi/qapi_netservices.h"
#include "qapi_i2s.h"
#include <qcli_api.h>

#include "malloc.h"
#include "adss_demo.h"
#include "adss_ftp.h"

#include "netutils.h"

/*****************************************************************************************************************************/
/*                                                                                                                           */
/*       ADSS_FTP                                                                                                             */
/*                                                                                                                           */
/*****************************************************************************************************************************/

/* ADSS FTP Global */
ADSS_FTP_SESSION_t *adss_ftp_session = NULL;
 
ADSS_RET_STATUS adss_playOnWifi_Init()
{
    adss_ftp_session = malloc(sizeof(ADSS_FTP_SESSION_t));

    if (!adss_ftp_session) {
        return ADSS_NO_MEMORY;
    }
    memset(adss_ftp_session, '\0', sizeof(ADSS_FTP_SESSION_t));
	
	qurt_signal_init(&adss_ftp_session->buf_signal);
	
	qurt_mutex_init(&adss_ftp_session->buf_empty_mutex);
	qurt_signal_init(&adss_ftp_session->buf_empty_signal);
	
	qurt_mutex_init(&adss_ftp_session->buf_data_mutex);
	qurt_signal_init(&adss_ftp_session->buf_data_signal);

	qurt_mutex_init(&adss_ftp_session->buf_free_link_mutex);

    return ADSS_SUCCESS;
}

static uint16_t adss_Ftp_Get_Data_Port(void)
{
    uint16_t sock_port = generate_rand32() % 0x7fff;
    if( sock_port < OTA_FTP_DATA_PORT )
        sock_port += OTA_FTP_DATA_PORT;
    return sock_port;
}

/*
 *
 */
ADSS_RET_STATUS adss_Ftp_Fin(void)
{
    /* close all sockets */
    adss_Ftp_Close_Data_Connect_Sock();
    adss_Ftp_Close_Data_Sock();
    adss_Ftp_Close_Control_Sock();

    /* Clean up and free all resources */
    if(adss_ftp_session != NULL) {
		
		qurt_signal_delete(&adss_ftp_session->buf_signal);
		qurt_signal_delete(&adss_ftp_session->buf_empty_signal);
		qurt_signal_delete(&adss_ftp_session->buf_data_signal);
	
		qurt_mutex_delete(&adss_ftp_session->buf_empty_mutex);	
		qurt_mutex_delete(&adss_ftp_session->buf_data_mutex);
		qurt_mutex_delete(&adss_ftp_session->buf_free_link_mutex);
		
        free(adss_ftp_session);
        adss_ftp_session = NULL;
    }

    return ADSS_SUCCESS;
}

/*
 * url format: <user>:<password>@<host>:<port>/<url-path>
 */
ADSS_RET_STATUS adss_Ftp_Init(char *interface_name, char *url)
{
    uint32_t addr = 0, mask = 0, gw = 0;
    char  *ptr, *ptr_next;
    int family;
    int32_t rtn;

    ADSS_FTP_DEBUG_PRINTF("FTP Init....\r\n");
    ADSS_FTP_DEBUG_PRINTF("url: %s\r\n", url);

    adss_ftp_session->control_sock = 0;
    adss_ftp_session->data_connect_sock = 0;
    adss_ftp_session->data_sock = 0;
    adss_ftp_session->local_ip_addr = 0;

    //parse url
    //get user name
    ptr = strchr(url, ':');
    if( ptr == NULL )
    {
        rtn = ADSS_ERR_FTP_URL_FORMAT;
        goto ftp_init_end;
    }
    strncpy(adss_ftp_session->user, url, ptr - url);
	adss_ftp_session->user[ptr - url] = '\0';

    //get password
	ptr++;
    ptr_next = strchr(ptr, '@');
    if( ptr_next == NULL )
    {
        rtn = ADSS_ERR_FTP_URL_FORMAT;
        goto ftp_init_end;
    }
    strncpy(adss_ftp_session->password, ptr, ptr_next - ptr);
	adss_ftp_session->password[ptr_next - ptr] = '\0';
	
    ptr_next ++;
	
	{    /* IPV4 */
        adss_ftp_session->v6_enable_flag = 0;
        family = AF_INET;

        //get host ip address
        ptr = strchr(ptr_next, ':');
        if( ptr == NULL )
        {
            rtn = ADSS_ERR_FTP_URL_FORMAT;
            goto ftp_init_end;
        }

		strncpy(adss_ftp_session->cmd_buf, ptr_next, ptr - ptr_next);
		adss_ftp_session->cmd_buf[ptr - ptr_next] = '\0';
        if( inet_pton(AF_INET, adss_ftp_session->cmd_buf, (uint32_t *) &(adss_ftp_session->remote_ip_addr)) != 0 )
        {
            rtn = ADSS_ERR_FTP_URL_FORMAT;
            goto ftp_init_end;
        }

        //get local IP address
        rtn = qapi_Net_IPv4_Config(interface_name, QAPI_NET_IPV4CFG_QUERY_E, &addr, &mask, &gw);
        if (rtn != 0)
        {
            ADSS_FTP_DEBUG_PRINTF("Getting interface address failed\r\n");
            rtn = ADSS_ERR_FTP_GET_LOCAL_ADDRESS;
            goto ftp_init_end;
        }
        adss_ftp_session->local_ip_addr = addr;
    }

    //get ftp port number
	ptr++;
    ptr_next = strchr(ptr, '/');
    if( ptr_next == NULL )
    {
        rtn = ADSS_ERR_FTP_URL_FORMAT;
        goto ftp_init_end;
    }
	strncpy(adss_ftp_session->cmd_buf, ptr, ptr_next - ptr);
	adss_ftp_session->cmd_buf[ptr_next - ptr] = '\0';
	
    adss_ftp_session->cmd_port = strtol(adss_ftp_session->cmd_buf,NULL,10);

    //get file name
	ptr_next++;
    strcpy(adss_ftp_session->file, ptr_next);
	
    if( adss_ftp_session->file[0] == '\0')
    {
        rtn = ADSS_ERR_FTP_URL_FORMAT;
        goto ftp_init_end;
    }

    //set data port
    adss_ftp_session->data_port = adss_Ftp_Get_Data_Port();

    //create control socket
    if((adss_ftp_session->control_sock = qapi_socket(family, SOCK_STREAM, 0)) == -1)
    {
        adss_ftp_session->control_sock = 0;
        ADSS_FTP_DEBUG_PRINTF("ERROR: Unable to create ftp control socket\r\n");
        rtn = ADSS_ERR_FTP_CREATE_SOCKET;
        goto ftp_init_end;
    }

    //create data socket
    if((adss_ftp_session->data_sock = qapi_socket(family, SOCK_STREAM, 0)) == -1)
    {
        adss_ftp_session->data_sock = 0;
        ADSS_FTP_DEBUG_PRINTF("ERROR: Unable to create ftp data socket\r\n");
        rtn = ADSS_ERR_FTP_CREATE_SOCKET;
        goto ftp_init_end;
    }

    ADSS_FTP_DEBUG_PRINTF("  ip:%xH, port:%d\r\n", adss_ftp_session->remote_ip_addr, adss_ftp_session->cmd_port);
    ADSS_FTP_DEBUG_PRINTF("  User:%s, Password:%s\r\n", adss_ftp_session->user, adss_ftp_session->password);
    ADSS_FTP_DEBUG_PRINTF("  file:%s\r\n", adss_ftp_session->file);

    return ADSS_SUCCESS;
ftp_init_end:
    adss_Ftp_Fin();
    return rtn;
}

/*
 *
 */
ADSS_RET_STATUS adss_Ftp_Close_Data_Sock(void)
{
    if( (adss_ftp_session != NULL) && (adss_ftp_session->data_sock != 0) )
    {
        qapi_socketclose(adss_ftp_session->data_sock);
        adss_ftp_session->data_sock = 0;
    }
    return ADSS_SUCCESS;
}

/*
 *
 */
ADSS_RET_STATUS adss_Ftp_Open_Data_Sock(uint16_t port)
{
    struct sockaddr_in local_addr;
    struct sockaddr_in6 local_addr6;
    struct sockaddr *addr;
    uint32_t addrlen;
    ADSS_RET_STATUS rtn = ADSS_SUCCESS;

    if (adss_ftp_session->v6_enable_flag)
    {
        memset(&local_addr6, 0, sizeof(local_addr6));
        local_addr6.sin_port = htons(port);
        local_addr6.sin_family = AF_INET6;

        addr = (struct sockaddr *)&local_addr6;
        addrlen = sizeof(struct sockaddr_in6);
    } else {
        memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin_port = htons(port);
        local_addr.sin_family = AF_INET;

        addr = (struct sockaddr *)&local_addr;
        addrlen = sizeof(struct sockaddr_in);
    }

    ADSS_FTP_DEBUG_PRINTF("connect ftp data port:%d\r\n", port);

    /* bind to local port.*/
    if(qapi_bind( adss_ftp_session->data_sock, addr, addrlen) == -1)
    {
        rtn = ADSS_ERR_FTP_BIND_FAIL;
    }

    return rtn;
}

/*
 *
 */
ADSS_RET_STATUS adss_Ftp_Close_Control_Sock(void)
{
    ADSS_FTP_DEBUG_PRINTF("close ftp control connection\r\n");

    if( adss_ftp_session == NULL )
    {
        return ADSS_ERR_FTP_SESSION_NOT_START;
    }

    if( adss_ftp_session->control_sock != 0 )
    {
        qapi_socketclose(adss_ftp_session->control_sock);
        adss_ftp_session->control_sock = 0;
    }
    return ADSS_SUCCESS;
}

/*
 *
 */
static ADSS_RET_STATUS adss_Ftp_Open_Control_Sock(void)
{
    struct sockaddr_in foreign_addr;
    struct sockaddr_in6 foreign_addr6;
    struct sockaddr *to;
    uint32_t tolen;
    ADSS_RET_STATUS rtn = ADSS_SUCCESS;

    ADSS_INFO_PRINTF("FTP Connecting ip:%xh port:%d\r\n", adss_ftp_session->remote_ip_addr, adss_ftp_session->cmd_port);

    if(adss_ftp_session->v6_enable_flag)
    {
        memset(&foreign_addr6, 0, sizeof(foreign_addr6));
        memcpy(&foreign_addr6.sin_addr, adss_ftp_session->remote_v6addr, sizeof(foreign_addr6.sin_addr));
        foreign_addr6.sin_port     = htons(adss_ftp_session->cmd_port);;
        foreign_addr6.sin_family   = AF_INET6;
        foreign_addr6.sin_scope_id = adss_ftp_session->scope_id;

        to = (struct sockaddr *)&foreign_addr6;
        tolen = sizeof(foreign_addr6);
    } else {
        memset(&foreign_addr, 0, sizeof(foreign_addr));
        foreign_addr.sin_addr.s_addr = adss_ftp_session->remote_ip_addr;
        foreign_addr.sin_port = htons(adss_ftp_session->cmd_port);
        foreign_addr.sin_family  = AF_INET;

        to = (struct sockaddr *)&foreign_addr;
        tolen = sizeof(foreign_addr);
    }

    /* Connect to the server.*/
    if(qapi_connect(adss_ftp_session->control_sock, to, tolen) == -1)
    {
        rtn = ADSS_ERR_FTP_CONNECT_FAIL;
    }

    return rtn;
}

/*
 *
 */
ADSS_RET_STATUS adss_Ftp_Close_Data_Connect_Sock(void)
{
    if( (adss_ftp_session != NULL) && (adss_ftp_session->data_connect_sock != 0) )
    {
        qapi_socketclose(adss_ftp_session->data_connect_sock);
        adss_ftp_session->data_connect_sock = 0;
    }
    return ADSS_SUCCESS;
}

/*
 * 
 */
ADSS_RET_STATUS adss_Ftp_Send_Cmd(char *cmd, char *params)
{
    ADSS_FTP_DEBUG_PRINTF("send: %s %s\n", cmd, params);
	snprintf(adss_ftp_session->cmd_buf, sizeof(adss_ftp_session->cmd_buf), "%s %s\r\n", cmd, params);
	
    if ( adss_ftp_session->control_sock == 0)
		return ADSS_ERR_FTP_CONTROL_SOCKET_CLOSED;
		
    if (qapi_send(adss_ftp_session->control_sock, adss_ftp_session->cmd_buf, strlen(adss_ftp_session->cmd_buf),0) <= 0)
        return ADSS_ERR_FTP_SEND_COMMAND;
	
    return ADSS_SUCCESS;
}

/*
 *
 */
ADSS_RET_STATUS adss_Ftp_Recv_Cmd(uint32_t to, int *resp_code)
{
    fd_set sockSet,master;
    int32_t conn_sock, received;
    ADSS_RET_STATUS rtn = ADSS_SUCCESS;

    *resp_code = 0;

    qapi_fd_zero(&master);
    qapi_fd_set(adss_ftp_session->control_sock, &master);
    if (qapi_fd_isset(adss_ftp_session->control_sock, &master) == 0)
    {
        return ADSS_ERR_FTP_PEER_CLOSED;
    }

    {
        sockSet = master;        /* Wait for Input */
        conn_sock = qapi_select(&sockSet, NULL, NULL, to);
		
        if (conn_sock == 0)
		{
			return ADSS_ERR_FTP_PEER_CLOSED;
		}
        if (qapi_fd_isset( adss_ftp_session->control_sock,&sockSet) == 0)
		{
	        return ADSS_ERR_FTP_PEER_CLOSED;
		}
        
		/*Packet is available, receive it*/
        received = qapi_recv( adss_ftp_session->control_sock, adss_ftp_session->resp_buf, sizeof(adss_ftp_session->resp_buf), 0);
        if( received <= 0 )
        {
            return ADSS_ERR_FTP_PEER_CLOSED;
        }
	}

    /*Valid packet received*/
    {
        adss_ftp_session->resp_buf[received] = '\0';
        ADSS_FTP_DEBUG_PRINTF("recv: %s", adss_ftp_session->resp_buf);

        adss_ftp_session->resp_buf[3] = '\0';
        *resp_code = atoi(adss_ftp_session->resp_buf);
    }

    return rtn;
}

/*
 *
 */
ADSS_RET_STATUS adss_Ftp_Send_Cmd_Resp(char *cmd, char *params, int *resp_code)
{
    uint32_t tmo = OTA_FTP_RECEIVE_TIMEOUT;
    ADSS_RET_STATUS rtn;

    *resp_code = 0;

    rtn = adss_Ftp_Send_Cmd(cmd, params);
    if( rtn != ADSS_SUCCESS )
        return rtn;

    return(adss_Ftp_Recv_Cmd(tmo, resp_code));
}


/*
 *
 */
 ADSS_RET_STATUS adss_Ftp_Login()
 {
    ADSS_RET_STATUS rtn = ADSS_SUCCESS;
    int resp;

    //send user
    if( (rtn = adss_Ftp_Send_Cmd_Resp("USER", adss_ftp_session->user, &resp)) != ADSS_SUCCESS )
    {
		return  rtn;
    }
    if( resp != 331 )
    {
        ADSS_FTP_DEBUG_PRINTF("FTP USER error: %d",resp);
        return ADSS_ERR_FTP_LOGIN_INCORRECT;
    }

    //send password
    if( (rtn = adss_Ftp_Send_Cmd_Resp("PASS", adss_ftp_session->password, &resp)) != ADSS_SUCCESS )
    {
		return rtn;
    }
	
    if( resp != 230 )
    {
        ADSS_FTP_DEBUG_PRINTF("FTP PASS error: %d\r\n",resp);
        return ADSS_ERR_FTP_LOGIN_INCORRECT;
    }

    ADSS_FTP_DEBUG_PRINTF("FTP Login ...\r\n");
	return ADSS_SUCCESS;
 }

ADSS_RET_STATUS adss_Ftp_Traverse_Dir()
{
    int resp;
    char *ptr1, *ptr2;
    ADSS_RET_STATUS rtn = ADSS_SUCCESS;

	//open data socket
    if( (rtn = adss_Ftp_Open_Data_Sock(adss_ftp_session->data_port)) != ADSS_SUCCESS )
    {
        return rtn;
    }

    //send CWD if need
    if( strchr(adss_ftp_session->file, '/') != NULL )
    {
        ptr2 = NULL;
        ptr1 = strtok(adss_ftp_session->file, "/");
        while(1)
        {
            if( (rtn = adss_Ftp_Send_Cmd_Resp("CWD", ptr1, &resp)) != ADSS_SUCCESS )
            {
                return rtn;
            }
            if( resp != 250 )
            {
                rtn = ADSS_ERR_FTP_DIR_NOT_EXIST;
                ADSS_FTP_DEBUG_PRINTF("FTP CWD error: %d\r\n",resp);
                return rtn;
            }
            if( ptr2 == NULL)
            {
                ptr1 = strtok(NULL, "/");
                ptr2 = strtok(NULL, "/");
            } else {
                ptr1 = ptr2;
                ptr2 = strtok(NULL, "/");
            }

            if( ptr2 == NULL )
            {
                break;
            }
        }

        strcpy(adss_ftp_session->file, ptr1);
    }
	return  ADSS_SUCCESS;
 }

ADSS_RET_STATUS adss_Ftp_Do_Connect()
{
    uint32_t tmo = OTA_FTP_RECEIVE_TIMEOUT;
    int resp;
    ADSS_RET_STATUS rtn = ADSS_SUCCESS;

	ADSS_FTP_DEBUG_PRINTF("\r\nConnect to FTP Server...\r\n");
    //coonect to ftp server
    if( (rtn = adss_Ftp_Open_Control_Sock()) != ADSS_SUCCESS )
    {
        return rtn;
    }

    //receive welcome
    if( (rtn = adss_Ftp_Recv_Cmd(tmo, &resp)) != ADSS_SUCCESS )
    {
        return rtn;
    }
    if( resp != 220 )
    {
        ADSS_FTP_DEBUG_PRINTF("FTP Welcome error: %d\r\n",resp);
        return ADSS_FAILURE;
    }

    // To receive all welcome message
    tmo = 500;
    while (adss_Ftp_Recv_Cmd(tmo, &resp) == ADSS_SUCCESS)
		;
	
	return ADSS_SUCCESS;
}
 
/*
 * url format: <user>:<password>@<host>:<port>/<url-path>
 */
ADSS_RET_STATUS adss_Ftp_Connect_Server(const char* interface_name, char *url, uint32_t offset)
{
    ADSS_RET_STATUS rtn = ADSS_SUCCESS;
    int resp;

    //init ftp session
    if( (rtn = adss_Ftp_Init((char *)interface_name, url)) != ADSS_SUCCESS )
    {
        ADSS_FTP_DEBUG_PRINTF("Error : adss_Ftp_Init error\r\n");
    return  rtn;
    }

	if ((rtn = adss_Ftp_Do_Connect()) != ADSS_SUCCESS)
	{
    return  rtn;
    }
	
	if ((rtn = adss_Ftp_Login()) != ADSS_SUCCESS)
	{
    return  rtn;
    }


//  open data socket
    if( (rtn = adss_Ftp_Open_Data_Sock(adss_ftp_session->data_port)) != ADSS_SUCCESS )
    {
        return rtn;
    }

    //send port
	{
		uint32_t  ipaddr = adss_ftp_session->local_ip_addr;
		uint16_t  port = adss_ftp_session->data_port;
        snprintf((char *)adss_ftp_session->resp_buf, 30, "%d,%d,%d,%d,%d,%d",
                (int)(ipaddr & 0xFF), (int)((ipaddr >> 8) & 0xFF), (int)((ipaddr >> 16) & 0xFF),(int)((ipaddr >> 24) & 0xFF), 
				(port >> 8) & 0xFF, port & 0xFF);
				  
		if( (rtn = adss_Ftp_Send_Cmd_Resp("PORT", adss_ftp_session->resp_buf, &resp)) != ADSS_SUCCESS )
		{
			return  rtn;
		}
    }

    if( resp != 200 )
    {
        rtn = ADSS_ERR_FTP_SET_PORT;
        ADSS_FTP_DEBUG_PRINTF("FTP PORT error: %d\r\n",resp);
    return  rtn;
    }

    //send "REST" if need
    if( offset > 0 )
    {
		char  buf[10];
        snprintf(buf, OTA_FTP_CMD_BUF_LEN, "%ld\r\n", offset);
        if( (rtn = adss_Ftp_Send_Cmd_Resp("REST", buf, &resp)) != ADSS_SUCCESS )
        {
			return rtn;
        }
        if( resp != 350 )
        {
            rtn = ADSS_ERR_FTP_RESTART_NOT_SUPPORT;
            ADSS_FTP_DEBUG_PRINTF("FTP RETR error: %d\r\n",resp);
			return  rtn;
        }
    }
    rtn = ADSS_SUCCESS;	
    return  rtn;
}


/*
 *
 */
 
/*
 *
 */
ADSS_RET_STATUS adss_Ftp_Setup_Data_connect()
{
    struct sockaddr_in foreign_addr;
    struct sockaddr_in6 foreign_addr6;
    struct sockaddr *from;
    int32_t fromlen;
    ADSS_RET_STATUS rtn = ADSS_SUCCESS;
    uint32_t count;

    if( adss_ftp_session->data_sock == 0 )
    {
        return ADSS_ERR_FTP_DATA_SOCK_CLOSED;
    }

    if( adss_ftp_session->v6_enable_flag )
    {
        from = (struct sockaddr *)&foreign_addr6;
        fromlen = sizeof(struct sockaddr_in6);
    }
    else
    {
        from = (struct sockaddr *)&foreign_addr;
        fromlen = sizeof(struct sockaddr_in);
    }
	
	if( (rtn = adss_Ftp_Send_Cmd("RETR", adss_ftp_session->file)) != ADSS_SUCCESS )
    {
        ADSS_FTP_DEBUG_PRINTF("FTP RETR error: %d\r\n", rtn);
		return  rtn;
    }

    /* for the first time, check if incoming socket is created */
    if ( adss_ftp_session->data_connect_sock == 0 )
    {
        /* set to non-blocking mode */
        qapi_setsockopt(adss_ftp_session->data_sock, SOL_SOCKET, SO_NBIO, NULL, 0);
        
        /* Listen. */
        ADSS_INFO_PRINTF("Listening data socket\r\n");
        if(qapi_listen( adss_ftp_session->data_sock, 1) == -1 )
        {
            ADSS_INFO_PRINTF("FTP incoming data connection timeout\r\n");
            rtn = ADSS_ERR_FTP_DATA_CONNECTION_TIMEOUT;
            return rtn;
        }

        count = 0;
        do
        {
            /*Accept incoming connection*/
            ADSS_INFO_PRINTF("Accept incoming data connection\r\n");
            if( (int)(adss_ftp_session->data_connect_sock = qapi_accept( adss_ftp_session->data_sock, from, &fromlen) ) != -1 )
            {
                break;
            }
    
            app_msec_delay(50);
            count = count + 1;
        } while (count < 2000);  /* wait for 20 seconds */

        /* set back to blocking mode */
        qapi_setsockopt(adss_ftp_session->data_sock, SOL_SOCKET, SO_BIO, NULL, 0);
        
        if( (int)adss_ftp_session->data_connect_sock == -1 )
        {
            rtn = ADSS_ERR_FTP_ACCEPT_DATA_CONNECT;
            adss_ftp_session->data_connect_sock = 0;
        }
    }

    ADSS_FTP_DEBUG_PRINTF("Ftp Setup Data Connection: rtn=%d\r\n", rtn);
    return rtn;
}
 
ADSS_RET_STATUS adss_Ftp_Recv_Data(uint8_t *buffer, uint32_t buf_len, uint32_t *ret_size)
{
    ADSS_RET_STATUS rtn = ADSS_SUCCESS;
    fd_set sockSet;
    int32_t conn_sock, received;
    uint32_t tmo;
	uint32_t rcv_total_len = 0;
	uint32_t need_rcv_len = buf_len;
    int resp;

    *ret_size = received = 0;

    if( adss_ftp_session->data_connect_sock == 0 )
    {
        return ADSS_ERR_FTP_DATA_SOCK_CLOSED;
    }
	
    /* -----------------------------------------------------*/
    /* process FTP data                                   */
    /* -----------------------------------------------------*/
    while (1)
    {
        /*-----------------------------------------------------------*/
        /* Receiving data from FTP server.*/
        tmo = OTA_FTP_RECEIVE_TIMEOUT;
		
        qapi_fd_zero(&sockSet);
        qapi_fd_set(adss_ftp_session->data_connect_sock, &sockSet);
        conn_sock = qapi_select(&sockSet, NULL, NULL, tmo);
		
        if (conn_sock != 0)
        {
            /*Packet is available, receive it*/
            received = qapi_recv( adss_ftp_session->data_connect_sock, (char *)buffer, need_rcv_len, 0);

            //ADSS_FTP_DEBUG_PRINTF("recv %dB\n", received);
            if( received >  0 )
            {
                //do receive data from peer
				need_rcv_len -= received;
				buffer += received;
				rcv_total_len += received;
				
				if (need_rcv_len == 0)
				{
					rtn = ADSS_SUCCESS;
					break;					
				}
            } 
			else 
			{
                /* peer closed connection*/
                adss_Ftp_Recv_Cmd(tmo, &resp);

                if( resp != 0 )
                {
                    //we have received the whole file
                    if( resp == 226 )     //recv 226 Transfer complete
                    {
                        rtn = ADSS_SUCCESS;
                        ADSS_FTP_DEBUG_PRINTF("Transfer Complete received\r\n");
                    } 
					else 
					{
                        //get error
                        ADSS_FTP_DEBUG_PRINTF("Transfer not Complete:%d\r\n", resp);
                        rtn = ADSS_ERR_FTP_FILE_NOT_COMPLETE;
                    }
                } 
				else 
				{
                    rtn = ADSS_ERR_FTP_PEER_CLOSED;
                }

                received = 0;
                break;
            }
        } 
		else 
		{            // receiving command
            adss_Ftp_Recv_Cmd(tmo, &resp);

            if( resp != 0 )
            {
                //we have received the whole file
                if( resp == 226 )     //recv 226 Transfer complete
                {
                    rtn = ADSS_SUCCESS;
                    ADSS_FTP_DEBUG_PRINTF("Transfer Complete received\r\n");
                } 
				else 
				{
                    //get error
                    ADSS_FTP_DEBUG_PRINTF("Transfer not Complete:%d\r\n", resp);
                    rtn = ADSS_ERR_FTP_FILE_NOT_COMPLETE;
                }
            }
            break;
        }
    }

    *ret_size = rcv_total_len;
    return rtn;
}

/*
 * url format: <user>:<password>@<host>:<port>/<url-path>
 */
 
void adss_mbox_ftp_dma_callback(void *hd, uint32_t status, void *param)
{
    send_count++;

	qurt_signal_set(&adss_ftp_session->adss_dma_cb_signal, ADSS_DMA_CALLBACK_SIG_MASK);
}

qapi_Status_t adss_Send_Speaker_Init(uint32_t buf_len, uint32_t pkt_count)
{
  qapi_I2S_Instance_e inst;
  qapi_Status_t  status;

  inst = QAPI_I2S_INSTANCE_001_E;

  i2s_port0_config.buf_Size = buf_len;
  i2s_port0_config.num_Tx_Desc = pkt_count;
  i2s_port0_config.num_Rx_Desc = 0;
  i2s_port0_config.freq = QAPI_I2S_FREQ_16_KHZ_E;	  // wifi throughput limitation

  status = qapi_I2S_Init(inst, &i2s_port0_config, (void **)&hdI2S);
  if ( QAPI_OK != status ) 
      return status;

  qapi_I2S_Open(hdI2S);

  status = qapi_I2S_Intr_Register (hdI2S, adss_mbox_ftp_dma_callback, 0);
  if ( QAPI_OK != status ) 
      return status; 
  
  return  QAPI_OK;
}

/*
 *  buffer management
 */
ADSS_RET_STATUS  init_buf_link(int size)
{
	Data_Buf_Link_t  *p;
	uint32_t   i;
	
    adss_ftp_session->m_pFreeLink = malloc(sizeof(Data_Buf_Link_t) * size);

    if (adss_ftp_session->m_pFreeLink == NULL) {
        return ADSS_NO_MEMORY;
    }

	p = adss_ftp_session->m_pFreeLink;
	p->next = NULL;
	p++;
	
	for (i=0; i < size - 1; i++, p++)
		release_buf_link(p);

	return ADSS_SUCCESS;
}

Data_Buf_Link_t  *get_buf_link()
{
	Data_Buf_Link_t  *p;
	
	qurt_mutex_lock(&adss_ftp_session->buf_free_link_mutex);	
	p = adss_ftp_session->m_pFreeLink;
	if (p)	
		adss_ftp_session->m_pFreeLink = p->next;	
	qurt_mutex_unlock(&adss_ftp_session->buf_free_link_mutex);	/* unlock */
	return p;
}

void release_buf_link(Data_Buf_Link_t *p)
{
	qurt_mutex_lock(&adss_ftp_session->buf_free_link_mutex);	
	p->next = adss_ftp_session->m_pFreeLink;
	adss_ftp_session->m_pFreeLink = p;	
	qurt_mutex_unlock(&adss_ftp_session->buf_free_link_mutex);	/* unlock */
} 

uint8_t *get_ftp_data_buf()
{
	uint32_t num = 0;
	uint32_t  signals;
	Data_Buf_Link_t  *p;
	uint8_t *pdata;
	
	do {
		qurt_mutex_lock(&adss_ftp_session->buf_data_mutex);
		if (adss_ftp_session->m_pDataLink != NULL)
		{
			p = adss_ftp_session->m_pDataLink;
			adss_ftp_session->m_pDataLink = p->next;
			num = 1;
		}	
		qurt_mutex_unlock(&adss_ftp_session->buf_data_mutex);	/* unlock */
		if (num != 0)
		{
			pdata = p->buf_ptr;
			release_buf_link(p);

			return pdata;			
		}
				
		signals = qurt_signal_wait(&adss_ftp_session->buf_data_signal, ADSS_DATA_BUF_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);	
	} while (signals & ADSS_DATA_BUF_AVAIL_SIG_MASK);

    return NULL;	
}

uint8_t *peek_get_ftp_data_buf()
{
	uint32_t num = 0;
	Data_Buf_Link_t  *p;
	uint8_t *pdata;
	
	do {
		qurt_mutex_lock(&adss_ftp_session->buf_data_mutex);
		if (adss_ftp_session->m_pDataLink != NULL)
		{
			p = adss_ftp_session->m_pDataLink;
			adss_ftp_session->m_pDataLink = p->next;
			num = 1;
		}	
		qurt_mutex_unlock(&adss_ftp_session->buf_data_mutex);	/* unlock */
		if (num != 0)
		{
			pdata = p->buf_ptr;
			release_buf_link(p);

			return pdata;			
		}				
	} while (0);
	
    return NULL;	
}

uint8_t *put_ftp_data_buf(uint8_t *pbuf)
{
	Data_Buf_Link_t  *p;

	p = get_buf_link();	
    p->buf_ptr = pbuf;
	
	p->next = NULL;			
	qurt_mutex_lock(&adss_ftp_session->buf_data_mutex);
	if (adss_ftp_session->m_pDataLink != NULL)
	{
		adss_ftp_session->m_pDataLink_Trail->next = p;
		adss_ftp_session->m_pDataLink_Trail = p;	
	}
	else 
	{
		adss_ftp_session->m_pDataLink = p;		
		adss_ftp_session->m_pDataLink_Trail = p;		
	}
		
	qurt_mutex_unlock(&adss_ftp_session->buf_data_mutex);	/* unlock */
				
	qurt_signal_set(&adss_ftp_session->buf_data_signal, ADSS_DATA_BUF_AVAIL_SIG_MASK);
	return  NULL;
}

uint8_t *get_ftp_empty_buf()
{
	uint32_t num = 0;
	uint32_t  signals;
	Data_Buf_Link_t  *p;
	uint8_t  *pempty;
	
	do {
		qurt_mutex_lock(&adss_ftp_session->buf_empty_mutex);
		if (adss_ftp_session->m_pEmptyLink != NULL)
		{
			p = adss_ftp_session->m_pEmptyLink;
			adss_ftp_session->m_pEmptyLink = p->next;
			num = 1;
		}	
		qurt_mutex_unlock(&adss_ftp_session->buf_empty_mutex);	/* unlock */
		if (num != 0)
		{
			pempty = p->buf_ptr;
			release_buf_link(p);
			return 	pempty;		
		}
				
		signals = qurt_signal_wait(&adss_ftp_session->buf_empty_signal, ADSS_EMPTY_BUF_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);	
	} while (signals & ADSS_EMPTY_BUF_AVAIL_SIG_MASK);

    return NULL;	
}

uint8_t *peek_get_ftp_empty_buf()
{
	uint32_t num = 0;
	Data_Buf_Link_t  *p;
	uint8_t  *pempty;
	
	do {
		qurt_mutex_lock(&adss_ftp_session->buf_empty_mutex);
		if (adss_ftp_session->m_pEmptyLink != NULL)
		{
			p = adss_ftp_session->m_pEmptyLink;
			adss_ftp_session->m_pEmptyLink = p->next;
			num = 1;
		}	
		qurt_mutex_unlock(&adss_ftp_session->buf_empty_mutex);	/* unlock */
		if (num != 0)
		{
			pempty = p->buf_ptr;
			release_buf_link(p);
			return 	pempty;		
		}			
	} while (0);
	
	return NULL;
}

void put_ftp_empty_buf(uint8_t *pbuf)
{
	Data_Buf_Link_t  *p;
	
    p = get_buf_link();	
	p->buf_ptr = pbuf;
	
	qurt_mutex_lock(&adss_ftp_session->buf_empty_mutex);
	if (adss_ftp_session->m_pEmptyLink != NULL)
	{
		p->next = adss_ftp_session->m_pEmptyLink;
	}
	else
	{
		p->next = NULL;
	}
	adss_ftp_session->m_pEmptyLink = p;
	
	qurt_mutex_unlock(&adss_ftp_session->buf_empty_mutex);	/* unlock */
		
	qurt_signal_set(&adss_ftp_session->buf_empty_signal, ADSS_EMPTY_BUF_AVAIL_SIG_MASK);	
}

void ftp_data_receive_task(void *param)
{
	uint32_t  	ret_size, buf_len;
	ADSS_RET_STATUS rtn;
	uint8_t     *pbuf;
    int resp;

	buf_len = *(uint32_t *)param;
/*
 *  set up data connection
 */
	rtn = adss_Ftp_Setup_Data_connect();
    if( rtn != ADSS_SUCCESS )
    {
        adss_Ftp_Send_Cmd_Resp("QUIT","", &resp);
        adss_Ftp_Fin();
		return;
    }

	rtn = adss_Ftp_Recv_Data((uint8_t *)&adss_ftp_session->wav_fmt, sizeof(adss_ftp_session->wav_fmt), &ret_size);
	
	do {
		pbuf = get_ftp_empty_buf();
		if (pbuf == NULL)
			break;
		
		rtn = adss_Ftp_Recv_Data(pbuf, buf_len, &ret_size);
		if (rtn != ADSS_SUCCESS)
			break;
		put_ftp_data_buf(pbuf);
	} while (1);
	
	adss_ftp_session->thread_id = 0;
	
	qurt_signal_set(&adss_ftp_session->adss_dma_cb_signal, ADSS_WAV_FILE_DL_DONE_SIG_MASK);

	qurt_thread_stop();
}

ADSS_RET_STATUS adss_Ftp_play_wifi(const char* interface_name, char *url, uint32_t buf_len, uint32_t pkt_count)
{
	uint32_t  	i;
    uint8_t   	**pbuf_v, *pbuf;
	uint32_t    sent_len;
    qapi_Status_t  status;
    int resp;
	uint32      signals;
	ADSS_RET_STATUS rtn;
	qurt_time_t  duration = 1000;
	
    ADSS_FTP_DEBUG_PRINTF("buf size:%d pkt count:%d\r\n", buf_len,  pkt_count);

	rtn = adss_playOnWifi_Init();
    if( rtn != ADSS_SUCCESS )
    {
		return  rtn;
    }
	
	rtn = init_buf_link(pkt_count);
    if( rtn != ADSS_SUCCESS )
    {
		return  rtn;
    }

	rtn = adss_Ftp_Connect_Server(interface_name, url, 0);
    if( rtn != ADSS_SUCCESS )
    {
        adss_Ftp_Send_Cmd_Resp("QUIT","", &resp);
        adss_Ftp_Fin();
		return  rtn;
    }

	qurt_thread_attr_init (&adss_ftp_session->attr);
	qurt_thread_attr_set_name (&adss_ftp_session->attr, FTP_THREAD_NAME);
	qurt_thread_attr_set_priority (&adss_ftp_session->attr, FTP_THREAD_PRIORITY);
	qurt_thread_attr_set_stack_size (&adss_ftp_session->attr, FTP_THREAD_STACK_SIZE);
	qurt_thread_create(&adss_ftp_session->thread_id, &adss_ftp_session->attr, ftp_data_receive_task,  &buf_len);
	
	pbuf_v = (uint8_t **)malloc(sizeof(uint8_t *) * pkt_count);
	if (pbuf_v == NULL)
		return ADSS_NO_MEMORY;
	
	status = adss_Send_Speaker_Init(buf_len, pkt_count);
    if (status != QAPI_OK)
	{
		ADSS_FTP_DEBUG_PRINTF("I2S Init Fail\r\n");
		return ADSS_FAILURE;
	}
	qurt_signal_init(&adss_ftp_session->adss_dma_cb_signal);
	
	for (i=0; i < pkt_count; i++)
	{
		status = qapi_I2S_Get_Buffer(&pbuf);
		put_ftp_empty_buf(pbuf);
	}

	for (i=0; i < pkt_count; i++)
	{
		pbuf_v[i] = get_ftp_data_buf();
	    ADSS_FTP_DEBUG_PRINTF("Data buffer: %p\r\n", pbuf_v[i]);
	}
	
	qapi_I2S_Send_Receive(hdI2S, pbuf_v, pkt_count, NULL, NULL, 0);

	do
	{
		signals = qurt_signal_wait(&adss_ftp_session->adss_dma_cb_signal, ADSS_DMA_WAV_DL_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);
		if (signals & ADSS_WAV_FILE_DL_DONE_SIG_MASK)
			break;
		
		status = qapi_I2S_Get_Buffer(&pbuf);
		put_ftp_empty_buf(pbuf);

		pbuf = peek_get_ftp_data_buf();

		if (pbuf != NULL)
		{
			status = qapi_I2S_Send_Data(hdI2S, pbuf, buf_len, &sent_len);	  			
		}
	} while (audio_echo_loop_flag);

	audio_echo_loop_flag = 0;
	
	qurt_signal_set(&adss_ftp_session->buf_empty_signal, ADSS_USR_TASK_DONE_SIG_MASK);
	while (adss_ftp_session->thread_id != 0)
	{
		qurt_thread_sleep (duration);		
	}

	qurt_signal_delete(&adss_ftp_session->adss_dma_cb_signal);
	
	adss_Ftp_Fin();

	qapi_I2S_Deinit (hdI2S);
	qurt_thread_stop();

	return 0;
}
