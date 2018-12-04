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

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "qapi_status.h"
#include "bench.h"
#include "qapi_netservices.h"
#include "qapi_dnsc.h"
#include "qapi_fs.h"
#include "qapi_crypto.h"
#include "qapi_csr_generator.h"
#include "qapi_rtc.h"
#include "qapi_ssl_cert.h"

#ifdef CONFIG_NET_SSL_DEMO
/*
 * This file contains the command handlers for certificate management operations
 * on non-volatile memory like store, delete, list, etc.
 *
 */

extern QCLI_Group_Handle_t qcli_net_handle; /* Handle for Net Command Group. */

const char * file_type_certificate = "cert";
const char * file_type_ca_list = "ca_list";

uint8_t *cert_data_buf;
uint16_t cert_data_buf_len;

extern int certcs_download_file(
    char * hostname,
    int port,
    char * filename,
    uint8_t ** pp_buffer,
    uint32_t * p_buffer_size
    );

#define CERT_PRINTF(...) QCLI_Printf(qcli_net_handle, __VA_ARGS__)
#define CERTCS_PRINTF(...) QCLI_Printf(qcli_net_handle, __VA_ARGS__)

QCLI_Command_Status_t hash_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t get_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t generate_csr(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cert_validation(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t cert_expiration(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

QCLI_Command_Status_t cert_command_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if(Parameter_Count < 1)
    {
        return QCLI_STATUS_USAGE_E;
    }

    if (0 == strcmp(Parameter_List[0].String_Value, "store")) {
        return store_cert(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strcmp(Parameter_List[0].String_Value, "delete")) {
        return delete_cert(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strcmp(Parameter_List[0].String_Value, "list")) {
        return list_cert(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strcmp(Parameter_List[0].String_Value, "get")) {
        return get_cert(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strcmp(Parameter_List[0].String_Value, "hash")) {
        return hash_cert(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strcmp(Parameter_List[0].String_Value, "gencsr")) {
        return generate_csr(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strcmp(Parameter_List[0].String_Value, "validate")) {
        return cert_validation(Parameter_Count - 1, &Parameter_List[1]);
    }
    else if (0 == strcmp(Parameter_List[0].String_Value, "expiration")) {
        return cert_expiration(Parameter_Count - 1, &Parameter_List[1]);
    }
    return QCLI_STATUS_USAGE_E;
}






int certcs_resolve_hostname(char * hostname, struct ip46addr * p_ip46addr, int32_t * p_scope_id)
{
    int are_parameters_valid = 1 &&
        hostname &&
        p_ip46addr &&
        p_scope_id;

    if ( !are_parameters_valid ) {
        return -1;
    }

    memset(p_ip46addr, 0, sizeof(*p_ip46addr));


    if ( inet_pton(AF_INET, hostname, &p_ip46addr->a) == 0 )
    {
        // valid IPv4 address
        p_ip46addr->type = AF_INET;
        return 0;
    }

    if ( inet_pton(AF_INET6, hostname, &p_ip46addr->a) == 0 )
    {
        // valid IPv6 address
        if ( QAPI_IS_IPV6_LINK_LOCAL(&p_ip46addr->a) )
        {
            // if this is a link local address, then the interface must be specified after %
            char * interface_name_with_percent_char = strchr(hostname, '%');
            char * interface_name = NULL;
            if ( interface_name_with_percent_char ) {
                interface_name = interface_name_with_percent_char + 1;
            }
            else {
                // the IPv6 address is link local, but it does not contain the '%' character to specify the interface
                CERTCS_PRINTF("ERROR: IPv6 link local address is specified, but it is missing %% characters after which the interface name is followed\r\n");
                return -2;
            }

            int status = qapi_Net_IPv6_Get_Scope_ID(interface_name, p_scope_id);
            if ( status != 0 ) {
                // the IPv6 address is link local, but cannot find the scope id for the interface to use it on
                CERTCS_PRINTF("ERROR: IPv6 link local address is specified, but cannot resolve the scope id for the interface (%s)\r\n", interface_name);
                return -3;
            }
        }
        p_ip46addr->type = AF_INET6;
        return 0;
    }

    if ( strlen(hostname) >= QAPI_NET_DNS_MAX_HOSTNAME_LEN )
    {
        // the hostname is too long
        CERTCS_PRINTF("ERROR: Hostname (%s) is too long\r\n", hostname);
        return -4;
    }

    p_ip46addr->type = AF_INET;
    if ( qapi_Net_DNSc_Reshost(hostname, p_ip46addr) == 0 )
    {
        // valid hostname, which IPv4 got successfully resolved
        return 0;
    }

    p_ip46addr->type = AF_INET6;
    if ( qapi_Net_DNSc_Reshost(hostname, p_ip46addr) == 0 )
    {
        // valid hostname, which IPv6 got successfully resolved
        return 0;
    }

    CERTCS_PRINTF("ERROR: Failed to resolve hostname (%s)\r\n", hostname);
    return -5;
}


int certcs_connect(
    char * hostname,
    int port,
    int * p_socket
    )
{
    int are_parameters_valid = 1 &&
        hostname &&
        ( port > 0 ) &&
        p_socket;

    if ( !are_parameters_valid ) {
        return -1;
    }

    struct ip46addr ipaddr;
    int32_t scope_id;

    int status = certcs_resolve_hostname(
        hostname,
        &ipaddr,
        &scope_id
        );
    if ( 0 != status ) {
        return -2;
    }

    *p_socket = qapi_socket(ipaddr.type, SOCK_STREAM, 0);
    if ( *p_socket < 0)
    {
        // failed to create socket
        CERTCS_PRINTF("ERROR: failed to create socket\r\n, error=%d", *p_socket);
        return -3;
    }

    struct sockaddr_in sockaddr;
    struct sockaddr_in6 sockaddr6;

    struct sockaddr *to;
    uint32_t tolen;
    if ( ipaddr.type == AF_INET )
    {
        memset(&sockaddr, 0, sizeof(sockaddr));
        sockaddr.sin_addr.s_addr = ipaddr.a.addr4;
        sockaddr.sin_port = htons(port);
        sockaddr.sin_family = AF_INET;

        to      = (struct sockaddr *)&sockaddr;
        tolen   = sizeof(sockaddr);
    }
    else
    {
        memset(&sockaddr6, 0, sizeof(sockaddr6));
        memcpy(&sockaddr6.sin_addr, &ipaddr.a, sizeof(ip6_addr));
        sockaddr6.sin_port     = htons(port);
        sockaddr6.sin_family   = AF_INET6;
        sockaddr6.sin_scope_id = scope_id;

        to      = (struct sockaddr *)&sockaddr6;
        tolen   = sizeof(sockaddr6);
    }

    status = qapi_connect(*p_socket, to, tolen);
    if ( status < 0)
    {
        // failed to connect
        CERTCS_PRINTF("ERROR: failed on a call to qapi_connect, error=%d\r\n", status);
        qapi_socketclose(*p_socket);
        return -4;
    }

    return 0;
}


int certcs_send_request(
    int socket,
    char * filename
    )
{
    int filename_length = strlen(filename);
    int request_length = sizeof(CERT_HEADER_T) + filename_length;

    uint8_t * p_request = 0;
    p_request = (uint8_t *) malloc(request_length);
    if ( !p_request ) {
        CERTCS_PRINTF("ERROR: failed to allocate request message (%d bytes)\r\n", request_length);
        return -2;
    }
    memset(p_request, 0, request_length);

    CERT_HEADER_T * p_request_header = (CERT_HEADER_T *) p_request;
    p_request_header->id[0] = 'C';
    p_request_header->id[1] = 'R';
    p_request_header->id[2] = 'T';
    p_request_header->id[3] = '0';
    p_request_header->length = htonl(filename_length);

    memcpy(&p_request_header->data[0], filename, filename_length);

    int status = 0;
    uint8_t * p_buffer = p_request;
    do
    {
        int bytes_sent = qapi_send(socket, (char *) p_buffer, request_length, 0);
        if ( bytes_sent < 0 ) {
            CERTCS_PRINTF("ERROR: failed on a call to qapi_send, error=%d\r\n", bytes_sent);
            status = -3;
            break;
        }
        if ( bytes_sent > request_length ) {
            CERTCS_PRINTF("ERROR: sent more bytes than expected, expcted=%d bytes, sent=%d bytes", request_length, bytes_sent);
            status = -4;
            break;
        }
        p_buffer += bytes_sent;
        request_length -= bytes_sent;
    } while ( request_length > 0);

    free(p_request);

    return status;
}


int certcs_receive_data(
    int socket,
    uint8_t * p_buffer,
    size_t buffer_size
    )
{
    int are_parameters_valid = 1 &&
        p_buffer &&
        (buffer_size > 0);

    if ( !are_parameters_valid ) {
        return -1;
    }

    do
    {
        fd_set rset;
        qapi_fd_zero(&rset);
        qapi_fd_set(socket, &rset);

        int select_status = qapi_select(&rset, NULL, NULL, CLIENT_WAIT_TIME);

        if ( select_status > 0 )
        {
            // can receive data
            int bytes_received = qapi_recv(socket, (char *) p_buffer, buffer_size, 0);
            if ( bytes_received < 0 ) {
                CERTCS_PRINTF("ERROR: failed on a call to qapi_recv, status=%d\r\n", bytes_received);
                return -3;
            }
            else {
                if ( bytes_received > buffer_size ) {
                    CERTCS_PRINTF("ERROR: received more bytes than expected. expected=%d bytes, received=%d bytes\r\n", buffer_size, bytes_received);
                }
                p_buffer += bytes_received;
                buffer_size -= bytes_received;
            }
        }
        else
        {
            if ( select_status < 0 ) {
                CERTCS_PRINTF("ERROR: no response\n");
                return -2;
            }
        }

    } while ( buffer_size > 0 );

    return 0;
}

typedef struct certcs_heaser_s {
    uint8_t id[4];
    uint32_t length;
} certcs_heaser_t;





int certcs_receive_response(
    int socket,
    uint8_t ** pp_buffer,
    uint32_t * p_buffer_size
    )
{
    int are_params_valid = 1 &&
        pp_buffer &&
        p_buffer_size;

    if ( !are_params_valid ) {
        return -1;
    }

    certcs_heaser_t certcs_header;
    memset(&certcs_header, 0, sizeof(certcs_header));

    int status = certcs_receive_data(socket, (uint8_t*) &certcs_header, sizeof(certcs_header));
    if ( 0 != status ) {
        CERTCS_PRINTF("ERROR: failed to get certcs_header, status=%d\r\n", status);
        return -2;
    }

    int is_valid_certcs_header = 1 &&
        (certcs_header.id[0] == 'C') &&
        (certcs_header.id[1] == 'R') &&
        (certcs_header.id[2] == 'T');
    if ( !is_valid_certcs_header ) {
        CERTCS_PRINTF("ERROR: Invalid certcs header\r\n");
        return -3;
    }

    uint32_t file_size = htonl(certcs_header.length);

    if(file_size == 0) {
        CERTCS_PRINTF("ERROR: file size is 0\r\n");
        return -6;
    }

    *pp_buffer = (uint8_t *) malloc(file_size);
    if ( !*pp_buffer ) {
        CERTCS_PRINTF("ERROR: failed to allocate buffer\r\n");
        return -4;
    }

    status = certcs_receive_data(socket, *pp_buffer, file_size);
    if ( 0 != status ) {
        CERTCS_PRINTF("ERROR: failed to receive data, status=%d\r\n", status);
        free(*pp_buffer);
        *pp_buffer = 0;
        return -5;
    }

    *p_buffer_size = file_size;
    return 0;
}

int certcs_download_file(
    char * hostname,
    int port,
    char * filename,
    uint8_t ** pp_buffer,
    uint32_t * p_buffer_size
    )
{
    int are_parameters_valid = 1 &&
        hostname &&
        ( port > 0 ) &&
        filename &&
        pp_buffer &&
        p_buffer_size;

    if ( !are_parameters_valid ) {
        return -1;
    }

    int socket;
    int status =  certcs_connect(
        hostname,
        port,
        &socket
        );
    if ( 0 != status ) {
        return -2;
    }

    status = certcs_send_request(socket, filename);
    if ( 0 != status ) {
        status = -3;
        goto certcs_download_file_cleanup;
    }

    status = certcs_receive_response(socket, pp_buffer, p_buffer_size);
    if ( 0 != status ) {
        status = -4;
        goto certcs_download_file_cleanup;
    }

certcs_download_file_cleanup:
    qapi_socketclose(socket);
    return status;
}



QCLI_Command_Status_t get_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char * hostname = 0;
    char * port_as_string = 0;
    char * certificate_or_ca_list_filename = 0;
    char * key_filename = 0;
    char * type_as_string = 0;
    char * output_filename = 0;
    uint16_t port = 0;
    int type = 0;

    uint8_t * certificate_or_ca_list_buffer = 0;
    uint32_t certificate_or_ca_list_buffer_size = 0;
    uint8_t * key_buffer = 0;
    uint32_t key_buffer_size = 0;

    int parameter_index;
    int status;

    int return_status = QCLI_STATUS_ERROR_E;

    if (Parameter_Count < 2)
    {
        CERTCS_PRINTF("Invalid number of arguments\r\n");
        goto get_cert_on_error;
    }

    certificate_or_ca_list_filename = Parameter_List[0].String_Value;
    hostname = Parameter_List[1].String_Value;


    for ( parameter_index = 2; parameter_index < Parameter_Count; parameter_index++ )
    {
        if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-p") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
                CERTCS_PRINTF("ERROR: -p option is present, but no port is specified\r\n");
                goto get_cert_on_error;
            }
            port_as_string = (char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-k") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
                CERTCS_PRINTF("ERROR: -k option is present, but no key filename is specified\r\n");
                goto get_cert_on_error;
            }
            key_filename = (char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-s") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
                CERTCS_PRINTF("ERROR: -s option is present, but no output filename is specified\r\n");
                goto get_cert_on_error;
            }
            output_filename = (char*)Parameter_List[parameter_index].String_Value;
        }

        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-t") )
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
                CERTCS_PRINTF("ERROR: -t option is present, but no type (cert/ca_list) is specified\r\n");
                goto get_cert_on_error;
            }
            type_as_string = (char*)Parameter_List[parameter_index].String_Value;
            parameter_index++;
        }
        else
        {
            CERTCS_PRINTF("ERROR: Unknown option (%s)\r\n", (char*)Parameter_List[parameter_index].String_Value);
            goto get_cert_on_error;
        }
    }

    if ( port_as_string ) {
        port = atoi(port_as_string);
    }
    else {
        // default port
        port = 1443;
    }

    if ( !output_filename ) {
        output_filename = certificate_or_ca_list_filename;
    }


    if ( 0 == strcmp(type_as_string, "pem_cert") )
    {
        if ( !certificate_or_ca_list_filename ) {
            CERTCS_PRINTF("ERROR: must specify certificate filename\r\n");
            goto get_cert_on_error;
        }
        if ( !key_filename ) {
            CERTCS_PRINTF("ERROR: must specify key filename\r\n");
            goto get_cert_on_error;
        }
        type = QAPI_NET_SSL_PEM_CERTIFICATE_WITH_PEM_KEY_E;
    }
    else if ( 0 == strcmp(type_as_string, "pem_ca_list") )
    {
        type = QAPI_NET_SSL_PEM_CA_LIST_E;
    }
    else if ( 0 == strcmp(type_as_string, "cert") )
    {
        type = QAPI_NET_SSL_BIN_CERTIFICATE_E;
    }
    else if ( 0 == strcmp(type_as_string, "ca_list"))
    {
        type = QAPI_NET_SSL_BIN_CA_LIST_E;
    }
    else if ( 0 == strcmp(type_as_string, "pem_cert_tee_key") )
    {
        if ( !certificate_or_ca_list_filename ) {
            CERTCS_PRINTF("ERROR: must specify certificate filename to download\r\n");
            goto get_cert_on_error;
        }
        if ( !key_filename ) {
            CERTCS_PRINTF("ERROR: must specify internally generated keypair filename using -k option\r\n");
            goto get_cert_on_error;
        }
        type = QAPI_NET_SSL_PEM_CERTIFICATE_WITH_TEE_KEYPAIR_E;
    }
    else
    {
        CERTCS_PRINTF("ERROR: must use -t option to specify whether this is cert/ca_list/pem_cert/pem_ca_list\r\n");
        goto get_cert_on_error;
    }




    status =
        certcs_download_file(
            hostname,
            port,
            certificate_or_ca_list_filename,
            &certificate_or_ca_list_buffer,
            &certificate_or_ca_list_buffer_size
            );
    if ( 0 != status ) {
        CERTCS_PRINTF("ERROR: failed to download the cert/ca_list file, status=%d\r\n", status);
        goto get_cert_on_error;
    }
    CERTCS_PRINTF("Successfully downloaded %s\r\n", certificate_or_ca_list_filename);


    if ( QAPI_NET_SSL_BIN_CERTIFICATE_E == type )
    {
        qapi_Net_SSL_Cert_Info_t cert_info;
        memset(&cert_info, 0, sizeof(cert_info));
        cert_info.cert_Type = type;
        cert_info.info.bin_Cert.cert_Buf = certificate_or_ca_list_buffer;
        cert_info.info.bin_Cert.cert_Size = certificate_or_ca_list_buffer_size;

        status = qapi_Net_SSL_Cert_Store(&cert_info, output_filename);
        if ( QAPI_OK != status ) {
            CERTCS_PRINTF("Failed to store certificate\r\n");
            goto get_cert_on_error;
        }
        else
        {
            CERTCS_PRINTF("Successfully stored certificate\r\n");
        }
    }
    else if ( QAPI_NET_SSL_BIN_CA_LIST_E == type )
    {
        qapi_Net_SSL_Cert_Info_t cert_info;
        memset(&cert_info, 0, sizeof(cert_info));
        cert_info.cert_Type = type;
        cert_info.info.bin_CA_List.ca_List_Buf = certificate_or_ca_list_buffer;
        cert_info.info.bin_CA_List.ca_List_Size = certificate_or_ca_list_buffer_size;

        status = qapi_Net_SSL_Cert_Store(&cert_info, output_filename);
        if ( QAPI_OK != status ) {
            CERTCS_PRINTF("Failed to store CA list\r\n");
            goto get_cert_on_error;
        }
        else
        {
            CERTCS_PRINTF("Successfully stored CA list\r\n");
        }
    }
    else if ( QAPI_NET_SSL_PEM_CERTIFICATE_WITH_TEE_KEYPAIR_E == type )
    {
        qapi_Net_SSL_Cert_Info_t cert_info;
        memset(&cert_info, 0, sizeof(cert_info));
        cert_info.cert_Type = type;
        cert_info.info.pem_Cert_TEE_Keypair.cert_Buf = certificate_or_ca_list_buffer;
        cert_info.info.pem_Cert_TEE_Keypair.cert_Size = certificate_or_ca_list_buffer_size;
        cert_info.info.pem_Cert_TEE_Keypair.tee_Keypair_Name = key_filename;
        status = qapi_Net_SSL_Cert_Store(&cert_info, output_filename);
        if ( QAPI_OK != status ) {
            CERTCS_PRINTF("ERROR: failed to convert and store certificate, status=%d\r\n", status);
            goto get_cert_on_error;
        }
        CERTCS_PRINTF("Successfully converted and stored certificate\r\n");
    }
    else if ( QAPI_NET_SSL_PEM_CERTIFICATE_WITH_PEM_KEY_E == type )
    {
        status =
            certcs_download_file(
                hostname,
                port,
                key_filename,
                &key_buffer,
                &key_buffer_size
                );
        if ( 0 != status ) {
            CERTCS_PRINTF("ERROR: failed to download key file, status=%d\r\n", status);
            goto get_cert_on_error;
        }
        CERTCS_PRINTF("Successfully downloaded %s\r\n", key_filename);

        qapi_Net_SSL_Cert_Info_t cert_info;
        memset(&cert_info, 0, sizeof(cert_info));
        cert_info.cert_Type = type;
        cert_info.info.pem_Cert.cert_Buf = certificate_or_ca_list_buffer;
        cert_info.info.pem_Cert.cert_Size = certificate_or_ca_list_buffer_size;
        cert_info.info.pem_Cert.key_Buf = key_buffer;
        cert_info.info.pem_Cert.key_Size = key_buffer_size;
        status = qapi_Net_SSL_Cert_Store(&cert_info, output_filename);
        if ( QAPI_OK != status ) {
            CERTCS_PRINTF("ERROR: failed to convert and store certificate, status=%d\r\n", status);
            goto get_cert_on_error;
        }
        CERTCS_PRINTF("Successfully converted and stored certificate\r\n");
    }
    else if ( QAPI_NET_SSL_PEM_CA_LIST_E == type )
    {
        qapi_Net_SSL_Cert_Info_t cert_info;
        memset(&cert_info, 0, sizeof(cert_info));
        qapi_CA_Info_t ca_info;
        ca_info.ca_Buf = certificate_or_ca_list_buffer;
        ca_info.ca_Size = certificate_or_ca_list_buffer_size;
        cert_info.cert_Type = type;
        cert_info.info.pem_CA_List.ca_Cnt = 1;
        cert_info.info.pem_CA_List.ca_Info[0] = &ca_info;
        status = qapi_Net_SSL_Cert_Store(&cert_info, output_filename);
        if ( QAPI_OK != status ) {
            CERTCS_PRINTF("ERROR: failed to convert and store ca_list, status=%d\r\n", status);
            goto get_cert_on_error;
        }
        CERTCS_PRINTF("Successfully converted and stored CA list\r\n");
    }

    CERTCS_PRINTF("%s is stored in NV memory\n", output_filename);
    return_status =  QCLI_STATUS_SUCCESS_E;

get_cert_on_error:
    if ( QCLI_STATUS_SUCCESS_E != return_status ) {
        if ( !output_filename ) {
            CERT_PRINTF("ERROR: failed to store %s\n", output_filename);
        }

        CERTCS_PRINTF("Usage: cert get <filename> <host> [-p <port>] [-k <key>] [-s <name>] -t <filetype>\r\n");
        CERTCS_PRINTF("\t\tfilename: filename on host representing certificate or ca_list in either binary or PEM format\r\n");
        CERTCS_PRINTF("\t\thost: hostname to connect to and retrieve the certificate or ca_list from\r\n");
        CERTCS_PRINTF("\t\tport: port to connect to and retrieve the certificate or ca_list from\r\n");
        CERTCS_PRINTF("\t\tkey: name of a file containing key in pem format (in case of pem_cert), or the name of generated key (in case of pem_cert_tee_key)\r\n");
        CERTCS_PRINTF("\t\tname: name for the certificate / ca_list to be stored on the device\r\n");
        CERTCS_PRINTF("\t\tfiletype: type of the file to retrieve from host, could be: cert/ca_cert/pem_cert/pem_ca_list/pem_cert_tee_key\r\n");
        CERTCS_PRINTF("\t\t\t\tcert: certificate in binary format\r\n");
        CERTCS_PRINTF("\t\t\t\tca_list: CA list in binary format\r\n");
        CERTCS_PRINTF("\t\t\t\tpem_cert: certificate in PEM format\r\n");
        CERTCS_PRINTF("\t\t\t\tpem_ca_list: CA list in PEM format\r\n");
        CERTCS_PRINTF("\t\t\t\tpem_cert_tee_key: certificate in PEM format, key from internally generated keypair\r\n");
    }
    if ( certificate_or_ca_list_buffer ) {
        free(certificate_or_ca_list_buffer);
        certificate_or_ca_list_buffer = 0;
    }
    if ( key_buffer ) {
        free(key_buffer);
        key_buffer = 0;
    }
    return return_status;
}



QCLI_Command_Status_t store_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char * name;
    char * option_file_type;
    char * file_type;


    if(Parameter_Count < 3) {
        CERT_PRINTF("ERROR: Invalid number of parameters\r\n");
        goto store_cert_on_error;
    }

    name = Parameter_List[0].String_Value;
    option_file_type = Parameter_List[1].String_Value;
    file_type = Parameter_List[2].String_Value;

    if ( 0 ||
        !option_file_type ||
        !file_type ||
        !( strcmp("-t", option_file_type) == 0 ) ||
        !( (strcmp(file_type_certificate, file_type) == 0) || (strcmp(file_type_ca_list, file_type) == 0) )
    )
    {
        CERT_PRINTF("ERROR: Invalid filetype.  Must be either cert or ca_list\r\n");
        goto store_cert_on_error;
    }

    if (cert_data_buf_len == 0)
    {
        CERT_PRINTF("ERROR: no certificate data.\nHint: Use the \"cert get\" command to read a certificate from a certificate server.\n");
        goto store_cert_on_error;
    }

    int result;
    if ( strcmp(file_type_certificate, file_type) == 0 ) {
        qapi_Net_SSL_Cert_Info_t cert_info;
        memset(&cert_info, 0, sizeof(cert_info));
        cert_info.cert_Type = QAPI_NET_SSL_BIN_CERTIFICATE_E;
        cert_info.info.bin_Cert.cert_Buf = cert_data_buf;
        cert_info.info.bin_Cert.cert_Size = cert_data_buf_len;
        result = qapi_Net_SSL_Cert_Store(&cert_info, name);
    }
    else {
        qapi_Net_SSL_Cert_Info_t cert_info;
        memset(&cert_info, 0, sizeof(cert_info));
        cert_info.cert_Type = QAPI_NET_SSL_BIN_CA_LIST_E;
        cert_info.info.bin_CA_List.ca_List_Buf = cert_data_buf;
        cert_info.info.bin_CA_List.ca_List_Size = cert_data_buf_len;
        result = qapi_Net_SSL_Cert_Store(&cert_info, name);
    }

    if (QAPI_OK == result)
    {
        CERT_PRINTF("%s is stored in NV memory\n", name);
    }
    else
    {
        CERT_PRINTF("ERROR: failed to store %s\n", name);
        goto store_cert_on_error;
    }
    return QCLI_STATUS_SUCCESS_E;

store_cert_on_error:
    CERT_PRINTF("Usage: cert store <name> -t <filetype>\n"
                    "This stores a certificate or CA list in non-volatile memory with <name>. filetype must be either cert of ca_list\n");
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t delete_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    char *name;
    char * option_file_type;
    char * file_type;

    if(Parameter_Count < 1)
    {
        if (cert_data_buf) {
            cert_data_buf_len = 0;
            free(cert_data_buf);
            cert_data_buf = NULL;
            CERT_PRINTF("Deleted the certificate data stored in RAM.\n");
            return QCLI_STATUS_SUCCESS_E;
        }
        else {
            goto delete_cert_on_error;
        }
    }

    if (Parameter_Count < 3) {
        goto delete_cert_on_error;
    }

    name = Parameter_List[0].String_Value;
    option_file_type = Parameter_List[1].String_Value;
    file_type = Parameter_List[2].String_Value;

    if ( 0 ||
        !option_file_type ||
        !file_type ||
        !( strcmp("-t", option_file_type) == 0 ) ||
        !( (strcmp(file_type_certificate, file_type) == 0) || (strcmp(file_type_ca_list, file_type) == 0) )
    )
    {
        CERT_PRINTF("ERROR: Invalid filetype.  Must be either cert or ca_list\r\n");
        goto delete_cert_on_error;
    }

    int status;
    if ( strcmp(file_type_certificate, file_type) == 0 ) {
        qapi_Net_SSL_Cert_Info_t cert_info;
        memset(&cert_info, 0, sizeof(cert_info));
        cert_info.cert_Type = QAPI_NET_SSL_BIN_CERTIFICATE_E;
        cert_info.info.bin_Cert.cert_Buf = 0;
        cert_info.info.bin_Cert.cert_Size = 0;
        status = qapi_Net_SSL_Cert_Store(&cert_info, name);
    }
    else {
        qapi_Net_SSL_Cert_Info_t cert_info;
        memset(&cert_info, 0, sizeof(cert_info));
        cert_info.cert_Type = QAPI_NET_SSL_BIN_CA_LIST_E;
        cert_info.info.bin_CA_List.ca_List_Buf = 0;
        cert_info.info.bin_CA_List.ca_List_Size = 0;
        status = qapi_Net_SSL_Cert_Store(&cert_info, name);
    }

    if (QAPI_OK == status)
    {
        CERT_PRINTF("Deleted %s from NV memory\n", name);
    }
    else
    {
        CERT_PRINTF("ERROR: failed to delete %s\n", name);
        goto delete_cert_on_error;
    }

    return QCLI_STATUS_SUCCESS_E;

delete_cert_on_error:
    CERT_PRINTF("Usage: cert delete <name> -t <filetype>\n"
            "This deletes the certificate or CA list with <name> from non-volatile memory. filetype must be either cert or ca_list.\n");
    return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t list_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_Net_SSL_Cert_List_t list;
    int32_t i, numFiles;

    if ( Parameter_Count < 2 ) {
        goto list_cert_on_error;
    }

    if ( 0 != strcmp("-t", Parameter_List[0].String_Value) ) {
        goto list_cert_on_error;
    }

    if ( 0 == strcmp(file_type_certificate, Parameter_List[1].String_Value) ) {
        numFiles = qapi_Net_SSL_Cert_List(QAPI_NET_SSL_CERTIFICATE_E, &list);
    }
    else if ( 0 == strcmp(file_type_ca_list, Parameter_List[1].String_Value) ) {
        numFiles = qapi_Net_SSL_Cert_List(QAPI_NET_SSL_CA_LIST_E, &list);
    }
    else {
        goto list_cert_on_error;
    }

    if (numFiles < 0)
    {
        CERT_PRINTF("ERROR: failed to list files (%d)\n", numFiles);
        return QCLI_STATUS_ERROR_E;
    }

    CERT_PRINTF("%d %s stored in NV memory\n", numFiles, numFiles == 1 ? "file" : "files");
    for (i=0; i<numFiles; i++)
    {
        CERT_PRINTF("%s\n", list.name[i]);
    }
    return QCLI_STATUS_SUCCESS_E;

list_cert_on_error:
    CERT_PRINTF("Usage: cert list -t <filetype>\r\n"
            "Lists the certificates / CA lists stored on the device. filetype must be either cert or ca_list.\r\n");
    return QCLI_STATUS_ERROR_E;
}



QCLI_Command_Status_t hash_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_Status_t status;

    if ( Parameter_Count != 3 ) {
        status = QAPI_ERR_INVALID_PARAM;
        goto hash_cert_on_error;
    }

    if ( 0 != strcmp("-t", Parameter_List[0].String_Value) ) {
        status = QAPI_ERR_INVALID_PARAM;
        goto hash_cert_on_error;
    }

    char * name = Parameter_List[2].String_Value;

    uint8_t hash[QAPI_CRYPTO_SHA256_DIGEST_BYTES];

    if ( 0 == strcmp(file_type_certificate, Parameter_List[1].String_Value) ) {
        status = qapi_Net_SSL_Cert_Get_Hash(name , QAPI_NET_SSL_CERTIFICATE_E, (qapi_Net_SSL_Cert_Hash_t) hash);
    }
    else if ( 0 == strcmp(file_type_ca_list, Parameter_List[1].String_Value) ) {
        status = qapi_Net_SSL_Cert_Get_Hash(name , QAPI_NET_SSL_CA_LIST_E, (qapi_Net_SSL_Cert_Hash_t) hash);
    }
    else {
        status = QAPI_ERR_INVALID_PARAM;
        goto hash_cert_on_error;
    }

    if ( QAPI_OK != status )
    {
        CERT_PRINTF("ERROR: failed to retrieve hash\r\n");
        goto hash_cert_on_error;
        return QCLI_STATUS_ERROR_E;
    }

    int i;
    for ( i = 0; i < QAPI_CRYPTO_SHA256_DIGEST_BYTES; i++)
    {
        CERT_PRINTF("%02X", hash[i]);
    }
    CERT_PRINTF("\r\n");

    return QCLI_STATUS_SUCCESS_E;

hash_cert_on_error:
    CERT_PRINTF("Usage: cert hash -t <type> <name>\r\n"
            "Prints SHA256 hash of the certificate / CA list stored on the device. type must be either cert or ca_list.  name is the name of the certificate / CA list.\r\n");
    return QCLI_STATUS_ERROR_E;
}


void hexdump(void * p_buffer, size_t buffer_size, size_t bytes_per_line)
{
    if ( 0 ||
        !p_buffer ||
        ( 0 == buffer_size )
        )
    {
        return;
    }

    if ( bytes_per_line == 0 ) {
        bytes_per_line = buffer_size;
    }

    uint8_t * p_temp = (uint8_t *) p_buffer;
    int bytes_printed;
    for ( bytes_printed = 0; bytes_printed < buffer_size; bytes_printed++ )
    {
        if ( (0 != bytes_printed) && ((bytes_printed % bytes_per_line) == 0) ) {
            CERT_PRINTF("\r\n");
        }
        CERT_PRINTF("%02x", p_temp[bytes_printed]);
    }
}


int demo_generate_and_print_pem_csr(
    const char * keypair_filename,
    qapi_Crypto_Keypair_Params_t * keypair_params,
    char * service_provider_id_string
    )
{
    if ( 0 ||
        !keypair_filename ||
        !keypair_params ||
        !service_provider_id_string
        )
    {
        return -1;
    }

    uint8_t * p_pem_csr = 0;
    size_t pem_csr_size = 0;

    size_t service_provider_id_size = strlen(service_provider_id_string);
    uint8_t * p_CSR = 0;
    size_t CSR_Size = 0;
    uint8_t * p_CSR_Protected = 0;
    size_t CSR_Protected_Size = 0;
    uint8_t nonce[QAPI_PROTECTED_CSR_NONCE_SIZE];
    uint8_t tag[QAPI_PROTECTED_CSR_TAG_SIZE];
    int status =
        qapi_Net_SSL_CSR_Generate(
            keypair_filename,
            keypair_params,
            (uint8_t*) service_provider_id_string,
            service_provider_id_size,
            &p_CSR,
            &CSR_Size,
            &p_CSR_Protected,
            &CSR_Protected_Size,
            nonce,
            tag
            );
    if ( 0 != status ) {
        CERT_PRINTF("Failed on a call to qapi_Net_SSL_CSR_Generate(), status=%d\r\n", status);
        status = -2;
        goto demo_generate_and_print_pem_csr_cleanup;
    }

    CERT_PRINTF("CSR hexdump:\r\n");
    hexdump(p_CSR, CSR_Size, 32);
    CERT_PRINTF("\r\n\r\n");

    CERT_PRINTF("Nonce: ");
    hexdump(nonce, sizeof(nonce), sizeof(nonce));
    CERT_PRINTF("\r\n");

    CERT_PRINTF("Tag: ");
    hexdump(tag, sizeof(tag), sizeof(tag));
    CERT_PRINTF("\r\n");

    CERT_PRINTF("Protected CSR hexdump:\r\n");
    hexdump(p_CSR_Protected, CSR_Protected_Size, 32);
    CERT_PRINTF("\r\n\r\n");


    int bin_csr_to_pem_csr(
        const uint8_t * p_bin_csr,
        const size_t bin_csr_size,
        uint8_t ** pp_pem_csr,
        size_t * p_pem_csr_size
        );

    status = bin_csr_to_pem_csr(
        p_CSR,
        CSR_Size,
        &p_pem_csr,
        &pem_csr_size
        );
    if ( 0 != status ) {
        CERT_PRINTF("Failed on a call to bin_csr_to_pem_csr(), status=%d\r\n", status);
        status = -3;
        goto demo_generate_and_print_pem_csr_cleanup;
    }

    CERT_PRINTF("PEM CSR:\r\n");
    int i;
    for ( i = 0; i < pem_csr_size; i++ ) {
        CERT_PRINTF("%c", p_pem_csr[i]);
    }
    CERT_PRINTF("\r\n");


demo_generate_and_print_pem_csr_cleanup:
    if ( p_CSR ) {
        free(p_CSR);
        p_CSR = 0;
    }
    if ( p_CSR_Protected ) {
        free(p_CSR_Protected);
        p_CSR_Protected = 0;
    }
    if ( p_pem_csr ) {
        free(p_pem_csr);
        p_pem_csr = 0;
    }

    return status;
}

//int csr_generator_test();

QCLI_Command_Status_t generate_csr(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    if ( Parameter_Count != 4 ) {
        CERT_PRINTF("Invalid number of arguments\r\n");
        goto generate_csr_on_error;

    }

    char * keypair_filename = Parameter_List[0].String_Value;
    char * key_type = Parameter_List[1].String_Value;
    char * key_params = Parameter_List[2].String_Value;
    char * service_provider_id_string = Parameter_List[3].String_Value;

    qapi_Crypto_Keypair_Params_t keypair_params;

    if ( 0 == strcmp(key_type, "ecc") ) {
        keypair_params.key_Type = QAPI_KEYPAIR_PARAM_KEY_TYPE_ECC_E;
    }
    else if ( 0 == strcmp(key_type, "rsa") ) {
        keypair_params.key_Type = QAPI_KEYPAIR_PARAM_KEY_TYPE_RSA_E;
        CERT_PRINTF("Generation of CSR using RSA key is not supported\r\n");
        goto generate_csr_on_error;
    }
    else {
        CERT_PRINTF("Unknown key type: %s.  Only \"ecc\" key type is supported for now", key_type);
        goto generate_csr_on_error;
    }

    if ( QAPI_KEYPAIR_PARAM_KEY_TYPE_ECC_E == keypair_params.key_Type ) {
        if ( 0 == strcmp(key_params, "secp192r1") ) {
            keypair_params.params.ecc.curve_Type = QAPI_EC_CURVE_ID_SECP192R1;
        }
        else if ( 0 == strcmp(key_params, "secp224r1") ) {
            keypair_params.params.ecc.curve_Type = QAPI_EC_CURVE_ID_SECP224R1;
        }
        else if ( 0 == strcmp(key_params, "secp256r1") ) {
            keypair_params.params.ecc.curve_Type = QAPI_EC_CURVE_ID_SECP256R1;
        }
        else if ( 0 == strcmp(key_params, "secp384r1") ) {
            keypair_params.params.ecc.curve_Type = QAPI_EC_CURVE_ID_SECP384R1;
        }
        else {
            CERT_PRINTF("Unsupported curve type: %s\r\n", key_params);
            goto generate_csr_on_error;
        }
    }


    int status = demo_generate_and_print_pem_csr(keypair_filename, &keypair_params, service_provider_id_string);
    if ( 0 != status ) {
        CERT_PRINTF("Failed on a call to demo_generate_and_print_pem_csr(), status=%d\r\n", status);
        //return QCLI_STATUS_ERROR_E;
    }

    return QCLI_STATUS_SUCCESS_E;

generate_csr_on_error:
    CERT_PRINTF("Usage: gencsr keypair_filename key_type key_params service_provider_id_string\r\n");
    CERT_PRINTF("\t\tkeypair_filename: filename to store keypair into, e.g. \"/spinor/keypair.bin\"\r\n");
    CERT_PRINTF("\t\tkey_type: key type \"rsa\" or \"ecc\".  Only \"ecc\" is supported for now\r\n");
    CERT_PRINTF("\t\tkey_params: key parameters.  This should be the curve type in case of \"ecc\" key. E.g. \"secp192r1\", \"secp224r1\", \"secp256r1\" or \"secp384r1\"\r\n");
    return QCLI_STATUS_ERROR_E;
}

/* CN=rootCA */
const char root_ca[] =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIDkTCCAnmgAwIBAgIJAL5dzF5xCbLbMA0GCSqGSIb3DQEBCwUAMF8xCzAJBgNV\n"
        "BAYTAlVTMQswCQYDVQQIDAJDQTERMA8GA1UEBwwIU2FuIEpvc2UxETAPBgNVBAoM\n"
        "CFF1YWxjb21tMQwwCgYDVQQLDANJb1QxDzANBgNVBAMMBnJvb3RDQTAeFw0xNzAy\n"
        "MjMwNjQ3MzJaFw0yMjEwMDMwNjQ3MzJaMF8xCzAJBgNVBAYTAlVTMQswCQYDVQQI\n"
        "DAJDQTERMA8GA1UEBwwIU2FuIEpvc2UxETAPBgNVBAoMCFF1YWxjb21tMQwwCgYD\n"
        "VQQLDANJb1QxDzANBgNVBAMMBnJvb3RDQTCCASIwDQYJKoZIhvcNAQEBBQADggEP\n"
        "ADCCAQoCggEBALZkC9sr4Tr/7XIxPUkL+3ZuHF0dajYCj/he4cUMfSWVjfTtBN0I\n"
        "Od/TcnV+oJpLITRyuoBurvOwkExXJ1OpllOtyjT1s6BX+2uosQJSFsQWZ2tSlOfs\n"
        "VNcOhwOVa+bw0pUdETdwO/K0pGMGxDsMp/SrzyyhTiShqAzG5JLxZfCxJQKdT2vj\n"
        "Bi87cx854WVhUq+cFTYvs1pQi3UtIQB2wRoECQnCSzvZGnWO4US6e+T4nKdenlI5\n"
        "HQuOmi5+MpScGLsYN72rFeIu96qC7geZpZ8jJ6hyN1veb1DovYZNLLJHEtI3xwja\n"
        "kUe0yLLJu85pKQBkj/pUjULH1tE01wE8pmkCAwEAAaNQME4wHQYDVR0OBBYEFAhI\n"
        "hzRwX/OwDtsONC706gCgmbX4MB8GA1UdIwQYMBaAFAhIhzRwX/OwDtsONC706gCg\n"
        "mbX4MAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBAHnNBdoBuW4oHS6c\n"
        "vvh3WZPQsIpRYDdfofk9AjzSKiHojjA1MfD7GzyUImv8FF3oeN63B8nw7qIscdTD\n"
        "EMSJFISiswitJ4AzhSsDpTbtLPQh0Wm/tKuZfyhkO0dZuEz0jXeR+YedOHcqQ/rN\n"
        "Frsq/soX7lNJ7hqz8alsEAaeBZiglz6mY3JGgtDLZ6j48U7tWkNzSIv6++4FGz28\n"
        "KRubHajq3mhvXoiGhcI7gP+sXI7XzuhrbwFKGKJaTqs6JkNPX+J2TI4EbhLJGayh\n"
        "Gri81Eb4my4wjFdYaC0gvkvR7TkLEq5dHDGRbQvKHPTs6AusNJYeiTgf4gRYz6WU\n"
        "HgE0gLE=\n"
        "-----END CERTIFICATE-----";

/* CN=rootCA */
const char root_ca_ecc[] =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIB8jCCAaCgAwIBAgIJAPLHLaB/B6SiMAoGCCqGSM49BAMCMF8xCzAJBgNVBAYT\n"
        "AlVTMQswCQYDVQQIDAJDQTERMA8GA1UEBwwIU2FuIEpvc2UxETAPBgNVBAoMCFF1\n"
        "YWxjb21tMQwwCgYDVQQLDANJb1QxDzANBgNVBAMMBnJvb3RDQTAeFw0xODAzMjgx\n"
        "OTUyMDhaFw0yMDEyMjIxOTUyMDhaMF8xCzAJBgNVBAYTAlVTMQswCQYDVQQIDAJD\n"
        "QTERMA8GA1UEBwwIU2FuIEpvc2UxETAPBgNVBAoMCFF1YWxjb21tMQwwCgYDVQQL\n"
        "DANJb1QxDzANBgNVBAMMBnJvb3RDQTBOMBAGByqGSM49AgEGBSuBBAAhAzoABE2J\n"
        "d6RNi6+xj+AMej0DSucOqQk1CbovPfCz4/a0u/mcOMcOokqcC6ELoiE5XAbWnGQ7\n"
        "UC3SpTf7o1AwTjAdBgNVHQ4EFgQUTmkH2EtApebyWcMOBhqLeYWXHFswHwYDVR0j\n"
        "BBgwFoAUTmkH2EtApebyWcMOBhqLeYWXHFswDAYDVR0TBAUwAwEB/zAKBggqhkjO\n"
        "PQQDAgNAADA9AhwFT9RXoet7YMU6lqlVwGn8P3fyxgxddQBp6U1HAh0AzOpCewpQ\n"
        "EwnNPuoIaije2IxbxMfJHF9aRWkI6A==\n"
        "-----END CERTIFICATE-----";

/* CN=rootCA, but signature is different */
const char root_ca_unknown[] =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIDnjCCAoagAwIBAgIJAJQCCbemnRVfMA0GCSqGSIb3DQEBCwUAMF8xCzAJBgNV\n"
        "BAYTAlVTMQswCQYDVQQIDAJDQTERMA8GA1UEBwwIU2FuIEpvc2UxETAPBgNVBAoM\n"
        "CFF1YWxjb21tMQwwCgYDVQQLDANJb1QxDzANBgNVBAMMBnJvb3RDQTAeFw0xNzAz\n"
        "MjEyMjI3MDNaFw0yMjEwMjkyMjI3MDNaMF8xCzAJBgNVBAYTAlVTMQswCQYDVQQI\n"
        "DAJDQTERMA8GA1UEBwwIU2FuIEpvc2UxETAPBgNVBAoMCFF1YWxjb21tMQwwCgYD\n"
        "VQQLDANJb1QxDzANBgNVBAMMBnJvb3RDQTCCASIwDQYJKoZIhvcNAQEBBQADggEP\n"
        "ADCCAQoCggEBAMdxK+IMi2Oo1jU9/qdKG9OHZ1iXo+1fk/cc6SwR8wA4AatO8p8o\n"
        "b6DaGeVHY2CYtWi8XkWAzvhZAvZoyhVdk7zh7S+qay83YqgTEVYpJZ46uvO4ZQid\n"
        "27d2kqtfU/Hlc1NEUP1J8v19nq4RodvzlK3MqVCaUo75UShJPdVY/oY66eu3vN0H\n"
        "/sIIGC+OPHxiG/qSJkH2H+nulXRhYu8PBQowYh/luTDORZB9wU7fScVrDlV9lTQv\n"
        "zQyy4rBSqrar95KElnD0ByxolugdDmL5kd4uOJh82Q0wxy3kLXj/kkWPdsZPB9mF\n"
        "gNIK/gG1c0JiWXGrDZFzvAUpfWbHEN0wNgsCAwEAAaNdMFswHQYDVR0OBBYEFJUa\n"
        "+UB6OBHsBTojenrDAqGgkr2cMB8GA1UdIwQYMBaAFJUa+UB6OBHsBTojenrDAqGg\n"
        "kr2cMAwGA1UdEwQFMAMBAf8wCwYDVR0PBAQDAgEGMA0GCSqGSIb3DQEBCwUAA4IB\n"
        "AQA38Zv5xabIE22RAr23iyYsEa59tv7fowyyvx2Ez+k8jxWrfWfdCyDACDxBnoET\n"
        "ajKh5fU5WTtUuy1/iMR6yBzZscSTYGV1DyXDdp8qG0GzjhBdaLH7rtypkNHsG6IZ\n"
        "oq8n7aV2rQ6F3+ChPUDLzMGaW0Zd084exQMhj7j3eKbXfM1NDMXfKZ1g+aibFiRs\n"
        "OCJKOJQkiTFFpTgiDpOIJni5oN2c+NhEXTDtmT6+3OANqis1/cjZ/DpjfZ8AifDR\n"
        "uMgp9jG33EaZJ3UA4pCobdLIs0rSyS++VwZ+eRfbuWxLwCjfnWCvitd3YenVcnBV\n"
        "E99Ki5T9qc7xTSn8RW8JyI82\n"
        "-----END CERTIFICATE-----";

/* CN=server signed by intermediaCA which is signed by rootCA */
const char peer_cert_with_intermediate_ca[] =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIDQzCCAisCCQD/lh6Cn/NY3TANBgkqhkiG9w0BAQsFADBoMQswCQYDVQQGEwJV\n"
        "UzELMAkGA1UECAwCQ0ExETAPBgNVBAcMCFNhbiBKb3NlMREwDwYDVQQKDAhRdWFs\n"
        "Y29tbTEMMAoGA1UECwwDSW9UMRgwFgYDVQQDDA9pbnRlcm1lZGlhdGVDQTAwHhcN\n"
        "MTcwMjIzMDY0NzMzWhcNMTgwNzA4MDY0NzMzWjBfMQswCQYDVQQGEwJVUzELMAkG\n"
        "A1UECAwCQ0ExETAPBgNVBAcMCFNhbiBKb3NlMREwDwYDVQQKDAhRdWFsY29tbTEM\n"
        "MAoGA1UECwwDSW9UMQ8wDQYDVQQDDAZzZXJ2ZXIwggEiMA0GCSqGSIb3DQEBAQUA\n"
        "A4IBDwAwggEKAoIBAQC+9RSiGwaEkIcTGMmUiJP/vvR8B+57yO/FKboH0OZLKEe4\n"
        "ILqest7pOxpFff/AB//BmSJrGPByxYMRnZ6tagCNRJe+cG59Z9Pypfp6HJgB9B+r\n"
        "l5lQNKY9l9JtSBr99oHinPs0bdgp+sQ8h2g89fRRBVTTZSvO7sPsa/jsiSZT74sL\n"
        "uCUmsd9YLApWFDvHXBNmJOH0lghhC55rK94nIvbK/NN6KSEojEnQEbXd/sf/uUm+\n"
        "H8DkUn5Miy2nC6fupZ9gD6TKIXqqQzWlRp3ktW/1zQvRa26m7trqi8X0CL+UbszN\n"
        "jp4eq7oyRBGhRrY4XUyz1n4doU+aaJVPuRc1DBqVAgMBAAEwDQYJKoZIhvcNAQEL\n"
        "BQADggEBAFvfrJSjDVoEJOpgDu4FwLGrZnqjwAYYD3AMYmgc/zo6xoyybL0ChgqR\n"
        "cnyQdFTy+xMBeIVnPUvN9pjXOn0jYW1i1oASWdhIopodfL8rqWIajAG0Y9fDZ7m6\n"
        "Y4kj/vgPkQGHLZlCEVgxD4fOSo7xwV6LU5YV5dt+hFaIzrrnZa8oHvGC3zXH/D+k\n"
        "KZ6w5XuOta1GOUsypDL1AKguz65hY4Oy+ZcZ3+iSi5QZL5FXUBV4LaDtf/8gBanl\n"
        "AHBLMYWoHfnvtZqOgGjo82HJqf01qCP0apOf/7tFwsbW9kTVJVbXTCDlBgj0NAvP\n"
        "M26ZlKfVNcx/cyY/tcAPd0dr+SF6pJw=\n"
        "-----END CERTIFICATE-----\n"
        "-----BEGIN CERTIFICATE-----\n"
        "MIIDQzCCAisCCQCqAzT8nyI5cTANBgkqhkiG9w0BAQsFADBfMQswCQYDVQQGEwJV\n"
        "UzELMAkGA1UECAwCQ0ExETAPBgNVBAcMCFNhbiBKb3NlMREwDwYDVQQKDAhRdWFs\n"
        "Y29tbTEMMAoGA1UECwwDSW9UMQ8wDQYDVQQDDAZyb290Q0EwHhcNMTcwMjIzMDY0\n"
        "NzMyWhcNMTgwNzA4MDY0NzMyWjBoMQswCQYDVQQGEwJVUzELMAkGA1UECAwCQ0Ex\n"
        "ETAPBgNVBAcMCFNhbiBKb3NlMREwDwYDVQQKDAhRdWFsY29tbTEMMAoGA1UECwwD\n"
        "SW9UMRgwFgYDVQQDDA9pbnRlcm1lZGlhdGVDQTAwggEiMA0GCSqGSIb3DQEBAQUA\n"
        "A4IBDwAwggEKAoIBAQCvSjdB3uSx9WRaHEB5qHvkT3qjDLR0r0HxtN/vEq2JIpVR\n"
        "lH2NhHoQfCtlmGi82yJiPa4yZrkidIjIo+oXnKYVxd2ILffvWE9hgGUY4cn+Gmg3\n"
        "2L96SXGY53N+uHP2EbjA544vTkN/MMiQ7BxsvmW4UVN4LXmKkiXKctebqZEElfuu\n"
        "r78q5mNFPKHC2MNgU8NYdUTUCl2vXiJ1LJYs74menLJi8I/Vcerf9o8Q95XqF/R/\n"
        "lC49CmrVTH0v1iLYv8pIZcoB1O1+py/5fliipyAvoTazdScotOzD2BIBTYtA7YxF\n"
        "8B5OJ0mRYM+RcaPy5sMdesgDHFLMonccF8ViuaMhAgMBAAEwDQYJKoZIhvcNAQEL\n"
        "BQADggEBAF15sTmxkfZqUK0XGVi74RrU/8PzgYWeWeUfasrU+dIfxLkOsDadx4rO\n"
        "hn5/6Zx2NWsS3zQb3orE89OdYjYQsE3Mo82ouXMbBCBtXr86uNNUkBT30dOmQJeB\n"
        "hyWfRRLhjlGT5k8KYynmWCds3RhSjxokwcoUO8EdD85Llg5Dr+tO7Mi3LCyW+9JQ\n"
        "y18PAk6cFBkfZEZHoh5PGmjcsJnx6fQx49z57v8cbj2ecZ5MiLmMYnWE7NnJb+rA\n"
        "a6vr5hhiDjdcGt6snnoC+T9k/d/u+v8XA1C6ulUH/cqpqPG8EltWfZKnc3NRaFXh\n"
        "gjeONR4VWQBpkH31M+6n1PQ4nS47aJQ=\n"
        "-----END CERTIFICATE-----\n";

/* CN=server signed by rootCA */
const char peer_cert[] =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIDOjCCAiICCQCzu0oC66/gBzANBgkqhkiG9w0BAQsFADBfMQswCQYDVQQGEwJV\n"
        "UzELMAkGA1UECAwCQ0ExETAPBgNVBAcMCFNhbiBKb3NlMREwDwYDVQQKDAhRdWFs\n"
        "Y29tbTEMMAoGA1UECwwDSW9UMQ8wDQYDVQQDDAZyb290Q0EwHhcNMTgwMzMwMDAz\n"
        "OTU2WhcNMjAxMjI0MDAzOTU2WjBfMQswCQYDVQQGEwJVUzELMAkGA1UECAwCQ0Ex\n"
        "ETAPBgNVBAcMCFNhbiBKb3NlMREwDwYDVQQKDAhRdWFsY29tbTEMMAoGA1UECwwD\n"
        "SW9UMQ8wDQYDVQQDDAZzZXJ2ZXIwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n"
        "AoIBAQC3Pl9m9T2u8rc3YmHiJjYvjDaUvJIuaSRnKSe85xnn1bmTm9ycgWRkhquu\n"
        "qcyPLrpqYayvghq82BOYzA9QgY/EWC7IDGTo7LrPOn7P5Rv5RBqUTcOOD7B3EjGq\n"
        "mF8TGZUh6o1ZH5ao2pfXIlKubKkf0vdzFZrC2SZVHx+x0z8cgyQ3HXfUTe2Vz8b+\n"
        "qNUiIu6oltTyqFkMTACzOT8hYYJlCr1VYl/SPM+S1xQaYWRTE2MvMFOMWGENspmP\n"
        "VIo2JXu2rDUydLifKvk5oErcMKQf7GN5CsyfwNHL0hccaCBydVo92sUqW8lgC1WP\n"
        "B8e+eNGRrI+K6WXjMQbwANlISXSjAgMBAAEwDQYJKoZIhvcNAQELBQADggEBAK4J\n"
        "KkJPn7ugp77KyYP5dmJTolR3Vrb3AF/2DSZDRJ2JCTM2WCdu07Uu8/qX+efeWMfG\n"
        "iSLFa+goVSHARo6Ldy/Khi9oE6+G6OMrS/wHNVn+TxtiC6x+fOi5HcdLD2tSGsqA\n"
        "wS1db1wasbBxszltCiEAjXeZX7VG1+109jIWw7WPi7O8N1G/eonXooaexKIzOb6Y\n"
        "1ZGkzTU4YLHAUMwsIb+6UeYQ8jWX/GgNNFuWbJYuP6OI3ts4Fn0Xm9zsFRuJLodU\n"
        "AlPBSKU4BG3WntD9cJttjT5tRKCUHK1FXHszKRx/qWfFgVeEXavseyaF2k3EDIpo\n"
        "c1OvZBZW16QXMYs4kMg=\n"
        "-----END CERTIFICATE-----\n";

/* CN=server signed by rootCA */
const char peer_cert_ecc[] =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIBnDCCAUkCCQDuDbRV5SwiEzAKBggqhkjOPQQDAjBfMQswCQYDVQQGEwJVUzEL\n"
        "MAkGA1UECAwCQ0ExETAPBgNVBAcMCFNhbiBKb3NlMREwDwYDVQQKDAhRdWFsY29t\n"
        "bTEMMAoGA1UECwwDSW9UMQ8wDQYDVQQDDAZyb290Q0EwHhcNMTgwMzI4MTk1NTA2\n"
        "WhcNMjAxMjIyMTk1NTA2WjBfMQswCQYDVQQGEwJVUzELMAkGA1UECAwCQ0ExETAP\n"
        "BgNVBAcMCFNhbiBKb3NlMREwDwYDVQQKDAhRdWFsY29tbTEMMAoGA1UECwwDSW9U\n"
        "MQ8wDQYDVQQDDAZzZXJ2ZXIwTjAQBgcqhkjOPQIBBgUrgQQAIQM6AARCVCV6QHUV\n"
        "lCeXGG0t7cvJ6dmorX+OuXdkBVEteu66D1rYKJJj6retNWt9cs2z3qv+kJI0JRfj\n"
        "ejAKBggqhkjOPQQDAgNBADA+Ah0Ak+7/2qmeWDkSNJymLQ5N+D9VKxHSdmkc+cpK\n"
        "hwIdAPKsihPimHSaZZT4lJ8SNTVIz2S/Lvzqs+Ih6O8=\n"
        "-----END CERTIFICATE-----\n";

const char peer_ecc_key[] =
        "-----BEGIN EC PARAMETERS-----\n"
        "BgUrgQQAIQ==\n"
        "-----END EC PARAMETERS-----\n"
        "-----BEGIN EC PRIVATE KEY-----\n"
        "MGgCAQEEHFr1zQrKrkjgbdfb44t+qDJhblf5CAqAEw7c+WqgBwYFK4EEACGhPAM6\n"
        "AARCVCV6QHUVlCeXGG0t7cvJ6dmorX+OuXdkBVEteu66D1rYKJJj6retNWt9cs2z\n"
        "3qv+kJI0JRfjeg==\n"
        "-----END EC PRIVATE KEY-----\n";

static void cert_validation_parse_error(qapi_Status_t error, uint32_t negative, uint32_t key_usage_purposes)
{
    switch(error)
    {
        case QAPI_ERR_SSL_CERT_NONE:   // name NOTOK, time NOTOK
            CERTCS_PRINTF("%s: Bad certificate, or both time and domain are invalid\n", negative ? "Success":"Failure");
            break;

        case QAPI_ERR_SSL_CERT_TIME:   // name NOTOK, time    OK
            CERTCS_PRINTF("%s: Mismatching domain name in certificate\n", negative ? "Success":"Failure");
            break;

        case QAPI_ERR_SSL_CERT_CN:     // name    OK, time NOTOK
            CERTCS_PRINTF("%s: Certificate expired\n", negative ? "Success":"Failure");
            break;

        case QAPI_OK:       // name    OK, time    OK
            CERTCS_PRINTF("%s: Certificate validated\n", negative ? "Failure":"Success");
            CERTCS_PRINTF("Certificate Key Usage Purposes flags: 0x%08X\n", key_usage_purposes);
            break;

        default:
            CERTCS_PRINTF("%s: Error code %d\n", negative ? "Success":"Failure", error);
            break;
    }
}

static int cert_validation_load_buffer(
        const char * filename,
        uint8_t ** pp_buffer,
        size_t * p_buffer_size)
{
    qapi_Status_t status = QAPI_ERROR;
    int fd = 0;
    uint8_t *p_buffer = NULL;
    struct qapi_fs_stat_type stat;

    status = qapi_Fs_Stat(filename, &stat);
    if ( QAPI_OK != status ) {
        CERTCS_PRINTF("Failed on a call to qapi_Fs_Stat()\r\n");
        goto cleanup;
    }

    p_buffer = (uint8_t*)malloc(stat.st_size);
    if(p_buffer == NULL) {
        CERTCS_PRINTF("Failed to allocate memory.\r\n");
        goto cleanup;
    }

    status = qapi_Fs_Open(filename, QAPI_FS_O_RDONLY, &fd);
    if ( QAPI_OK != status ) {
        CERTCS_PRINTF("Failed on a call to qapi_Fs_Open()\r\n");
        goto cleanup;
    }

    uint32_t bytes_read = 0;
    status = qapi_Fs_Read(fd, p_buffer, stat.st_size, &bytes_read);
    if( (QAPI_OK != status) || (bytes_read != stat.st_size) ) {
        CERTCS_PRINTF("Failed to read file (status=%d).\r\n", status);
        *pp_buffer = NULL;
        *p_buffer_size = 0;
    } else {
        *pp_buffer = p_buffer;
        *p_buffer_size = (size_t)stat.st_size;
    }

    qapi_Fs_Close(fd);

cleanup:
    if( (status != QAPI_OK) && (p_buffer != NULL) ) {
        free(p_buffer);
    }

    return status;
}

QCLI_Command_Status_t cert_expiration(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_Net_SSL_Cert_Expiration_t cert_expiration;
    char *cert_name = NULL;
    qapi_Status_t status;
    uint8_t internal = 0;
    uint32_t parameter_index;

    if(Parameter_Count < 1)
    {
        goto cert_expiration_usage;
    }

    for ( parameter_index = 0; parameter_index < Parameter_Count; parameter_index++ )
    {
        if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-r") ) // Specify certificate filename, optional
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
                CERTCS_PRINTF("ERROR: -r option is present, but no certificate filename is specified\r\n");
                return QCLI_STATUS_USAGE_E;
            }
            cert_name = (char*)Parameter_List[parameter_index].String_Value;
            CERTCS_PRINTF("Testing certificate: %s\n", cert_name);
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-i") ) // Specify internal certificate
        {
        	CERTCS_PRINTF("Testing internal certificate\n");
            internal = 1;
        }
        else
        {
            CERTCS_PRINTF("ERROR: Unknown option (%s)\r\n", (char*)Parameter_List[parameter_index].String_Value);
            return QCLI_STATUS_USAGE_E;
        }
    }

    if((!cert_name && !internal) ||
    		(cert_name && internal))
    {
    	goto cert_expiration_usage;
    }

    if(!cert_name)
    {
        qapi_Net_SSL_Cert_Info_t cert_info;

        cert_name = "tmp_cert";
        memset(&cert_info, 0, sizeof(cert_info));
        cert_info.cert_Type = QAPI_NET_SSL_PEM_CERTIFICATE_WITH_PEM_KEY_E;
        cert_info.info.pem_Cert.cert_Buf = (uint8_t *)peer_cert_ecc;
        cert_info.info.pem_Cert.cert_Size = sizeof(peer_cert_ecc);
        cert_info.info.pem_Cert.key_Buf = (uint8_t *)peer_ecc_key;
        cert_info.info.pem_Cert.key_Size = sizeof(peer_ecc_key);
        status = qapi_Net_SSL_Cert_Store(&cert_info, cert_name);
        if (QAPI_OK != status)
        {
            CERTCS_PRINTF("ERROR: Failed to store internal certificate\n");
            return QCLI_STATUS_ERROR_E;
        }
    }

    /* Get expiration details */
    if(qapi_Net_SSL_Cert_Get_Expiration(cert_name, QAPI_NET_SSL_CERTIFICATE_E, &cert_expiration) != QAPI_OK)
    {
        qapi_Fs_Unlink("/spinor/ssl/certs/tmp_cert");
        CERTCS_PRINTF("ERROR: Failed to get expiration of %s\n", cert_name);
        return QCLI_STATUS_ERROR_E;
    }

    /*
     * Expected output for the internal certificate:
     *
     * Common Name (CN): server
     * Issuer Common Name (CN): rootCA
     * Not Before: 3/28/2018 19:02:06
     * Not After: 12/22/2020 19:11:06
     */

    /* Display information */
    CERTCS_PRINTF("Common Name (CN): %s\n", cert_expiration.common_Name);
    CERTCS_PRINTF("Issuer Common Name (CN): %s\n", cert_expiration.issuer_Common_Name);
    CERTCS_PRINTF("Not Before: %d/%d/%d %d:%02d:%02d\n",
            cert_expiration.not_Before.month, cert_expiration.not_Before.day, cert_expiration.not_Before.year,
            cert_expiration.not_Before.hour, cert_expiration.not_Before.month, cert_expiration.not_Before.second);

    CERTCS_PRINTF("Not After: %d/%d/%d %d:%02d:%02d\n",
            cert_expiration.not_After.month, cert_expiration.not_After.day, cert_expiration.not_After.year,
            cert_expiration.not_After.hour, cert_expiration.not_After.month, cert_expiration.not_Before.second);

    /* Erase the temporary certificate (pointer equality check) */
    if(internal)
        qapi_Fs_Unlink("/spinor/ssl/certs/tmp_cert");

    return QCLI_STATUS_SUCCESS_E;

    cert_expiration_usage:
	CERTCS_PRINTF("usage: cert expiration [-i ] [-r <cert_name>]\n");
	CERTCS_PRINTF("Use -i to unit-test internal certificate, or -r to specify a filename in cert storage\n");

	return QCLI_STATUS_ERROR_E;
}

QCLI_Command_Status_t cert_validation(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qapi_Status_t ret;
    const char *CA_list_name = "rootCA";
    const char *CA_list = NULL;
    qapi_Net_SSL_Cert_Info_t cert_info;
    qapi_CA_Info_t ca_info;
    int parameter_index;
    char *common_name = NULL;
    uint32_t internal_ca = 0;
    char *cert_name = NULL;
    char *remote_cert_name = NULL;
    uint32_t key_usage_purposes;
    char *host_name = NULL;

    if(Parameter_Count == 0)
    	goto cert_validation_usage;

    for ( parameter_index = 0; parameter_index < Parameter_Count; parameter_index++ )
    {
        if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-c") ) // Specify RootCA filename, optional
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
                CERTCS_PRINTF("ERROR: -c option is present, but no CA list filename specified\r\n");
                return QCLI_STATUS_USAGE_E;
            }
            CA_list = (const char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-n") ) // Specify CN from command line, optional
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
                CERTCS_PRINTF("ERROR: -n option is present, but no Common Name is specified\r\n");
                return QCLI_STATUS_USAGE_E;
            }
            common_name = (char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-r") ) // Specify certificate filename, optional
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
                CERTCS_PRINTF("ERROR: -r option is present, but no certificate filename is specified\r\n");
                return QCLI_STATUS_USAGE_E;
            }
            cert_name = (char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-f") ) // Specify certificate filename, optional
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
                CERTCS_PRINTF("ERROR: -f option is present, but no remote certificate filename is specified\r\n");
                return QCLI_STATUS_USAGE_E;
            }
            remote_cert_name = (char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-h") ) // Specify certificate filename, optional
        {
            parameter_index++;
            if ( parameter_index == Parameter_Count ) {
                CERTCS_PRINTF("ERROR: -h option is present, but no remote host is specified\r\n");
                return QCLI_STATUS_USAGE_E;
            }
            host_name = (char*)Parameter_List[parameter_index].String_Value;
        }
        else if ( 0 == strcmp((char*)Parameter_List[parameter_index].String_Value, "-i") ) // Specify internal certificate
        {
        	CERTCS_PRINTF("Testing internal certificate\n");
            internal_ca = 1;
        }
        else
        {
            CERTCS_PRINTF("ERROR: Unknown option (%s)\r\n", (char*)Parameter_List[parameter_index].String_Value);
            goto cert_validation_usage;
        }
    }

    if(internal_ca && (remote_cert_name || cert_name || CA_list))
    {
        goto cert_validation_usage;
    }

    if(!remote_cert_name)
    {
		if((cert_name && !CA_list) ||
				(CA_list && !cert_name))
		{
			CERTCS_PRINTF("ERROR: -c and -r options must be used together, either use internal certs or use external ones\n");
			return QCLI_STATUS_USAGE_E;
		}
    }
    else /* -f option */
    {
    	if(!CA_list)
    	{
			CERTCS_PRINTF("ERROR: -c and -f options must be used together, either use internal certs or use external ones\n");
			return QCLI_STATUS_USAGE_E;
    	}

    	if(cert_name)
        {
            CERTCS_PRINTF("ERROR: -f and -r options cannot be used together\n");
            return QCLI_STATUS_USAGE_E;
        }

    	if(!host_name)
    	{
            CERTCS_PRINTF("ERROR: Specify host name -h option must be used with -f option\n");
            return QCLI_STATUS_USAGE_E;
    	}

    }

    /* If no CN was provided and not using a custom cert, set to 'server' */
    if(!cert_name && !common_name)
    {
        common_name = "server";
    }

    if(!common_name)
    {
        CERT_PRINTF("No common name to verify specified, accepting all\n");
    }
    else
    {
        CERT_PRINTF("Common Name (CN) to verify: '%s'\n", common_name);
    }

    if(CA_list && CA_list[0] && !internal_ca)
        CERT_PRINTF("CA List file: %s\n", CA_list);
    else
    {
        CERT_PRINTF("Using an internal CA list\n");
        CA_list = CA_list_name;
    }

    if(remote_cert_name && !internal_ca)
    {
        size_t peer_cert_buffer_size = 0;
        uint8_t *peer_cert_buffer = NULL;

        CERT_PRINTF("Downloading %s...\n", remote_cert_name);
        if(certcs_download_file(host_name, 1443, remote_cert_name, &peer_cert_buffer, (uint32_t *)&peer_cert_buffer_size) < 0)
        {
            CERT_PRINTF("ERROR: failed to download %s\n", remote_cert_name);
            return QCLI_STATUS_ERROR_E;
        }

        CERT_PRINTF("Testing %s, size = %dbytes\n", remote_cert_name, peer_cert_buffer_size);
        ret = qapi_Net_SSL_Cert_Validate((const char *)peer_cert_buffer, peer_cert_buffer_size, common_name, CA_list, &key_usage_purposes);
        cert_validation_parse_error(ret, 0, key_usage_purposes);

        /* Need to free the temporary buffer once we are done */
        free(peer_cert_buffer);
    }
    else
    /* If user specified a cert file, verify it. Otherwise, use the internal buffer */
    if(cert_name && !internal_ca)
    {
        size_t peer_cert_buffer_size = 0;
        uint8_t *peer_cert_buffer = NULL;

        if(cert_validation_load_buffer(cert_name, &peer_cert_buffer, &peer_cert_buffer_size) < 0)
        {
            goto cert_cleanup;
        }

        CERT_PRINTF("Testing %s, size = %dbytes\n", cert_name, peer_cert_buffer_size);
        ret = qapi_Net_SSL_Cert_Validate((const char *)peer_cert_buffer, peer_cert_buffer_size, common_name, CA_list, &key_usage_purposes);
        cert_validation_parse_error(ret, 0, key_usage_purposes);

        /* Need to free the temporary buffer once we are done */
        free(peer_cert_buffer);
    }
    else
    {
        qapi_Time_t sys_tm, test_tm;

        /* For the purpose of this test, set the platform time to a date and time that
         * is within the expiration of the certificates, April 1st 2018.
         */
        qapi_Core_RTC_Get(&sys_tm);

        memset(&test_tm, 0, sizeof(qapi_Time_t));
        test_tm.year = 2018;
        test_tm.month = 4;
        test_tm.day = 1;
        if(qapi_Core_RTC_Set(&test_tm) != QAPI_OK)
        {
            CERT_PRINTF("ERROR: Cannot set system time which is needed for the internal certificates\n");
            goto cert_cleanup;
        }

        /* Create a new CA list file, RSA */
        memset(&cert_info, 0, sizeof(cert_info));
        ca_info.ca_Buf = (uint8_t *)root_ca;
        ca_info.ca_Size = sizeof(root_ca);
        cert_info.cert_Type = QAPI_NET_SSL_PEM_CA_LIST_E;
        cert_info.info.pem_CA_List.ca_Cnt = 1;
        cert_info.info.pem_CA_List.ca_Info[0] = &ca_info;
        ret = qapi_Net_SSL_Cert_Store(&cert_info, CA_list_name);
        if ( QAPI_OK != ret ) {
            CERT_PRINTF("ERROR: failed to convert and store ca_list, status=%d\n", ret);
            return QCLI_STATUS_ERROR_E;
        }

        CERT_PRINTF("Testing peer_cert, size = %dbytes\n", sizeof(peer_cert)-1);
        ret = qapi_Net_SSL_Cert_Validate(peer_cert, sizeof(peer_cert)-1, common_name, CA_list, &key_usage_purposes);
        cert_validation_parse_error(ret, 0, key_usage_purposes);

        CERT_PRINTF("Testing peer_cert_with_intermediate_ca, size = %dbytes\n", sizeof(peer_cert_with_intermediate_ca)-1);
        ret = qapi_Net_SSL_Cert_Validate(peer_cert_with_intermediate_ca, sizeof(peer_cert_with_intermediate_ca)-1, common_name, CA_list, &key_usage_purposes);
        cert_validation_parse_error(ret, 0, key_usage_purposes);

        /* Change the time to the future (from my current 2018 point of view :-)) */
        memset(&test_tm, 0, sizeof(qapi_Time_t));
        test_tm.year = 2025;
        test_tm.month = 4;
        test_tm.day = 1;
        if(qapi_Core_RTC_Set(&test_tm) != QAPI_OK)
        {
            CERT_PRINTF("ERROR: Cannot set system time which is needed for the internal certificates\n");
            goto cert_cleanup;
        }

        CERT_PRINTF("Negative testing for expiration of peer_cert, size = %dbytes\n", sizeof(peer_cert)-1);
        ret = qapi_Net_SSL_Cert_Validate(peer_cert, sizeof(peer_cert)-1, common_name, CA_list, &key_usage_purposes);
        cert_validation_parse_error(ret, 1, 0);

        /* Unlink the temporary CA list file */
        qapi_Fs_Unlink(CA_list_name);

        /* Create a new CA list file, ECC */
        memset(&cert_info, 0, sizeof(cert_info));
        ca_info.ca_Buf = (uint8_t *)root_ca_ecc;
        ca_info.ca_Size = sizeof(root_ca_ecc);
        cert_info.cert_Type = QAPI_NET_SSL_PEM_CA_LIST_E;
        cert_info.info.pem_CA_List.ca_Cnt = 1;
        cert_info.info.pem_CA_List.ca_Info[0] = &ca_info;
        ret = qapi_Net_SSL_Cert_Store(&cert_info, CA_list_name);
        if ( QAPI_OK != ret ) {
            CERT_PRINTF("ERROR: failed to convert and store ca_list, status=%d\n", ret);
            return QCLI_STATUS_ERROR_E;
        }

        memset(&test_tm, 0, sizeof(qapi_Time_t));
        test_tm.year = 2018;
        test_tm.month = 4;
        test_tm.day = 1;
        if(qapi_Core_RTC_Set(&test_tm) != QAPI_OK)
        {
            CERT_PRINTF("ERROR: Cannot set system time which is needed for the internal certificates\n");
            goto cert_cleanup;
        }

        CERT_PRINTF("Testing peer_cert_ecc, size = %dbytes\n", sizeof(peer_cert_ecc)-1);
        ret = qapi_Net_SSL_Cert_Validate(peer_cert_ecc, sizeof(peer_cert_ecc)-1, common_name, CA_list, &key_usage_purposes);
        cert_validation_parse_error(ret, 0, key_usage_purposes);

        /* Change the time to the future (from my current 2018 point of view :-)) */
        memset(&test_tm, 0, sizeof(qapi_Time_t));
        test_tm.year = 2025;
        test_tm.month = 4;
        test_tm.day = 1;
        if(qapi_Core_RTC_Set(&test_tm) != QAPI_OK)
        {
            CERT_PRINTF("ERROR: Cannot set system time which is needed for the internal certificates\n");
            goto cert_cleanup;
        }

        CERT_PRINTF("Negative testing for expiration of peer_cert_ecc, size = %dbytes\n", sizeof(peer_cert_ecc)-1);
        ret = qapi_Net_SSL_Cert_Validate(peer_cert_ecc, sizeof(peer_cert_ecc)-1, common_name, CA_list, &key_usage_purposes);
        cert_validation_parse_error(ret, 1, 0);

        /* Restore system time to previous value, there will be some gap though. */
        qapi_Core_RTC_Set(&sys_tm);

        /* Unlink the temporary CA list file */
        qapi_Fs_Unlink(CA_list_name);
    }

    if(internal_ca && !cert_name)
    {
        char *bad_cert;

        CERT_PRINTF("Using an unknown internal CA list\n");

        memset(&cert_info, 0, sizeof(cert_info));
        ca_info.ca_Buf = (uint8_t *)root_ca_unknown;
        ca_info.ca_Size = sizeof(root_ca_unknown);
        cert_info.cert_Type = QAPI_NET_SSL_PEM_CA_LIST_E;
        cert_info.info.pem_CA_List.ca_Cnt = 1;
        cert_info.info.pem_CA_List.ca_Info[0] = &ca_info;
        ret = qapi_Net_SSL_Cert_Store(&cert_info, CA_list_name);
        if ( QAPI_OK != ret ) {
            CERT_PRINTF("ERROR: failed to convert and store ca_list, status=%d\n", ret);
            return QCLI_STATUS_ERROR_E;
        }
        CA_list = CA_list_name;

        /* Perform negative tests only if using internal certs */
        CERT_PRINTF("Negative testing peer_cert, size = %dbytes\n", sizeof(peer_cert)-1);
        ret = qapi_Net_SSL_Cert_Validate(peer_cert, sizeof(peer_cert)-1, common_name, CA_list, &key_usage_purposes);
        cert_validation_parse_error(ret, 1, 0);

        /* Additional negative tests */
        bad_cert = malloc(sizeof(peer_cert_ecc));

        if(bad_cert)
        {
            CERT_PRINTF("Additional negative tests:\n");

            strcpy(bad_cert, "-----BEGIN CERTIFICATE-----\n");
            strcat(bad_cert, "-----END CERTIFICATE-----");
            ret = qapi_Net_SSL_Cert_Validate(bad_cert, strlen(bad_cert), common_name, CA_list, NULL);
            cert_validation_parse_error(ret, 1, 0);

            strcpy(bad_cert, "-----BEGIN CERTIFICATE-----\n");
            strcat(bad_cert, "MIIBnDCCAUkCCQDuDbRV5SwiEzAKBggqhkjOPQQDAjBfMQswCQYDVQQGEwJVUzEL\n");
            ret = qapi_Net_SSL_Cert_Validate(bad_cert, strlen(bad_cert), common_name, CA_list, NULL);
            cert_validation_parse_error(ret, 1, 0);

            strcpy(bad_cert, "-----BEGIN CERTIFICATE-----\n");
            strcat(bad_cert, "MIIBnDCCAUkCCQDuDbRV5SwiEzAKBggqhkjOPQQDAjBfMQswCQYDVQQGEwJVUzEL\n");
            strcat(bad_cert, "-----END CERTIFICATE-----");
            ret = qapi_Net_SSL_Cert_Validate(bad_cert, strlen(bad_cert), common_name, CA_list, NULL);
            cert_validation_parse_error(ret, 1, 0);

            strcpy(bad_cert, "-----BEGIN CERTIFICATE-----\n");
            strcat(bad_cert, "MjfgsFYosjy%SRHOJHwrHEOMHSIRy*wsroy4sF#morsMHORMhs@omhSROHMoshsROM\n");
            strcat(bad_cert, "-----END CERTIFICATE-----");
            ret = qapi_Net_SSL_Cert_Validate(bad_cert, strlen(bad_cert), common_name, CA_list, NULL);
            cert_validation_parse_error(ret, 1, 0);

            strcpy(bad_cert, "metHETOKO%EmtmetdkmzW$Y40yjS%YLMSDG(JSytsgsg4$MG)smvesj56tS$Eyk09\n");
            ret = qapi_Net_SSL_Cert_Validate(bad_cert, strlen(bad_cert), common_name, CA_list, NULL);
            cert_validation_parse_error(ret, 1, 0);

            free(bad_cert);
        }

        /* Unlink the temporary file */
        qapi_Fs_Unlink(CA_list_name);
    }

    cert_cleanup:

    return QCLI_STATUS_SUCCESS_E;

    cert_validation_usage:
	CERT_PRINTF("usage: cert validate [-c <ca_list_name>] [-n <common_name>] [-r <cert_name>] [-f <cert_name> -h <host_name>] [-i]\n");
	CERT_PRINTF("-c CA list name in cert store\n-n Common Name to test, default: 'server'\n-r Certificate name in flash, or\n");
	CERT_PRINTF("-f Certificate name to download from host name specified by -h, or\n-i to run internal unit-tests\n");
	return QCLI_STATUS_ERROR_E;
}

#endif
