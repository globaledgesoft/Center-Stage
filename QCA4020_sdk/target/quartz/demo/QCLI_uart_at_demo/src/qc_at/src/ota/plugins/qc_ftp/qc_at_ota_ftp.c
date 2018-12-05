/* Copyright (c) 2011-2016.  Qualcomm Atheros, Inc.
 * All rights reserved.
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
#include "qapi/qapi_types.h"
#include "qapi/qapi_status.h"
#include "qapi/qapi_socket.h"
#include "qapi/qapi_netservices.h"
#include "qapi/qapi_firmware_upgrade.h"
#include "malloc.h"
#include "qc_at_ota_ftp.h"
#include "net_utils.h"

/*****************************************************************************************************************************/
/*                                                                                                                           */
/*       Firmware_Upgrade_FTP                                                                                                             */
/*                                                                                                                           */
/*****************************************************************************************************************************/
extern uint32_t generate_rand32(void);

#define htonl(l) (((((l) >> 24) & 0x000000ff)) | \
                 ((((l) >>  8) & 0x0000ff00)) | \
                 (((l) & 0x0000ff00) <<  8) | \
                 (((l) & 0x000000ff) << 24))
#define ntohl(l) htonl(l)
#define htons(s)    ((((s) >> 8) & 0xff) | (((s) << 8) & 0xff00))
#define ntohs(s) htons(s)

#define FW_UPGRADE_FTP_DATA_PORT                        1068
#define FW_UPGRADE_FTP_CMD_BUF_LEN                      1024
#define FW_UPGRADE_FTP_RECEIVE_TIMEOUT                 (30*1000)
#define FW_UPGRADE_FTP_RECEIVE_SHORT_TIMEOUT           (500)
#define FW_UPGRADE_FTP_TEN_MSEC                         (10)
#define FW_UPGRADE_FTP_CONNECTION_COUNT                 (200)

#undef DEBUG_FW_UPGRADE_FTP_PRINTF
#if defined(DEBUG_FW_UPGRADE_FTP_PRINTF)
#define FW_UPGRADE_FTP_D_PRINTF(args...) printf(args)
#else
#define FW_UPGRADE_FTP_D_PRINTF(args...)
#endif

#ifdef V1
#undef atoi
#define atoi(s)     ns_atoi((s))
extern int ns_atoi(char *str);

#undef strtol
#define strtol(s, e, b)     ns_atol((s))
extern long ns_atol(char *str);

#undef strrchr
#define strrchr(s, c)   ns_strrchr((s), (c))
extern char *ns_strrchr(const char *s, int c);

#undef strchr
#define strchr(s, c)    ns_strchr((s), (c))
extern char *ns_strchr(const char *s, int c);

#endif /*V1*/

/*************************************************************************************************************/
/*************************************************************************************************************/
typedef struct {
    int32_t data_sock;                /* data socket.*/
    int32_t peer_sock;
    int32_t control_sock;             /* control socket.*/
    uint32_t local_ip_addr;
    uint8_t local_v6addr[16];
    uint32_t remote_ip_addr;
    uint8_t  remote_v6addr[16];
    uint16_t cmd_port;
    uint16_t data_port;
    char *user;
    char *password;
    char *file;
    uint8_t v6_enable_flag;
    int32_t scope_id;
} QCOM_FTP_SESSION_INFO_t;

/*************************************************************************************************************/
/*************************************************************************************************************/
/*  FTP Global */
QCOM_FTP_SESSION_INFO_t *ftp_sess = NULL;

/*************************************************************************************************************/
/*************************************************************************************************************/
static int32_t fw_Upgrade_Ftp_Init(char *interface_name, char *url);
static int32_t fw_Upgrade_Ftp_Fin(void);
static uint16_t fw_Upgrade_Ftp_Get_Data_Port(void);
static int32_t fw_Upgrade_Ftp_Open_Control_Sock(void);
static int32_t fw_Upgrade_Ftp_Close_Control_Sock(void);
static int32_t fw_Upgrade_Ftp_Open_Data_Sock(uint16_t port);
static int32_t fw_Upgrade_Ftp_Close_Data_Sock(void);
static int32_t fw_Upgrade_Ftp_Close_Peer_Sock(void);
static int32_t fw_Upgrade_Ftp_Send_Cmd(char *cmd);
static int32_t fw_Upgrade_Ftp_Recv_Cmd(uint32_t tmo, int *resp_code);
static int32_t fw_Upgrade_Ftp_Send_Cmd_Resp(char *cmd, int *resp_code);
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Ftp_Connect_Server(const char* interface_name, const char *url, uint32_t offset);


/*************************************************************************************************************/
/*************************************************************************************************************/
/*
 *
 */
static uint16_t fw_Upgrade_Ftp_Get_Data_Port(void)
{
    uint16_t sock_port = generate_rand32() % 0x7fff;
    if( sock_port < FW_UPGRADE_FTP_DATA_PORT )
        sock_port += FW_UPGRADE_FTP_DATA_PORT;
    return sock_port;
}

/*
 *
 */
static int32_t fw_Upgrade_Ftp_Fin(void)
{
    /* close all sockets */
    fw_Upgrade_Ftp_Close_Peer_Sock();
    fw_Upgrade_Ftp_Close_Data_Sock();
    fw_Upgrade_Ftp_Close_Control_Sock();

    /* Clean up and free all resources */
    if(ftp_sess != NULL) {
        free(ftp_sess);
        ftp_sess = NULL;
    }

    return QAPI_FW_UPGRADE_OK_E;
}

/*
 * url format: <user>:<password>@<host>:<port>/<url-path>
 *         or  <user>:<password>@|ipv6|:<port>/<url-path>
 */
static int32_t fw_Upgrade_Ftp_Init(char *interface_name, char *url)
{
    uint32_t addr = 0, mask = 0, gw = 0;
    char  *ptr, *ptr_next;
    int family;
    int32_t rtn;

    FW_UPGRADE_FTP_D_PRINTF("FTP Init....\r\n");
    FW_UPGRADE_FTP_D_PRINTF("     url: %s\r\n", url);

    if(ftp_sess != NULL )
      return QCOM_FW_UPGRADE_ERR_FTP_SESSION_ALREADY_START_E;

    ftp_sess = malloc(sizeof(QCOM_FTP_SESSION_INFO_t));

    if (!ftp_sess) {
        return QCOM_FW_UPGRADE_ERR_FTP_NO_MEMORY_E;
    }
    memset(ftp_sess, '\0', sizeof(QCOM_FTP_SESSION_INFO_t));

    ftp_sess->control_sock = 0;
    ftp_sess->peer_sock = 0;
    ftp_sess->data_sock = 0;
    ftp_sess->local_ip_addr = 0;

    //parse url
    //get user name
    ptr = strtok(url, ":");
    if( ptr == NULL )
    {
        rtn = QCOM_FW_UPGRADE_ERR_FTP_URL_FORMAT_E;
        goto ftp_init_end;
    }
    ftp_sess->user = ptr;

    //get password
    ptr = strtok(NULL, "@");
    if( ptr == NULL )
    {
        rtn = QCOM_FW_UPGRADE_ERR_FTP_URL_FORMAT_E;
        goto ftp_init_end;
    }
    ftp_sess->password = ptr;

    ptr_next = ptr + strlen(ptr) + 1;

    if( *ptr_next == '|')   //IPV6
    {
        ip6_addr v6Global, v6GlobalExtd, v6LinkLocal, v6DefGw;
        uint32_t LinkPrefix, GlobalPrefix, DefGwPrefix, GlobalPrefixExtd;

        ftp_sess->v6_enable_flag = 1;
        family = AF_INET6;

        ptr_next++;
        //find next '|'
        ptr = strchr(ptr_next, '|');
        if( ptr == NULL )
        {
            rtn = QCOM_FW_UPGRADE_ERR_FTP_URL_FORMAT_E;
            goto ftp_init_end;
        }
        *ptr = '\0';

        /*Get IPv6 address of Peer*/
        if(inet_pton(AF_INET6, ptr_next, ftp_sess->remote_v6addr) != 0 )
        {
            rtn = QCOM_FW_UPGRADE_ERR_FTP_URL_FORMAT_E;
            goto ftp_init_end;
        }

        /* get local IPV6 address */
        memset(&v6LinkLocal,0, sizeof(ip6_addr));
        memset(&v6Global,0, sizeof(ip6_addr));
        memset(&v6DefGw,0, sizeof(ip6_addr));
        memset(&v6GlobalExtd,0, sizeof(ip6_addr));
        LinkPrefix = GlobalPrefix = DefGwPrefix = GlobalPrefixExtd = 0;
        if(qapi_Net_IPv6_Get_Address(interface_name,
                (uint8_t *)&v6LinkLocal,
                (uint8_t *)&v6Global,
                (uint8_t *)&v6DefGw,
                (uint8_t *)&v6GlobalExtd,
                &LinkPrefix,
                &GlobalPrefix,
                &DefGwPrefix,
                &GlobalPrefixExtd) != 0)
        {
            rtn = QCOM_FW_UPGRADE_ERR_FTP_GET_LOCAL_ADDRESS_E;
            goto ftp_init_end;
        }

        if( QAPI_IS_IPV6_LINK_LOCAL(ftp_sess->remote_v6addr) )
        {
            //save local link as local ip
            memcpy(ftp_sess->local_v6addr, (uint8_t *)&v6LinkLocal, sizeof(ip6_addr));
            if( qapi_Net_IPv6_Get_Scope_ID(interface_name, &(ftp_sess->scope_id)) != 0 )
            {
                rtn = QCOM_FW_UPGRADE_ERR_FTP_GET_LOCAL_ADDRESS_E;
                goto ftp_init_end;
            }
        } else {
            //save global as local ip
            memcpy(ftp_sess->local_v6addr, (uint8_t *)&v6Global, sizeof(ip6_addr));
        }

        //move to next segment
        *ptr = '|';
        ptr = strtok(ptr, ":");
        if( ptr == NULL )
        {
            rtn = QCOM_FW_UPGRADE_ERR_FTP_URL_FORMAT_E;
            goto ftp_init_end;
        }
    } else {    /* IPV4 */
        ftp_sess->v6_enable_flag = 0;
        family = AF_INET;

        //get host ip address
        ptr = strtok(NULL, ":");
        if( ptr == NULL )
        {
            rtn = QCOM_FW_UPGRADE_ERR_FTP_URL_FORMAT_E;
            goto ftp_init_end;
        }

        if( inet_pton(AF_INET, ptr, (uint32_t *) &(ftp_sess->remote_ip_addr)) != 0 )
        {
            rtn = QCOM_FW_UPGRADE_ERR_FTP_URL_FORMAT_E;
            goto ftp_init_end;
        }

        //get local IP address
        rtn = qapi_Net_IPv4_Config(interface_name, QAPI_NET_IPV4CFG_QUERY_E, &addr, &mask, &gw);
        if (rtn != 0)
        {
            FW_UPGRADE_FTP_D_PRINTF("Getting interface address failed\r\n");
            rtn = QCOM_FW_UPGRADE_ERR_FTP_GET_LOCAL_ADDRESS_E;
            goto ftp_init_end;
        }
        ftp_sess->local_ip_addr = addr;
    }

    //get ftp port number
    ptr = strtok(NULL, "/");
    if( ptr == NULL )
    {
        rtn = QCOM_FW_UPGRADE_ERR_FTP_URL_FORMAT_E;
        goto ftp_init_end;
    }
    ftp_sess->cmd_port = strtol(ptr,NULL,10);

    //get file name
    ftp_sess->file = ptr + strlen(ptr) + 1;
    if( ftp_sess->file == NULL )
    {
        rtn = QCOM_FW_UPGRADE_ERR_FTP_URL_FORMAT_E;
        goto ftp_init_end;
    }

    //set data port
    ftp_sess->data_port = fw_Upgrade_Ftp_Get_Data_Port();

    //create control socket
    if((ftp_sess->control_sock = qapi_socket(family, SOCK_STREAM, 0)) == -1)
    {
        ftp_sess->control_sock = 0;
        FW_UPGRADE_FTP_D_PRINTF("ERROR: Unable to create ftp control socket\r\n");
        rtn = QCOM_FW_UPGRADE_ERR_FTP_CREATE_SOCKET_E;
        goto ftp_init_end;
    }

    //create data socket
    if((ftp_sess->data_sock = qapi_socket(family, SOCK_STREAM, 0)) == -1)
    {
        ftp_sess->data_sock = 0;
        FW_UPGRADE_FTP_D_PRINTF("ERROR: Unable to create ftp data socket\r\n");
        rtn = QCOM_FW_UPGRADE_ERR_FTP_CREATE_SOCKET_E;
        goto ftp_init_end;
    }

    FW_UPGRADE_FTP_D_PRINTF("  ip:%xH, port:%d\r\n", ftp_sess->remote_ip_addr, ftp_sess->cmd_port);
    FW_UPGRADE_FTP_D_PRINTF("  User:%s, Password:%s\r\n", ftp_sess->user, ftp_sess->password);
    FW_UPGRADE_FTP_D_PRINTF("  file:%s\r\n", ftp_sess->file);

    return QAPI_FW_UPGRADE_OK_E;
ftp_init_end:
    fw_Upgrade_Ftp_Fin();
    return rtn;
}

/*
 *
 */
static int32_t fw_Upgrade_Ftp_Close_Data_Sock(void)
{
    if( (ftp_sess != NULL) && (ftp_sess->data_sock != 0) )
    {
        qapi_socketclose(ftp_sess->data_sock);
        ftp_sess->data_sock = 0;
    }
    return QAPI_FW_UPGRADE_OK_E;
}

/*
 *
 */
static int32_t fw_Upgrade_Ftp_Open_Data_Sock(uint16_t port)
{
    struct sockaddr_in local_addr;
    struct sockaddr_in6 local_addr6;
    struct sockaddr *addr;
    uint32_t addrlen;
    int32_t rtn = QAPI_FW_UPGRADE_OK_E;

    if( ftp_sess == NULL )
    {
        return QCOM_FW_UPGRADE_ERR_FTP_SESSION_NOT_START_E;
    }

    if (ftp_sess->v6_enable_flag)
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

    FW_UPGRADE_FTP_D_PRINTF("connect ftp data port:%d\r\n", port);

    /* bind to local port.*/
    if(qapi_bind( ftp_sess->data_sock, addr, addrlen) == -1)
    {
        rtn = QCOM_FW_UPGRADE_ERR_FTP_BIND_FAIL_E;
    }

    return rtn;
}

/*
 *
 */
static int32_t fw_Upgrade_Ftp_Close_Control_Sock(void)
{
    FW_UPGRADE_FTP_D_PRINTF("close ftp control connection\r\n");

    if( ftp_sess == NULL )
    {
        return QCOM_FW_UPGRADE_ERR_FTP_SESSION_NOT_START_E;
    }

    if( ftp_sess->control_sock != 0 )
    {
        qapi_socketclose(ftp_sess->control_sock);
        ftp_sess->control_sock = 0;
    }
    return QAPI_FW_UPGRADE_OK_E;
}

/*
 *
 */
static int32_t fw_Upgrade_Ftp_Open_Control_Sock(void)
{
    struct sockaddr_in foreign_addr;
    struct sockaddr_in6 foreign_addr6;
    struct sockaddr *to;
    uint32_t tolen;
    int32_t rtn = QAPI_FW_UPGRADE_OK_E;

    if( ftp_sess == NULL )
    {
        return QCOM_FW_UPGRADE_ERR_FTP_SESSION_NOT_START_E;
    }

    FW_UPGRADE_FTP_D_PRINTF("FTP Connecting ip:%xh port:%d\r\n", ftp_sess->remote_ip_addr, ftp_sess->cmd_port);

    if(ftp_sess->v6_enable_flag)
    {
        memset(&foreign_addr6, 0, sizeof(foreign_addr6));
        memcpy(&foreign_addr6.sin_addr, ftp_sess->remote_v6addr, sizeof(foreign_addr6.sin_addr));
        foreign_addr6.sin_port     = htons(ftp_sess->cmd_port);;
        foreign_addr6.sin_family   = AF_INET6;
        foreign_addr6.sin_scope_id = ftp_sess->scope_id;

        to = (struct sockaddr *)&foreign_addr6;
        tolen = sizeof(foreign_addr6);
    } else {
        memset(&foreign_addr, 0, sizeof(foreign_addr));
        foreign_addr.sin_addr.s_addr = ftp_sess->remote_ip_addr;
        foreign_addr.sin_port = htons(ftp_sess->cmd_port);
        foreign_addr.sin_family  = AF_INET;

        to = (struct sockaddr *)&foreign_addr;
        tolen = sizeof(foreign_addr);
    }

    /* Connect to the server.*/
    if(qapi_connect(ftp_sess->control_sock, to, tolen) == -1)
    {
        rtn = QCOM_FW_UPGRADE_ERR_FTP_CONNECT_FAIL_E;
    }

    return rtn;
}

/*
 *
 */
static int32_t fw_Upgrade_Ftp_Close_Peer_Sock(void)
{
    if( (ftp_sess != NULL) && (ftp_sess->peer_sock != 0) )
    {
        qapi_socketclose(ftp_sess->peer_sock);
        ftp_sess->peer_sock = 0;
    }
    return QAPI_FW_UPGRADE_OK_E;
}

/*
 * return: send length
 */
static int32_t fw_Upgrade_Ftp_Send_Cmd(char *cmd)
{
    if( ftp_sess == NULL )
    {
        return QCOM_FW_UPGRADE_ERR_FTP_SESSION_NOT_START_E;
    }

    FW_UPGRADE_FTP_D_PRINTF("send: %s", cmd);
    if ( ftp_sess->control_sock != 0)
    {
        if (qapi_send(ftp_sess->control_sock, cmd, strlen(cmd),0) <= 0)
            return QCOM_FW_UPGRADE_ERR_FTP_SEND_COMMAND_E;
        else
            return QAPI_FW_UPGRADE_OK_E;
    }
    return QCOM_FW_UPGRADE_ERR_FTP_CONTROL_SOCKET_CLOSED_E;
}

/*
 *
 */
static int32_t fw_Upgrade_Ftp_Recv_Cmd(uint32_t to, int *resp_code)
{
    fd_set sockSet,master;
    int32_t conn_sock, received;
    int32_t rtn = QAPI_FW_UPGRADE_OK_E;
    char *buffer;

    *resp_code = 0;
    if( ftp_sess == NULL )
    {
        return QCOM_FW_UPGRADE_ERR_FTP_SESSION_NOT_START_E;
    }

    buffer = malloc(FW_UPGRADE_FTP_CMD_BUF_LEN);
    if (buffer == NULL)
    {
        return QCOM_FW_UPGRADE_ERR_FTP_NO_MEMORY_E;
    }

    qapi_fd_zero(&master);
    qapi_fd_set(ftp_sess->control_sock, &master);
    if (qapi_fd_isset(ftp_sess->control_sock, &master) == 0)
    {
        rtn = QCOM_FW_UPGRADE_ERR_FTP_PEER_CLOSED_E;
        goto ftp_recv_cmd_end;
    }

    while(1) /* Receiving data.*/
    {
        sockSet = master;        /* Wait for Input */
        conn_sock = qapi_select(&sockSet, NULL, NULL, to);
        if(conn_sock != 0)
        {
            if(qapi_fd_isset( ftp_sess->control_sock,&sockSet) )
            {
                /*Packet is available, receive it*/
                received = qapi_recv( ftp_sess->control_sock, (char*)(&buffer[0]), FW_UPGRADE_FTP_CMD_BUF_LEN, 0);
                if( received <= 0 )
                {
                    /* Test ended, peer closed connection*/
                    rtn = QCOM_FW_UPGRADE_ERR_FTP_PEER_CLOSED_E;
                }
                break;
            } else {
	        /* Test ended, peer closed connection*/
	        rtn = QCOM_FW_UPGRADE_ERR_FTP_PEER_CLOSED_E;
                break;
            }
        } else {
             /* Test ended, peer closed connection*/
            rtn = QCOM_FW_UPGRADE_ERR_FTP_PEER_CLOSED_E;
            break;
        }
    }

    /*Valid packet received*/
    if (rtn == QAPI_FW_UPGRADE_OK_E)
    {
        buffer[received] = '\0';
        FW_UPGRADE_FTP_D_PRINTF("recv: %s", buffer);

        buffer[3] = '\0';
        *resp_code = atoi(buffer);
    }

ftp_recv_cmd_end:
    if( buffer != NULL )
    {
        free(buffer);
    }

    return rtn;
}

/*
 *
 */
static int32_t fw_Upgrade_Ftp_Send_Cmd_Resp(char *cmd, int *resp_code)
{
    uint32_t tmo = FW_UPGRADE_FTP_RECEIVE_TIMEOUT;
    int32_t rtn;

    *resp_code = 0;
    if( ftp_sess == NULL )
        return QCOM_FW_UPGRADE_ERR_FTP_SESSION_NOT_START_E;

    rtn = fw_Upgrade_Ftp_Send_Cmd(cmd);
    if( rtn != QAPI_FW_UPGRADE_OK_E )
        return rtn;

    return(fw_Upgrade_Ftp_Recv_Cmd(tmo, resp_code));
}


/*
 *
 */
/*
 * url format: <user>:<password>@<host>:<port>/<url-path>
 */
static qapi_Fw_Upgrade_Status_Code_t fw_Upgrade_Ftp_Connect_Server(const char* interface_name, const char *url, uint32_t offset)
{
    struct sockaddr_in foreign_addr;
    struct sockaddr_in6 foreign_addr6;
    struct sockaddr *from;
    int32_t fromlen;
    uint32_t len, count, tmo = FW_UPGRADE_FTP_RECEIVE_TIMEOUT;
    int32_t rtn = QAPI_FW_UPGRADE_OK_E;
    int resp;
    char *buf = NULL, *local_url = NULL;
    char *ptr1, *ptr2;
    char ip_str[48];

    //allocate buf for preparing command
    if( (buf = malloc(FW_UPGRADE_FTP_CMD_BUF_LEN)) == NULL )
    {
        rtn = QCOM_FW_UPGRADE_ERR_FTP_NO_MEMORY_E;
        goto ftp_init_end;
    }
    memset(buf, 0, FW_UPGRADE_FTP_CMD_BUF_LEN);

    //alloate buf to parse url
    len = strlen(url)+1;
    local_url = malloc(len);
    if (local_url == NULL)
    {
        rtn = QCOM_FW_UPGRADE_ERR_FTP_NO_MEMORY_E;
        goto ftp_init_end;
    }
    memset(local_url, 0, len);

    //copy url at local
    strncpy(local_url, url, len);

    //init ftp session
    if( (rtn = fw_Upgrade_Ftp_Init((char *)interface_name, local_url)) != QAPI_FW_UPGRADE_OK_E )
    {
        FW_UPGRADE_FTP_D_PRINTF("Error : fw_Upgrade_Ftp_Init error\r\n");
        goto ftp_init_end;
    }

    FW_UPGRADE_FTP_D_PRINTF("\r\nConnect to FTP Server...\r\n");
    //coonect to ftp server
    if( (rtn = fw_Upgrade_Ftp_Open_Control_Sock()) != QAPI_FW_UPGRADE_OK_E )
    {
        goto ftp_init_end;
    }

    //receive welcome
    if( (rtn = fw_Upgrade_Ftp_Recv_Cmd(tmo, &resp)) != QAPI_FW_UPGRADE_OK_E )
    {
    	goto ftp_init_end;
    }
    if( resp != 220 )
    {
        FW_UPGRADE_FTP_D_PRINTF("FTP Welcome error: %d\r\n",resp);
        goto ftp_init_end;
    }

    {
        // To receive all welcome message
        tmo = FW_UPGRADE_FTP_RECEIVE_SHORT_TIMEOUT;
        while (fw_Upgrade_Ftp_Recv_Cmd(tmo, &resp) == QAPI_FW_UPGRADE_OK_E);
    }

    //send user
    snprintf(buf, FW_UPGRADE_FTP_CMD_BUF_LEN, "USER %s\r\n", ftp_sess->user);
    if( (rtn = fw_Upgrade_Ftp_Send_Cmd_Resp(buf,&resp)) != QAPI_FW_UPGRADE_OK_E )
    {
        goto ftp_init_end;
    }
    if( resp != 331 )
    {
        rtn = QCOM_FW_UPGRADE_ERR_FTP_LOGIN_INCORRECT_E;
        FW_UPGRADE_FTP_D_PRINTF("FTP USER error: %d",resp);
        goto ftp_init_end;
    }

    //send password
    snprintf(buf, FW_UPGRADE_FTP_CMD_BUF_LEN, "PASS %s\r\n", ftp_sess->password);
    if( (rtn = fw_Upgrade_Ftp_Send_Cmd_Resp(buf,&resp)) != QAPI_FW_UPGRADE_OK_E )
    {
        goto ftp_init_end;
    }
    if( resp != 230 )
    {
        rtn = QCOM_FW_UPGRADE_ERR_FTP_LOGIN_INCORRECT_E;
        FW_UPGRADE_FTP_D_PRINTF("FTP PASS error: %d\r\n",resp);
        goto ftp_init_end;
    }

    FW_UPGRADE_FTP_D_PRINTF("FTP Login ...\r\n");

    //send syst
    if( (rtn = fw_Upgrade_Ftp_Send_Cmd_Resp("SYST\r\n",&resp)) != QAPI_FW_UPGRADE_OK_E )
    {
    	goto ftp_init_end;
    }

    if( resp != 215 )
    {
        rtn = QCOM_FW_UPGRADE_ERR_FTP_SYST_E;
        FW_UPGRADE_FTP_D_PRINTF("FTP SYST error: %d\r\n",resp);
        goto ftp_init_end;
    }

    //setup BINARY mode
    if( (rtn = fw_Upgrade_Ftp_Send_Cmd_Resp("TYPE I\r\n",&resp)) != QAPI_FW_UPGRADE_OK_E )
    {
    	goto ftp_init_end;
    }
    if( resp != 200 )
    {
        rtn = QCOM_FW_UPGRADE_ERR_FTP_SET_TYPE_E;
        FW_UPGRADE_FTP_D_PRINTF("FTP TYPE error: %d\r\n",resp);
        goto ftp_init_end;
    }

    //open data socket
    if( (rtn = fw_Upgrade_Ftp_Open_Data_Sock(ftp_sess->data_port)) != QAPI_FW_UPGRADE_OK_E )
    {
        goto ftp_init_end;
    }

    //send CWD if need
    if( strchr(ftp_sess->file, '/') != NULL )
    {
        ptr2 = NULL;
        ptr1 = strtok(ftp_sess->file, "/");
        while(1)
        {
            snprintf(buf, FW_UPGRADE_FTP_CMD_BUF_LEN, "CWD %s\r\n", ptr1);
            if( (rtn = fw_Upgrade_Ftp_Send_Cmd_Resp(buf,&resp)) != QAPI_FW_UPGRADE_OK_E )
            {
                goto ftp_init_end;
            }
            if( resp != 250 )
            {
                rtn = QCOM_FW_UPGRADE_ERR_FTP_DIR_NOT_EXIST_E;
                FW_UPGRADE_FTP_D_PRINTF("FTP CWD error: %d\r\n",resp);
                goto ftp_init_end;
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

        ftp_sess->file = ptr1;
    }


    //send port
    if( ftp_sess->v6_enable_flag )
    {
        //IPV6
        snprintf(buf, FW_UPGRADE_FTP_CMD_BUF_LEN, "EPRT |2|%s|%d|\r\n",
                inet_ntop(AF_INET6, (void *) ftp_sess->local_v6addr, ip_str, sizeof(ip_str)),
                ftp_sess->data_port);
    } else {
        //IPV4
#if 0 //PORT
        snprintf(buf, FW_UPGRADE_FTP_CMD_BUF_LEN, "PORT %d,%d,%d,%d,%d,%d\r\n",
                (ftp_sess->local_ip_addr)&0xff,
                (ftp_sess->local_ip_addr)>>8 & 0xff,
                (ftp_sess->local_ip_addr)>>16 &0xff,
                (ftp_sess->local_ip_addr)>>24&0xff,
                ftp_sess->data_port/256,
                ftp_sess->data_port%256);
#else  //EPRT
        snprintf(buf, FW_UPGRADE_FTP_CMD_BUF_LEN, "EPRT |1|%s|%d|\r\n",
                inet_ntop(AF_INET, (void *) &(ftp_sess->local_ip_addr), ip_str, sizeof(ip_str)),
                ftp_sess->data_port);
#endif
    }

    if( (rtn = fw_Upgrade_Ftp_Send_Cmd_Resp(buf,&resp)) != QAPI_FW_UPGRADE_OK_E )
    {
    	goto ftp_init_end;
    }
    if( resp != 200 )
    {
        rtn = QCOM_FW_UPGRADE_ERR_FTP_SET_PORT_E;
        FW_UPGRADE_FTP_D_PRINTF("FTP PORT error: %d\r\n",resp);
        goto ftp_init_end;
    }

    //send "REST" if need
    if( offset > 0 )
    {
        snprintf(buf, FW_UPGRADE_FTP_CMD_BUF_LEN, "REST %ld\r\n", offset);
        if( (rtn = fw_Upgrade_Ftp_Send_Cmd_Resp(buf,&resp)) != QAPI_FW_UPGRADE_OK_E )
        {
        	goto ftp_init_end;
        }
        if( resp != 350 )
        {
            rtn = QCOM_FW_UPGRADE_ERR_FTP_RESTART_NOT_SUPPORT_E;
            FW_UPGRADE_FTP_D_PRINTF("FTP RETR error: %d\r\n",resp);
            goto ftp_init_end;
        }
    }

    //send get file
    snprintf(buf, FW_UPGRADE_FTP_CMD_BUF_LEN, "RETR %s\r\n", ftp_sess->file);
    if( (rtn = fw_Upgrade_Ftp_Send_Cmd(buf)) != QAPI_FW_UPGRADE_OK_E )
    {
    	goto ftp_init_end;
    }

    //accept data socket request from remote
    if( ftp_sess->v6_enable_flag )
    {
        from = (struct sockaddr *)&foreign_addr6;
        fromlen = sizeof(struct sockaddr_in6);
    }
    else
    {
        from = (struct sockaddr *)&foreign_addr;
        fromlen = sizeof(struct sockaddr_in);
    }

    /* for the first time, check if incoming socket is created */
    if ( ftp_sess->peer_sock == 0 )
    {
        /* set to non-blocking mode */
        qapi_setsockopt(ftp_sess->data_sock, SOL_SOCKET, SO_NBIO, NULL, 0);

        /* Listen. */
        if(qapi_listen( ftp_sess->data_sock, 1) == -1 )
        {
            rtn = QCOM_FW_UPGRADE_ERR_FTP_DATA_CONNECTION_TIMEOUT_E;
            goto ftp_init_end;
        }

        count = 0;
        do
        {
            /*Accept incoming connection*/
            if( (int)(ftp_sess->peer_sock = qapi_accept( ftp_sess->data_sock, from, &fromlen) ) != -1 )
            {
                break;
            }

            app_msec_delay(FW_UPGRADE_FTP_TEN_MSEC);
            count = count + 1;
        } while (count < FW_UPGRADE_FTP_CONNECTION_COUNT);  /* wait for 2 seconds */

        /* set back to blocking mode */
        qapi_setsockopt(ftp_sess->data_sock, SOL_SOCKET, SO_BIO, NULL, 0);

        /* no connect request */
        if( (int)ftp_sess->peer_sock == -1 )
        {
            rtn = QCOM_FW_UPGRADE_ERR_FTP_ACCEPT_DATA_CONNECT_E;
            ftp_sess->peer_sock = 0;
        }
    }

    if( (rtn = fw_Upgrade_Ftp_Recv_Cmd(tmo, &resp)) != QAPI_FW_UPGRADE_OK_E )
    {
    	goto ftp_init_end;
    }

    if( resp != 150 )
    {
        rtn = QCOM_FW_UPGRADE_ERR_FTP_FILE_NOT_FOUND_E;
        goto ftp_init_end;
    }

    rtn = QAPI_FW_UPGRADE_OK_E;
ftp_init_end:
    //send quit and receive goodbye
    if( rtn != QAPI_FW_UPGRADE_OK_E )
    {
        fw_Upgrade_Ftp_Send_Cmd_Resp("QUIT\r\n", &resp);
        fw_Upgrade_Ftp_Fin();
    }

    //free memory
    if( buf != NULL )
        free(buf);
    if(local_url != NULL )
        free(local_url);

    return (qapi_Fw_Upgrade_Status_Code_t)rtn;
}


/*
 *
 */
qapi_Fw_Upgrade_Status_Code_t plugin_Ftp_Recv_Data(uint8_t *buffer, uint32_t buf_len, uint32_t *ret_size)
{
    int32_t rtn = QAPI_FW_UPGRADE_OK_E;
    fd_set sockSet;
    int32_t conn_sock, received;
    uint32_t tmo;
    int resp;

    *ret_size = received = 0;

    if( ftp_sess == NULL )
    {
        return (qapi_Fw_Upgrade_Status_Code_t)QCOM_FW_UPGRADE_ERR_FTP_SESSION_NOT_START_E;
    }

    if( ftp_sess->data_sock == 0 )
    {
        return (qapi_Fw_Upgrade_Status_Code_t)QCOM_FW_UPGRADE_ERR_FTP_DATA_SOCK_CLOSED_E;
    }

    /* -----------------------------------------------------*/
    /* process FTP data                                   */
    /* -----------------------------------------------------*/
    while(1)
    {
        /*-----------------------------------------------------------*/
        /* Receiving data from FTP server.*/
        tmo = FW_UPGRADE_FTP_RECEIVE_TIMEOUT;
        qapi_fd_zero(&sockSet);
        qapi_fd_set(ftp_sess->peer_sock, &sockSet);
        conn_sock = qapi_select(&sockSet, NULL, NULL, tmo);
        if(conn_sock != 0)
        {
            /*Packet is available, receive it*/
            received = qapi_recv( ftp_sess->peer_sock, (char *)buffer, buf_len, 0);

            //FW_UPGRADE_FTP_D_PRINTF("recv %dB\n", received);
            if( received >  0 )
            {
                //do receive data from peer
                rtn = QAPI_FW_UPGRADE_OK_E;
                break;
            } else {
                /* peer closed connection*/
                fw_Upgrade_Ftp_Recv_Cmd(tmo, &resp);

                if( resp != 0 )
                {
                    //we have received the whole file
                    if( resp == 226 )     //recv 226 Transfer complete
                    {
                        rtn = QAPI_FW_UPGRADE_OK_E;
                        FW_UPGRADE_FTP_D_PRINTF("Transfer Complete received\r\n");
                    } else {
                        //get error
                        FW_UPGRADE_FTP_D_PRINTF("Transfer not Complete:%d\r\n", resp);
                        rtn = QCOM_FW_UPGRADE_ERR_FTP_FILE_NOT_COMPLETE_E;
                    }
                } else {
                    rtn = QCOM_FW_UPGRADE_ERR_FTP_PEER_CLOSED_E;
                }

                received = 0;
                break;
            }
        } else {            // receiving command
            fw_Upgrade_Ftp_Recv_Cmd(tmo, &resp);

            if( resp != 0 )
            {
                //we have received the whole file
                if( resp == 226 )     //recv 226 Transfer complete
                {
                    rtn = QAPI_FW_UPGRADE_OK_E;
                    FW_UPGRADE_FTP_D_PRINTF("Transfer Complete received\r\n");

                } else {
                    //get error
                    FW_UPGRADE_FTP_D_PRINTF("Transfer not Complete:%d\r\n", resp);
                    rtn = QCOM_FW_UPGRADE_ERR_FTP_FILE_NOT_COMPLETE_E;
                }
            }
            break;
        }
    }

    *ret_size = received;
    FW_UPGRADE_FTP_D_PRINTF("plugin_Ftp_Recv_Data: rtn=%d, reveived=%d\r\n", rtn, *ret_size);
    return (qapi_Fw_Upgrade_Status_Code_t) rtn;
}

/*
 * url format: <user>:<password>@<host>:<port>/<url-path>
 */
qapi_Fw_Upgrade_Status_Code_t plugin_Ftp_Init(const char* interface_name, const char *url, void *init_param)
{
	return fw_Upgrade_Ftp_Connect_Server(interface_name, url, 0);
}

/*
 *
 */
qapi_Fw_Upgrade_Status_Code_t plugin_Ftp_Fin(void)
{
    /*send quit and no need to receive goodbye */
    fw_Upgrade_Ftp_Send_Cmd("QUIT\r\n");
    app_msec_delay(FW_UPGRADE_FTP_TEN_MSEC);
    fw_Upgrade_Ftp_Fin();
    FW_UPGRADE_FTP_D_PRINTF("plugin_Ftp_Fin\r\n");
    return QAPI_FW_UPGRADE_OK_E;
}

/*
 *
 */
qapi_Fw_Upgrade_Status_Code_t plugin_Ftp_Abort(void)
{
    /*send abort and doesn't need to receive response here */
    fw_Upgrade_Ftp_Send_Cmd("ABOR\r\n");
    fw_Upgrade_Ftp_Close_Peer_Sock();
    return QAPI_FW_UPGRADE_OK_E;
}

/*
 *
 */
qapi_Fw_Upgrade_Status_Code_t plugin_Ftp_Resume(const char* interface_name, const char *url, uint32_t offset)
{
	return fw_Upgrade_Ftp_Connect_Server(interface_name, url, offset);
}
