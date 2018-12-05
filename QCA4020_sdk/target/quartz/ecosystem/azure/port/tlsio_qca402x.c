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

// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "qapi_status.h"
#include "qapi_types.h"
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/tlsio.h"
#include "tlsio_qca402x.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"
//#include "azure_c_shared_utility/crt_abstractions.h"
#include "qapi_socket.h"
#include "qapi_ns_utils.h"
#include "qapi_ssl.h"
#include "qapi_netservices.h"
#include "certs.h"


#define htons(s)    ((((s) >> 8) & 0xff) | (((s) << 8) & 0xff00))

typedef struct ssl_instance
{
    qapi_Net_SSL_Obj_Hdl_t sslCtx;
    qapi_Net_SSL_Con_Hdl_t ssl;
    qapi_Net_SSL_Config_t   config;
    uint8_t      config_set;
    qapi_Net_SSL_Role_t role;
} SSL_INSTANCE;

typedef enum TLSIO_STATE_TAG
{
    TLSIO_STATE_NOT_OPEN,
    TLSIO_STATE_OPEN,
    TLSIO_STATE_ERROR
} TLSIO_STATE;

typedef void* TlsContext;

typedef struct TLS_IO_INSTANCE_TAG
{
    ON_IO_OPEN_COMPLETE on_io_open_complete;
    void* on_io_open_complete_context;
    ON_BYTES_RECEIVED on_bytes_received;
    void* on_bytes_received_context;
    ON_IO_ERROR on_io_error;
    void* on_io_error_context;
    TLSIO_STATE tlsio_state;
    char* hostname;
    int port;
    char* certificate;
    int socket;
    TlsContext tls_context;
	struct ip46addr ipaddr;
	int x509_mode;
	char* x509_cert;

} TLS_IO_INSTANCE;

static int tlsio_qca402x_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* on_io_close_complete_context);

/*this function will clone an option given by name and value*/
static void* tlsio_qca402x_clone_option(const char* name, const void* value)
{
	/*Not supported*/
    return NULL;
}

static int add_ca_list(TLS_IO_INSTANCE* tls_io_instance)
{
	qapi_Status_t status;
	qapi_CA_Info_t ca_info;
	qapi_Net_SSL_Cert_Info_t cert_info;
	LogInfo("adding ca list\n");
	cert_info.cert_Type = QAPI_NET_SSL_PEM_CA_LIST_E;
	cert_info.info.pem_CA_List.ca_Cnt = 1;
	ca_info.ca_Buf = (uint8_t*)tls_io_instance->certificate;
    ca_info.ca_Size = strlen(tls_io_instance->certificate);

	cert_info.info.pem_CA_List.ca_Info[0] = &ca_info;
    status =
    qapi_Net_SSL_Cert_Store(&cert_info, (const char*)"ca_list.bin"); 

	return status;
	
}

/*this function destroys an option previously created*/
static void tlsio_qca402x_destroy_option(const char* name, const void* value)
{
    /*Not Supported*/
}



int tlsConnect(TLS_IO_INSTANCE* tls_io_instance)
{
	struct sockaddr_in foreign_addr;
//    struct sockaddr_in6 foreign_addr6; //To do: add support for IPv6
    struct sockaddr *to;
    uint32_t tolen;
	SSL_INSTANCE *ssl = tls_io_instance->tls_context;
	int family = AF_INET;
	int result = 0;
	
	/* Create socket */
    if ((tls_io_instance->socket = qapi_socket(family, SOCK_STREAM, 0)) == -1)
    {
        LogError("ERROR: Unable to create socket\n");
		return -1;
    }
    
    memset(&foreign_addr, 0, sizeof(foreign_addr));
    foreign_addr.sin_addr.s_addr    = tls_io_instance->ipaddr.a.addr4;
    foreign_addr.sin_port           = htons(tls_io_instance->port);
    foreign_addr.sin_family         = family;

    to = (struct sockaddr *)&foreign_addr;
    tolen = sizeof(foreign_addr);
    
    /* Connect to the server.*/
    LogInfo("Connecting\n");
	if((result = qapi_connect( tls_io_instance->socket, to, tolen)) != 0)
	{
		LogError("ERROR: Unable to connect to server\n");
		goto ERROR;
    } else 
	{
	    if (ssl->ssl == QAPI_NET_SSL_INVALID_HANDLE)
        {
			if(tls_io_instance->x509_mode == 1){
				if((result = qapi_Net_SSL_Cert_Load(ssl->sslCtx,QAPI_NET_SSL_CERTIFICATE_E, tls_io_instance->x509_cert) < 0)){
					LogError("ERROR: x509 cert load failed\n");
					goto ERROR;
				}
			}
              
			if((result = qapi_Net_SSL_Cert_Load(ssl->sslCtx,QAPI_NET_SSL_CA_LIST_E, (const char*)"ca_list.bin")) < 0){
				  LogError("ERROR: CA List load failed %d\n",result);
				  goto ERROR;
			}
			  
            // Create SSL connection object
            ;
            if ((ssl->ssl = qapi_Net_SSL_Con_New(ssl->sslCtx, QAPI_NET_SSL_TLS_E)) == QAPI_NET_SSL_INVALID_HANDLE)
            {
                LogError("ERROR: Unable to create SSL context\n");
                goto ERROR;
            }			
            // configure the SSL connection
            if (ssl->config_set)
            {
                result = qapi_Net_SSL_Configure(ssl->ssl, &ssl->config);
                if (result < QAPI_OK)
                {
                    LogError("ERROR: SSL configure failed (%d)\n", result);
                    goto ERROR;
                }
            }
        }

        // Add socket handle to SSL connection
        result = qapi_Net_SSL_Fd_Set(ssl->ssl, tls_io_instance->socket);
        if (result < 0)
        {
            LogError("ERROR: Unable to add socket handle to SSL (%d)\n", result);
            goto ERROR;
        }

        // SSL handshake with server
        result = qapi_Net_SSL_Connect(ssl->ssl);
        if (result < 0)
        {
            if (result == QAPI_SSL_OK_HS)
            {
                /** The peer's SSL certificate is trusted, CN matches the host name, time is valid */
                LogError("The certificate is trusted\n");
				result = 0;
            }
            else if (result == QAPI_ERR_SSL_CERT_CN)
            {
                /** The peer's SSL certificate is trusted, CN matches the host name, time is expired */
                LogError("ERROR: The certificate is expired\n");
                goto ERROR;
            }
            else if (result == QAPI_ERR_SSL_CERT_TIME)
            {
                /** The peer's SSL certificate is trusted, CN does NOT match the host name, time is valid */
                LogError("ERROR: The certificate is trusted, but the host name is not valid\n");
                goto ERROR;
            }
            else if (result == QAPI_ERR_SSL_CERT_NONE)
            {
                /** The peer's SSL certificate is trusted, CN does NOT match host name, time is expired */
                LogError( "ERROR: The certificate is expired and the host name is not valid\n");
                goto ERROR;
            }
            else
            {
                LogError( "ERROR: SSL connect failed (%d)\n", result);
                goto ERROR;
            }
        }
    }
ERROR:
	if(result != 0){
		if (ssl->ssl != QAPI_NET_SSL_INVALID_HANDLE)
		{
			qapi_Net_SSL_Shutdown(ssl->ssl);
			ssl->ssl = QAPI_NET_SSL_INVALID_HANDLE;

		}
		qapi_socketclose(tls_io_instance->socket);
	}
	return result;
}

int resolve_host(TLS_IO_INSTANCE* tls_io_instance)
{
	struct ip46addr ipaddr;
	char	*svr;
	int result = 0;


    if  (!qapi_Net_DNSc_Is_Started())
    {
        LogError("DNS client is not started yet.\n");
        return __FAILURE__;
    }

    svr = tls_io_instance->hostname;
    memset(&ipaddr, 0, sizeof(ipaddr));
        
    ipaddr.type = AF_INET;
        
    //ipaddr.type = AF_INET6;

	if ((result = qapi_Net_DNSc_Reshost(svr, &ipaddr)) != 0)
	{
		LogError("Unable to resolve %s\n", svr);
	}
	else
	{
		tls_io_instance->ipaddr = ipaddr;
	}
	return result;
}

static CONCRETE_IO_HANDLE tlsio_qca402x_create(void* io_create_parameters)
{
    TLS_IO_INSTANCE* result;
	SSL_INSTANCE *ssl;

    /* check whether the argument is good */
    if (io_create_parameters == NULL)
    {
        result = NULL;
        LogError("NULL tls_io_config.");
    }
    else
    {
        TLSIO_CONFIG* tls_io_config = io_create_parameters;
 
        /* check if the hostname is good */
        if (tls_io_config->hostname == NULL)
        {
            result = NULL;
            LogError("NULL hostname in the TLS IO configuration.");
        }
        else
        {
            /* allocate */
            result = malloc(sizeof(TLS_IO_INSTANCE));
            if (result == NULL)
            {
                LogError("Failed allocating TLSIO instance.");
            }
            else
            {
				memset(result, 0, sizeof(TLS_IO_INSTANCE));
                /* copy the hostname for later use in open */
                if (mallocAndStrcpy_s(&result->hostname, tls_io_config->hostname) != 0)
                {
                    LogError("Failed to copy the hostname.");
                    free(result);
                    result = NULL;
                }
                else
                {					
					resolve_host(result);
                    /* copy port and initialize all the callback data */
                    result->port = tls_io_config->port;
                    result->certificate = NULL;
                    result->on_bytes_received = NULL;
                    result->on_bytes_received_context = NULL;
                    result->on_io_open_complete = NULL;
                    result->on_io_open_complete_context = NULL;
                    result->on_io_error = NULL;
                    result->on_io_error_context = NULL;
                    result->tlsio_state = TLSIO_STATE_NOT_OPEN;

					ssl = malloc(sizeof(SSL_INSTANCE)); 
    
					if(ssl == NULL) {
						free(result->hostname);
                        free(result);
						result = NULL;
						goto ERROR;
					}
					memset(ssl, 0, sizeof(SSL_INSTANCE)); 
     
					ssl->role = QAPI_NET_SSL_CLIENT_E; 
					ssl->sslCtx = qapi_Net_SSL_Obj_New(ssl->role); 

					if (ssl->sslCtx == QAPI_NET_SSL_INVALID_HANDLE)
					{
						LogError("ERROR: Unable to create SSL context\n");
						free(ssl);
						free(result->hostname);
						free(result);
						result = NULL;
						goto ERROR;
					}

					// Reset config struct
					memset(&ssl->config, 0, sizeof(qapi_Net_SSL_Config_t));
					
					/*Configure time validation*/
					ssl->config_set = 1;
					ssl->config.verify.time_Validity = 1;
					
                    result->tls_context = ssl;
                   
                }
            }
        }
    }
ERROR:
    return result;
}

static void tlsio_qca402x_destroy(CONCRETE_IO_HANDLE tls_io)
{
    if (tls_io == NULL)
    {
        LogError("NULL tls_io.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
		SSL_INSTANCE* ssl = tls_io_instance->tls_context;
		
		if (ssl->sslCtx)
		{
			qapi_Net_SSL_Obj_Free(ssl->sslCtx);
			ssl->sslCtx = QAPI_NET_SSL_INVALID_HANDLE;
		}
		
        /* force a close when destroying */
        tlsio_qca402x_close(tls_io, NULL, NULL);

        if (tls_io_instance->certificate != NULL)
        {
            free(tls_io_instance->certificate);
        }
		if (tls_io_instance->x509_cert != NULL)
        {
            free(tls_io_instance->x509_cert);
        }
        free(tls_io_instance->hostname);
        free(tls_io);
    }
}

static int tlsio_qca402x_open(CONCRETE_IO_HANDLE tls_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    int result;

    /* check arguments */
    if ((tls_io == NULL) ||
        (on_io_open_complete == NULL) ||
        (on_bytes_received == NULL) ||
        (on_io_error == NULL))
    {
        result = __FAILURE__;
        LogError("NULL tls_io.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (tls_io_instance->tlsio_state != TLSIO_STATE_NOT_OPEN)
        {
            result = __FAILURE__;
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_NOT_OPEN.");
        }
        else
        {
            unsigned char is_error = 0;
            
            tls_io_instance->on_bytes_received = on_bytes_received;
            tls_io_instance->on_bytes_received_context = on_bytes_received_context;

            tls_io_instance->on_io_error = on_io_error;
            tls_io_instance->on_io_error_context = on_io_error_context;

            tls_io_instance->on_io_open_complete = on_io_open_complete;
            
           

            if (is_error != 0)
            {
                result = __FAILURE__;
                LogError("Setting the trusted certificates failed");
            }
            else
            {                
                if (tlsConnect(tls_io_instance))
                {
                    LogError("tlsConnect failed");
                    result = __FAILURE__;
                }
                else
                {
                    tls_io_instance->tlsio_state = TLSIO_STATE_OPEN;
                    on_io_open_complete(on_io_open_complete_context, IO_OPEN_OK);

                    result = 0;
                }

            }
        }
    }

    return result;
}

static int tlsio_qca402x_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* on_io_close_complete_context)
{
    int result = 0;
	SSL_INSTANCE* ssl;

    if (tls_io == NULL)
    {
        result = __FAILURE__;
        LogError("NULL tls_io.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
			ssl = tls_io_instance->tls_context;

        /* If we're not open do not try to close */
        if (tls_io_instance->tlsio_state == TLSIO_STATE_NOT_OPEN)
        {
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_NOT_OPEN or TLSIO_STATE_CLOSING.");
            result = __FAILURE__;
        }
        else
        {
			if(ssl->ssl){
					LogError("SSL shutdown\r\n");
				if(qapi_Net_SSL_Shutdown(ssl->ssl))
				{
					LogError("Shutting down TLS connection failed\r\n");
					result = __FAILURE__;
				}
				else
				{
					qapi_socketclose(tls_io_instance->socket);
					tls_io_instance->socket = (int)NULL;
					tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;

					/* trigger the callback and return */
					if (on_io_close_complete != NULL)
					{
						on_io_close_complete(on_io_close_complete_context);
					}

					result = 0;
				}         
				ssl->ssl = QAPI_NET_SSL_INVALID_HANDLE;
			}
        }
    }

    return result;
}



static int tlsio_qca402x_send(CONCRETE_IO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* on_send_complete_context)
{
    int result;
	SSL_INSTANCE* ssl;	

    if ((tls_io == NULL) ||
        (buffer == NULL) ||
        (size == 0))
    {
        result = __FAILURE__;
        LogError("NULL tls_io.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
		ssl = tls_io_instance->tls_context;		
		
        /* If we are not open, do not try to send */
        if (tls_io_instance->tlsio_state != TLSIO_STATE_OPEN)
        {
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_OPEN.");
            result = __FAILURE__;
        }
        else
        {
			if(qapi_Net_SSL_Write(ssl->ssl, (void*)buffer, size) == 0)
            
            {
                LogError("TLS library failed to encrypt bytes.");
                result = __FAILURE__;
            }
            else
            {
                if (on_send_complete != NULL)
                {
                    on_send_complete(on_send_complete_context, IO_SEND_OK);
                }
                result = 0;
            }           
        }
    }

    return result;
}

static int tlsRead(TLS_IO_INSTANCE* tls_io_instance,unsigned char* buffer, int size, int* received)
{
	SSL_INSTANCE* ssl = tls_io_instance->tls_context;
	fd_set rset;
	int conn_sock;
	
	qapi_fd_zero(&rset);
	qapi_fd_set(tls_io_instance->socket, &rset);
    
	conn_sock = qapi_select(&rset, NULL, NULL, 500);
	
	if (conn_sock == 0) {
			/* No activity. Continue with the next session */
			*received = 0;
			return 0;
	}
	else {
		
		*received = qapi_Net_SSL_Read(ssl->ssl, buffer, size);
		return 0;			
		
	}
}

static void tlsio_qca402x_dowork(CONCRETE_IO_HANDLE tls_io)
{
    /* check arguments */
    if (tls_io == NULL)
    {
        LogError("NULL tls_io.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        /* only perform work if we are not in error */
        if ((tls_io_instance->tlsio_state != TLSIO_STATE_NOT_OPEN) &&
            (tls_io_instance->tlsio_state != TLSIO_STATE_ERROR))
        {
            unsigned char buffer[64];

            int received = 1;

			while(received > 0){
				if (tlsRead(tls_io_instance, buffer, sizeof(buffer), &received) != 0) 
				{
					LogError("Error received bytes");

					/* mark state as error and indicate it to the upper layer */
					tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
					tls_io_instance->on_io_error(tls_io_instance->on_io_error_context);
				}
				else
				{					
					if (received > 0)
					{
#ifdef DBG_EN						
						LogError("RX %d %s\n",received,buffer);
#endif						
						/* if bytes have been received indicate them */
						tls_io_instance->on_bytes_received(tls_io_instance->on_bytes_received_context, buffer, received);
					}else if (received < 0){
						LogError("ERROR: received bytes %d\n", received);
						/* mark state as error and indicate it to the upper layer */
						tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
						tls_io_instance->on_io_error(tls_io_instance->on_io_error_context);
					}
				}
			}          
        }
    }
}

static int tlsio_qca402x_setoption(CONCRETE_IO_HANDLE tls_io, const char* optionName, const void* value)
{
    int result;

    /* check arguments */
    if ((tls_io == NULL) || (optionName == NULL))
    {
        LogError("NULL tls_io");
        result = __FAILURE__;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
          
        if (strcmp("TrustedCerts", optionName) == 0)
        {
            const char* cert = (const char*)value;
             LogInfo("Setting trusted cert\n"); 
            if (tls_io_instance->certificate != NULL)
            {
                // Free the memory if it has been previously allocated
                free(tls_io_instance->certificate);
                tls_io_instance->certificate = NULL;
            }

            if (cert == NULL)
            {
                result = 0;
            }
            else
            {
                // Store the certificate
                if (mallocAndStrcpy_s(&tls_io_instance->certificate, cert) != 0)
                {
                    LogError("Error allocating memory for certificates");
                    result = __FAILURE__;
                }
                else
                {
                    result = 0;
					if (tls_io_instance->certificate != NULL)
					{
						if(add_ca_list(tls_io_instance) != 0){
							result = __FAILURE__;
						}
					}
                }
            }
        }
		else if (strcmp("x509", optionName) == 0)
        {	
			/*Store the name of x509 cert, it should have been stored before the client is inialized*/
	   	    if (mallocAndStrcpy_s(&tls_io_instance->x509_cert, (const char*)value) != 0)
			{
				LogError("Error allocating memory for certificates");
				result = __FAILURE__;
			}
			else
			{
				result = 0;
				tls_io_instance->x509_mode = 1;
			}

		}	
        else
        {
            LogError("Unrecognized option");
            result = __FAILURE__;
        }
    }

    return result;
}

static OPTIONHANDLER_HANDLE tlsio_qca402x_retrieve_options(CONCRETE_IO_HANDLE handle)
{
    OPTIONHANDLER_HANDLE result;

    /* Codes_SRS_tlsio_qca402x_01_064: [ If parameter handle is `NULL` then `tlsio_qca402x_retrieve_options` shall fail and return NULL. ]*/
    if (handle == NULL)
    {
        LogError("invalid parameter detected: CONCRETE_IO_HANDLE handle=%p", handle);
        result = NULL;
    }
    else
    {
        /* Codes_SRS_tlsio_qca402x_01_065: [ `tlsio_qca402x_retrieve_options` shall produce an OPTIONHANDLER_HANDLE. ]*/
        result = OptionHandler_Create(tlsio_qca402x_clone_option, tlsio_qca402x_destroy_option, tlsio_qca402x_setoption);
        if (result == NULL)
        {
            /* Codes_SRS_tlsio_qca402x_01_068: [ If producing the OPTIONHANDLER_HANDLE fails then tlsio_qca402x_retrieve_options shall fail and return NULL. ]*/
            LogError("unable to OptionHandler_Create");
            /*return as is*/
        }
        else
        {
            TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)handle;

            /* Codes_SRS_tlsio_qca402x_01_066: [ `tlsio_qca402x_retrieve_options` shall add to it the options: ]*/
            if (
                (tls_io_instance->certificate != NULL) &&
                /* TODO: This only handles TrustedCerts, if you need to handle more options specific to your TLS library, fill the code in here

                (OptionHandler_AddOption(result, "my_option", tls_io_instance->...) != 0) ||
                */
                (OptionHandler_AddOption(result, "TrustedCerts", tls_io_instance->certificate) != 0)
                )
            {
                LogError("unable to save TrustedCerts option");
                OptionHandler_Destroy(result);
                result = NULL;
            }
            else
            {
                /*all is fine, all interesting options have been saved*/
                /*return as is*/
            }
        }
    }
    return result;
}

static const IO_INTERFACE_DESCRIPTION tlsio_qca402x_interface_description =
{
    tlsio_qca402x_retrieve_options,
    tlsio_qca402x_create,
    tlsio_qca402x_destroy,
    tlsio_qca402x_open,
    tlsio_qca402x_close,
    tlsio_qca402x_send,
    tlsio_qca402x_dowork,
    tlsio_qca402x_setoption
};

/* This simply returns the concrete implementations for the TLS adapter */
const IO_INTERFACE_DESCRIPTION* tlsio_qca402x_get_interface_description(void)
{
    return &tlsio_qca402x_interface_description;
}
