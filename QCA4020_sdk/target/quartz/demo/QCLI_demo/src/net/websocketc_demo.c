/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
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
#include "qurt_thread.h"
#include "qurt_error.h"
#include "qapi_websocket.h"
#include "qapi_delay.h"
#include "qapi_ssl.h"

#ifdef CONFIG_NET_WEBSOCKETC_DEMO
extern QCLI_Group_Handle_t qcli_net_handle; /* Handle for Net Command Group. */

#define WEBSOCKET_PRINTF(...) QCLI_Printf(qcli_net_handle, __VA_ARGS__)

#define WEBSOCKETC_DEMO_MAX_CLIENTS (4)
#define MIN(a, b) (a < b ? a : b)

#define DUMP_DIRECTION_TX	(0)
#define DUMP_DIRECTION_RX	(1)
void bench_print_buffer(const char *buf, uint32_t len,
		struct sockaddr *sock_addr, uint8_t direction);

uint32_t g_max_size_to_print = 128;
uint32_t g_websocketc_demo_tx_timeout_us = 10000;
uint32_t g_websocketc_demo_tx_retry_delay_us = 500;

#define WEBSOCKET_DEMO_MAX_SUBPROTOCOLS (4)

typedef struct websocket_client_demo_s {
	qapi_Net_Websocket_Hdl_t websocket_handle;
	qapi_Net_SSL_Obj_Hdl_t ssl_obj_handle;
	uint32_t client_number;
	qbool_t echo_mode;
	qbool_t tx_dump_enabled;
	qbool_t rx_dump_enabled;
} websocketc_demo_ctxt_t;

websocketc_demo_ctxt_t g_websocket_client_demo[WEBSOCKETC_DEMO_MAX_CLIENTS];

void websocketc_demo_help() {
	WEBSOCKET_PRINTF("websocketc new\n");
	WEBSOCKET_PRINTF("\t-o  <origin>\n");
	WEBSOCKET_PRINTF("\t-k  <subprotocol> (can appear more than once)\n");
	WEBSOCKET_PRINTF("\t-s  <enable ssl = 1 or 0>\n");
	WEBSOCKET_PRINTF("\t-l  CA list name\n");
	WEBSOCKET_PRINTF("\t-c  certificate name\n");
	WEBSOCKET_PRINTF("\t-t  <handshake_timeout ms>\n");
	WEBSOCKET_PRINTF("\t-z  <closing_timeout ms>\n");

	WEBSOCKET_PRINTF("websocketc destroy client_id\n");

	WEBSOCKET_PRINTF("websocket sslconfig client_id\n");
	WEBSOCKET_PRINTF("\tprotocol <version, e.g. TLS1.2>\n");
	WEBSOCKET_PRINTF("\ttime <0 for no cert time check or 1>\n");
	WEBSOCKET_PRINTF("\tcipher <cipher>\n");

	WEBSOCKET_PRINTF(
			"websocketc addhttpheader client_id <header_name> <header_value>\n");

	WEBSOCKET_PRINTF("websocketc resethttpheaders client_id\n");

	WEBSOCKET_PRINTF("websocketc connect client_id\n");
	WEBSOCKET_PRINTF("\t-h  <host>\n");
	WEBSOCKET_PRINTF("\t-p  <port>\n");
	WEBSOCKET_PRINTF("\t-r  <resource_path>\n");

	WEBSOCKET_PRINTF("websocketc close client_id\n");

	WEBSOCKET_PRINTF("websocketc dump client_id <tx/rx>  <on/off>\n");

	WEBSOCKET_PRINTF("websocketc echo client_id <on/off>\n");

	WEBSOCKET_PRINTF("websocketc ping client_id\n");
	WEBSOCKET_PRINTF("\t-d  <data>\n");

	WEBSOCKET_PRINTF("websocketc pong client_id\n");
	WEBSOCKET_PRINTF("\t-d  <data>\n");

	WEBSOCKET_PRINTF("websocketc send client_id\n");
	WEBSOCKET_PRINTF("\t-m  <message>\n");
	WEBSOCKET_PRINTF("\t-e  <end_of_message = 1 or 0>\n");
}

qapi_Status_t websocketc_demo_print_statistics(
		websocketc_demo_ctxt_t *p_demo_client_ctxt) {
	WEBSOCKET_PRINTF("Client %d websocket statistics\n",
			p_demo_client_ctxt->client_number);

	uint32_t stat;
	size_t stat_size = sizeof(stat);

	if (qapi_Net_Websocket_Get_Opt(p_demo_client_ctxt->websocket_handle,
			QAPI_NET_WEBSOCKET_OPTION_NUM_RX_BYTES, &stat,
			&stat_size) != QAPI_OK) {
		return QAPI_ERROR;
	}
	WEBSOCKET_PRINTF("\t%16s: %8d\n", "Bytes received", stat);

	if (qapi_Net_Websocket_Get_Opt(p_demo_client_ctxt->websocket_handle,
			QAPI_NET_WEBSOCKET_OPTION_NUM_TX_BYTES, &stat,
			&stat_size) != QAPI_OK) {
		return QAPI_ERROR;
	}
	WEBSOCKET_PRINTF("\t%16s: %8d\n", "Bytes sent", stat);

	if (qapi_Net_Websocket_Get_Opt(p_demo_client_ctxt->websocket_handle,
			QAPI_NET_WEBSOCKET_OPTION_NUM_RX_PINGS, &stat,
			&stat_size) != QAPI_OK) {
		return QAPI_ERROR;
	}
	WEBSOCKET_PRINTF("\t%16s: %8d\n", "Pings received", stat);

	if (qapi_Net_Websocket_Get_Opt(p_demo_client_ctxt->websocket_handle,
			QAPI_NET_WEBSOCKET_OPTION_NUM_TX_PONGS, &stat,
			&stat_size) != QAPI_OK) {
		return QAPI_ERROR;
	}
	WEBSOCKET_PRINTF("\t%16s: %8d\n", "Pongs sent", stat);

	if (qapi_Net_Websocket_Get_Opt(p_demo_client_ctxt->websocket_handle,
			QAPI_NET_WEBSOCKET_OPTION_NUM_TX_PINGS, &stat,
			&stat_size) != QAPI_OK) {
		return QAPI_ERROR;
	}
	WEBSOCKET_PRINTF("\t%16s: %8d\n", "Pings sent", stat);

	if (qapi_Net_Websocket_Get_Opt(p_demo_client_ctxt->websocket_handle,
			QAPI_NET_WEBSOCKET_OPTION_NUM_RX_PONGS, &stat,
			&stat_size) != QAPI_OK) {
		return QAPI_ERROR;
	}
	WEBSOCKET_PRINTF("\t%16s: %8d\n", "Pongs received", stat);

	return QAPI_OK;
}

int websocketc_demo_send_retry(websocketc_demo_ctxt_t *p_demo_client_ctxt,
		QAPI_Net_Websocket_Data_Type_t type, const uint8_t *data,
		size_t data_length, qbool_t end_of_message) {

	int total_bytes_sent = 0;
	int time_spent = 0;
	do {
		// Send may only send part of the message if the system does
		// not have enough network buffers.  If that occurs, we wait
		// and retry until reaching a timeout.
		int bytes_sent = qapi_Net_Websocket_Send(
				p_demo_client_ctxt->websocket_handle, type,
				&data[total_bytes_sent], data_length - total_bytes_sent,
				end_of_message);
		if ((bytes_sent < 0)
				&& (bytes_sent != QAPI_NET_WEBSOCKET_STATUS_NO_BUFFERS)) {
			return bytes_sent;
		}

		// QAPI_NET_WEBSOCKET_STATUS_NO_BUFFERS indicates that there were
		// no network buffers available and we were not able to send any
		// bytes.
		if (bytes_sent != QAPI_NET_WEBSOCKET_STATUS_NO_BUFFERS) {
			// Print the bytes sent if dump is enabled
			if (p_demo_client_ctxt->tx_dump_enabled) {

				WEBSOCKET_PRINTF(
						"\nDUMP TX (bytes sent: %d, offset: %d, bytes requested: %d, end of message: %d)\n",
						bytes_sent, total_bytes_sent, data_length,
						end_of_message);

				if (bytes_sent > 0) {
					int bytes_to_print = MIN(g_max_size_to_print, bytes_sent);
					if (bytes_sent > g_max_size_to_print) {
						WEBSOCKET_PRINTF("printing first %d bytes\n",
								bytes_to_print);
					}
					bench_print_buffer((const char*) (&data[total_bytes_sent]),
							bytes_sent, NULL,
							DUMP_DIRECTION_TX);
				}
			}

			total_bytes_sent += bytes_sent;

			if (total_bytes_sent == data_length) {
				return total_bytes_sent;
			}
		}

		time_spent += g_websocketc_demo_tx_retry_delay_us;
		qapi_Task_Delay(g_websocketc_demo_tx_retry_delay_us);

	} while (time_spent < g_websocketc_demo_tx_timeout_us);

	return total_bytes_sent;
}

void websocketc_demo_event_handler(qapi_Net_Websocket_Hdl_t websocket_handle,
		void * arg, QAPI_Net_Websocket_Event_t event,
		qapi_Net_Websocket_Event_Info_t *event_info) {
	websocketc_demo_ctxt_t *p_demo_client_ctxt = (websocketc_demo_ctxt_t*) arg;

	switch (event) {
	case QAPI_NET_WEBSOCKET_EVENT_CONNECT_E:
		WEBSOCKET_PRINTF("websocket client %d connected\n",
				p_demo_client_ctxt->client_number);
		if (event_info->subprotocol) {
			WEBSOCKET_PRINTF("\tselected subprotocol = %s\n",
					event_info->subprotocol);
		} else {
			if (event_info->subprotocol) {
				WEBSOCKET_PRINTF("\tno subprotocols configured\n");
			}
		}
		break;
	case QAPI_NET_WEBSOCKET_EVENT_MESSAGE_E: {
		if (p_demo_client_ctxt->echo_mode) {
			int bytes_sent = websocketc_demo_send_retry(p_demo_client_ctxt,
					event_info->data_Type, event_info->data,
					event_info->data_Length, event_info->end_Of_Message);
			if (bytes_sent < 0) {
				WEBSOCKET_PRINTF("websocket echo failed with status: %d\n",
						bytes_sent);
				return;
			} else if (bytes_sent != event_info->data_Length) {
				WEBSOCKET_PRINTF(
						"websocket echo timeout, unable to send message due to lack of network buffers.  Closing websocket.\n");
				qapi_Net_Websocket_Close(websocket_handle,
						QAPI_NET_WEBSOCKET_CLOSE_STATUS_GOING_AWAY, NULL);
				return;
			}
		}

		if (p_demo_client_ctxt->rx_dump_enabled) {
			WEBSOCKET_PRINTF(
					"\nDUMP RX (bytes received: %d, end of message: %d)\n",
					event_info->data_Length, event_info->end_Of_Message);
			if (event_info->data_Length > 0) {
				int bytes_to_print = MIN(g_max_size_to_print,
						event_info->data_Length);
				if (event_info->data_Length > g_max_size_to_print) {
					WEBSOCKET_PRINTF("printing first %d bytes\n",
							bytes_to_print);
				}
				bench_print_buffer(event_info->data, bytes_to_print, NULL,
				DUMP_DIRECTION_RX);
			}
		}
	}
		break;
	case QAPI_NET_WEBSOCKET_EVENT_PONG_E:
		WEBSOCKET_PRINTF("websocket pong received with %d byte payload\n",
				event_info->data_Length);
		break;
	case QAPI_NET_WEBSOCKET_EVENT_CLOSE_E:
		if (websocketc_demo_print_statistics(p_demo_client_ctxt) != QAPI_OK) {
			WEBSOCKET_PRINTF("error printing stats\n");
		}
		WEBSOCKET_PRINTF("websocket client %d closed with close status %d\n",
				p_demo_client_ctxt->client_number,
				event_info->close_Status_Code);
	};
}

int websocketc_demo_client_new(uint32_t Parameter_Count,
		QCLI_Parameter_t *Parameter_List) {

	websocketc_demo_ctxt_t* p_demo_ctxt = NULL;
	uint32_t num_subprotocols = 0;
	qbool_t ssl_enabled = FALSE;
	qapi_Net_Websocket_Client_Config_t config = {0};
	char* pp_subprotocol_list[WEBSOCKET_DEMO_MAX_SUBPROTOCOLS];
	char *p_cert_name = NULL;
	char *p_ca_list_name = NULL;

	// Find a free demo client context
	int client_id;
	for (client_id = 0; client_id < WEBSOCKETC_DEMO_MAX_CLIENTS; client_id++) {
		if (g_websocket_client_demo[client_id].websocket_handle == 0) {
			p_demo_ctxt = &g_websocket_client_demo[client_id];
			break;
		}
	}

	if (!p_demo_ctxt) {
		WEBSOCKET_PRINTF(
				"websocket error exceeded maximum supported demo clients.\n");
		return QAPI_ERROR;
	}

	memset(p_demo_ctxt, 0, sizeof(struct websocket_client_demo_s));
	p_demo_ctxt->client_number = client_id + 1;

	/********************************************************
	 *               [0]   [1]
	 * websocket new -o  <origin>
	 *               -k  <subprotocol> (can appear more than once)
	 *               -r  <max_rx_chunk_size>
	 *               -t  <handshake_timeout>
	 *               -z  <closing_timeout>
	 *               -s  <enable ssl = 1 or 0>
	 *               -l  <ca_list_file>
	 *               -c  <certificate_name>
	 ********************************************************/
	int i = 0;
	for (i = 0; (Parameter_Count > 0) && (i < Parameter_Count-1); i++) {
		if (Parameter_List[i].String_Value[0] == '-') {
			switch (Parameter_List[i].String_Value[1]) {
			case 'o':
				i++;
				config.origin =
					Parameter_List[i].String_Value;
				break;

			case 'k':
				i++;
				if (num_subprotocols >= WEBSOCKET_DEMO_MAX_SUBPROTOCOLS) {
					WEBSOCKET_PRINTF(
							"websocket error exceeded maximum %d allowed sub-protocols.\n",
							WEBSOCKET_DEMO_MAX_SUBPROTOCOLS);
					return QCLI_STATUS_ERROR_E;
				}
				pp_subprotocol_list[num_subprotocols] = Parameter_List[i].String_Value;
				num_subprotocols++;
				break;

			case 'r':
				i++;
				config.max_Recv_Chunk_Size =
						Parameter_List[i].Integer_Value;
				break;

			case 't':
				i++;
				config.handshake_Timeout_Millseconds =
					Parameter_List[i].Integer_Value;
				break;

			case 's':
				i++;
				if(Parameter_List[i].Integer_Value > 0) {
					ssl_enabled = TRUE;
				}
				break;

			case 'l':
				i++;
				p_ca_list_name = Parameter_List[i].String_Value;
				break;

			case 'c':
				i++;
				p_cert_name = Parameter_List[i].String_Value;
				break;

			case 'z':
				i++;
				config.closing_Timeout_Millseconds =
					Parameter_List[i].Integer_Value;
				break;

			default:
				WEBSOCKET_PRINTF("Unknown option: %s\n",
						Parameter_List[i].String_Value);
				return QCLI_STATUS_ERROR_E;
			}
		} else {
			WEBSOCKET_PRINTF("Unknown option: %s\n",
					Parameter_List[i].String_Value);
			return QCLI_STATUS_ERROR_E;
		}

		if (i == Parameter_Count) {
			WEBSOCKET_PRINTF("What is value of %s?\n",
					Parameter_List[i - 1].String_Value);
			return QCLI_STATUS_ERROR_E;
		}
	} /* for */

	if( !ssl_enabled && ((p_cert_name != NULL) || (p_ca_list_name != NULL)) ) {
		WEBSOCKET_PRINTF("error must enable ssl to configure CA list or certificate.\n");
		return QCLI_STATUS_ERROR_E;
	}

	// If SSL enabled set allocate SSL context and set the
	// CA list and/or certificate if specified
	if(ssl_enabled)
	{
		/* Create SSL Object. */
		if (p_demo_ctxt->ssl_obj_handle == 0) {
			p_demo_ctxt->ssl_obj_handle = qapi_Net_SSL_Obj_New(
					QAPI_NET_SSL_CLIENT_E);
			if (p_demo_ctxt->ssl_obj_handle == 0) {
				WEBSOCKET_PRINTF("websocket sslconfig allocation failure.\n");
				goto error;
			}
		}

		/* Configure the certificate */
		if(p_cert_name) {
			if (qapi_Net_SSL_Cert_Load(p_demo_ctxt->ssl_obj_handle, QAPI_NET_SSL_CERTIFICATE_E,
					p_cert_name) < 0) {
				WEBSOCKET_PRINTF(
						"ERROR: Unable to load %s from secure storage repository\n",
						p_cert_name);
				goto error;
			}
			WEBSOCKET_PRINTF("%s loaded from secure storage repository\n", p_cert_name);
		}

		/* Configure the CA list */
		if(p_ca_list_name) {
			if (qapi_Net_SSL_Cert_Load(p_demo_ctxt->ssl_obj_handle, QAPI_NET_SSL_CA_LIST_E,
					p_ca_list_name) < 0) {
				WEBSOCKET_PRINTF(
						"ERROR: Unable to load %s from secure storage repository\n",
						p_ca_list_name);
				goto error;
			}
			WEBSOCKET_PRINTF("%s loaded from secure storage repository\n", p_ca_list_name);
		}
	}

	config.subprotocol_List = (const char**)pp_subprotocol_list;
	config.subprotocol_List_Length = num_subprotocols;
	config.ssl_Object_Handle = p_demo_ctxt->ssl_obj_handle;

	/* Create a new websocket context */
	p_demo_ctxt->websocket_handle = qapi_Net_Websocket_Client_New(
			&config);
	if (p_demo_ctxt->websocket_handle == 0) {
		WEBSOCKET_PRINTF("Error allocating websocket context.\r\n");
		goto error;
	}

	qapi_Net_Websocket_Register_Event_Callback(
			p_demo_ctxt->websocket_handle, websocketc_demo_event_handler,
			p_demo_ctxt);

	WEBSOCKET_PRINTF("Websocket created. Id %d\n",
			p_demo_ctxt->client_number);

	return QCLI_STATUS_SUCCESS_E;

error:

	if(p_demo_ctxt->ssl_obj_handle != 0) {
		qapi_Net_SSL_Obj_Free(p_demo_ctxt->ssl_obj_handle);
		p_demo_ctxt->ssl_obj_handle = 0;
	}
	return QCLI_STATUS_ERROR_E;
}

#define WEBSOCKET_DEMO_CONNECT_NUM_ARGS (6)
int websocketc_demo_client_connect(uint32_t Parameter_Count,
		QCLI_Parameter_t *Parameter_List, websocketc_demo_ctxt_t *p_demo_ctxt) {
	const char *p_server = NULL;
	uint16_t port = 0;
	const char *p_resource_path = NULL;

	if(Parameter_Count != WEBSOCKET_DEMO_CONNECT_NUM_ARGS) {
		WEBSOCKET_PRINTF("Invalid number of parameters for connect\n");
		return QCLI_STATUS_ERROR_E;
	}

	/********************************************************
 	 *                              [0]   [1]  [2]  [3]   [4]    [5]
	 * websocket connect client_id  -h  <host> -p  <port> -r  <resource_path>
	 ********************************************************/
	int i = 0;
	for (i = 0; i < Parameter_Count-1; i++) {
		if (Parameter_List[i].String_Value[0] == '-') {
			switch (Parameter_List[i].String_Value[1]) {
			case 'h':
				i++;
				p_server = Parameter_List[i].String_Value;
				break;

			case 'r':
				i++;
				p_resource_path = Parameter_List[i].String_Value;
				break;

			case 'p':
				i++;
				port = Parameter_List[i].Integer_Value;
				break;

			default:
				WEBSOCKET_PRINTF("Unknown option: %s\n",
						Parameter_List[i].String_Value);
				return QCLI_STATUS_ERROR_E;
			}
		} else {
			WEBSOCKET_PRINTF("Unknown option: %s\n",
					Parameter_List[i].String_Value);
			return QCLI_STATUS_ERROR_E;
		}

		if (i == Parameter_Count) {
			WEBSOCKET_PRINTF("What is value of %s?\n",
					Parameter_List[i - 1].String_Value);
			return QCLI_STATUS_ERROR_E;
		}
	} /* for */

	// Connect to the server
	int status = qapi_Net_Websocket_Client_Connect(
			p_demo_ctxt->websocket_handle, p_server, port, p_resource_path);
	if (status != QAPI_OK) {
		WEBSOCKET_PRINTF("websocket client connect failed with status %d\n",
				status);
		return QCLI_STATUS_ERROR_E;
	}

	return QAPI_OK;
}

int websocketc_demo_client_destroy(uint32_t Parameter_Count,
		QCLI_Parameter_t *Parameter_List, websocketc_demo_ctxt_t *p_demo_ctxt) {

	if(p_demo_ctxt->websocket_handle != 0) {
		int status = qapi_Net_Websocket_Client_Free(p_demo_ctxt->websocket_handle);
		if (status != QAPI_OK) {
			WEBSOCKET_PRINTF("websocket client connect failed with status %d\n",
					status);
		}
	}

	if(p_demo_ctxt->ssl_obj_handle != 0) {
		qapi_Net_SSL_Obj_Free(p_demo_ctxt->ssl_obj_handle);
	}

	memset(p_demo_ctxt, 0, sizeof(websocketc_demo_ctxt_t));

	WEBSOCKET_PRINTF("websocket client context destroyed.\n");

	return QCLI_STATUS_SUCCESS_E;
}

int websocketc_demo_client_close(uint32_t Parameter_Count,
		QCLI_Parameter_t *Parameter_List, websocketc_demo_ctxt_t *p_demo_ctxt) {
	qapi_Net_Websocket_Close(p_demo_ctxt->websocket_handle,
			QAPI_NET_WEBSOCKET_CLOSE_STATUS_NORMAL, NULL);
	WEBSOCKET_PRINTF("websocket client successfully closed connection\n");
	return QCLI_STATUS_SUCCESS_E;
}

int websocketc_demo_echo(uint32_t cmd_arg_count, QCLI_Parameter_t *p_cmd_args,
		websocketc_demo_ctxt_t *p_demo_ctxt) {
	if (cmd_arg_count == 0) {
		websocketc_demo_help();
		return QCLI_STATUS_ERROR_E;
	}
	if (strncmp(p_cmd_args[0].String_Value, "on", 2) == 0) {
		p_demo_ctxt->echo_mode = 1;
	} else if (strncmp(p_cmd_args[0].String_Value, "off", 3) == 0) {
		p_demo_ctxt->echo_mode = 0;
	} else {
		websocketc_demo_help();
		return QCLI_STATUS_ERROR_E;
	}

	return QCLI_STATUS_SUCCESS_E;
}

int websocketc_demo_dump(uint32_t cmd_arg_count, QCLI_Parameter_t *p_cmd_args,
		websocketc_demo_ctxt_t *p_demo_ctxt) {

	if (cmd_arg_count != 2) {
		websocketc_demo_help();
		return QCLI_STATUS_ERROR_E;
	}

	/********************************************************
	 *                  [0]     [1]
	 * websocket dump <tx/rx>  <on/off>
	 ********************************************************/

	if ((strncmp(p_cmd_args[0].String_Value, "tx", 2) == 0)
			&& (strncmp(p_cmd_args[1].String_Value, "on", 2) == 0)) {
		p_demo_ctxt->tx_dump_enabled = TRUE;
	} else if ((strncmp(p_cmd_args[0].String_Value, "tx", 2) == 0)
			&& (strncmp(p_cmd_args[1].String_Value, "off", 3) == 0)) {
		p_demo_ctxt->tx_dump_enabled = FALSE;
	} else if ((strncmp(p_cmd_args[0].String_Value, "rx", 2) == 0)
			&& (strncmp(p_cmd_args[1].String_Value, "on", 2) == 0)) {
		p_demo_ctxt->rx_dump_enabled = TRUE;
	} else if ((strncmp(p_cmd_args[0].String_Value, "rx", 2) == 0)
			&& (strncmp(p_cmd_args[1].String_Value, "off", 3) == 0)) {
		p_demo_ctxt->rx_dump_enabled = FALSE;
	} else {
		websocketc_demo_help();
		return QCLI_STATUS_ERROR_E;
	}

	return QCLI_STATUS_SUCCESS_E;
}

#define WEBSOCKET_DEMO_SEND_MIN_NUM_ARGS (4)
int websocketc_demo_send(uint32_t cmd_arg_count, QCLI_Parameter_t *p_cmd_args,
		websocketc_demo_ctxt_t *p_demo_ctxt) {
	if (cmd_arg_count != WEBSOCKET_DEMO_SEND_MIN_NUM_ARGS) {
		websocketc_demo_help();
		return QCLI_STATUS_ERROR_E;
	}

	/********************************************************
	 *                   [0]        [1]      [2]      [3]      [4]
	 * websocket send client_id     -m     message   -e   <end_of_message>
	 ********************************************************/
	char *p_message = NULL;
	uint32_t message_length = 0;
	uint8_t end_of_message = 0;

	int i = 0;
	for (i = 0; i < cmd_arg_count-1; i++) {
		if (p_cmd_args[i].String_Value[0] == '-') {
			switch (p_cmd_args[i].String_Value[1]) {
			case 'e':
				i++;
				end_of_message = p_cmd_args[i].Integer_Value;
				break;
			case 'm':
				i++;
				p_message = p_cmd_args[i].String_Value;
				message_length = strlen(p_message);
				break;
			default:
				WEBSOCKET_PRINTF("Unknown option: %s\n",
						p_cmd_args[i].String_Value);
				return QCLI_STATUS_ERROR_E;
			}
		} else {
			WEBSOCKET_PRINTF("Unknown option: %s\n",
					p_cmd_args[i].String_Value);
			return QCLI_STATUS_ERROR_E;
		}

		if (i == cmd_arg_count) {
			WEBSOCKET_PRINTF("What is value of %s?\n",
					p_cmd_args[i - 1].String_Value);
			return QCLI_STATUS_ERROR_E;
		}
	} /* for */

	int bytes_sent = websocketc_demo_send_retry(p_demo_ctxt,
			QAPI_NET_WEBSOCKET_DATA_TYPE_TEXT_E, (uint8_t*) p_message,
			message_length, end_of_message);
	if (bytes_sent < 0) {
		WEBSOCKET_PRINTF("websocket send failed with status: %d\n", bytes_sent);
		return QCLI_STATUS_ERROR_E;
	} else if (bytes_sent != message_length) {
		WEBSOCKET_PRINTF(
				"websocket send timeout, unable to send message due to lack of network buffers.  Closing websocket.\n");
		qapi_Net_Websocket_Close(p_demo_ctxt->websocket_handle,
				QAPI_NET_WEBSOCKET_CLOSE_STATUS_GOING_AWAY, NULL);
		return QCLI_STATUS_ERROR_E;
	} else {
		WEBSOCKET_PRINTF("websocket sent message.\n");
	}

	return QCLI_STATUS_SUCCESS_E;
}

int websocketc_demo_ping(uint32_t cmd_arg_count, QCLI_Parameter_t *p_cmd_args,
		websocketc_demo_ctxt_t *p_demo_ctxt) {
	char *p_ping_data = NULL;
	uint32_t ping_data_length = 0;

	int i;
	for (i = 0; (cmd_arg_count > 0) && (i < cmd_arg_count-1); i++) {
		if (p_cmd_args[i].String_Value[0] == '-') {
			switch (p_cmd_args[i].String_Value[1]) {
			case 'd':
				i++;
				p_ping_data = p_cmd_args[i].String_Value;
				ping_data_length = strlen(p_ping_data);
				break;

			default:
				WEBSOCKET_PRINTF("Unknown option: %s\n",
						p_cmd_args[i].String_Value);
				return QCLI_STATUS_ERROR_E;
			}
		} else {
			WEBSOCKET_PRINTF("Unknown option: %s\n",
					p_cmd_args[i].String_Value);
			return QCLI_STATUS_ERROR_E;
		}

		if (i == cmd_arg_count) {
			WEBSOCKET_PRINTF("What is value of %s?\n",
					p_cmd_args[i - 1].String_Value);
			return QCLI_STATUS_ERROR_E;
		}
	} /* for */

	qapi_Status_t status = qapi_Net_Websocket_Ping(
			p_demo_ctxt->websocket_handle, (uint8_t*) p_ping_data,
			ping_data_length);
	if (status != QAPI_OK) {
		WEBSOCKET_PRINTF("websocket ping failed with status code: %d\n",
				status);
		return QCLI_STATUS_ERROR_E;
	}
	WEBSOCKET_PRINTF("websocket ping sent.\n");

	return QCLI_STATUS_SUCCESS_E;
}

int websocketc_demo_pong(uint32_t cmd_arg_count, QCLI_Parameter_t *p_cmd_args,
		websocketc_demo_ctxt_t *p_demo_ctxt) {
	char *p_pong_data = NULL;
	uint32_t pong_data_length = 0;

	int i;

	for (i = 0; (cmd_arg_count > 0) && (i < cmd_arg_count-1); i++) {
		if (p_cmd_args[i].String_Value[0] == '-') {
			switch (p_cmd_args[i].String_Value[1]) {
			case 'd':
				i++;
				if (i == cmd_arg_count) {
					WEBSOCKET_PRINTF("What is value of %s?\n",
							p_cmd_args[i - 1].String_Value);
					return QCLI_STATUS_ERROR_E;
				}

				p_pong_data = p_cmd_args[i].String_Value;
				pong_data_length = strlen(p_pong_data);
				break;

			default:
				WEBSOCKET_PRINTF("Unknown option: %s\n",
						p_cmd_args[i].String_Value);
				return QCLI_STATUS_ERROR_E;
			}
		} else {
			WEBSOCKET_PRINTF("Unknown option: %s\n",
					p_cmd_args[i].String_Value);
			return QCLI_STATUS_ERROR_E;
		}
	} /* for */

	qapi_Status_t status = qapi_Net_Websocket_Pong(
			p_demo_ctxt->websocket_handle, (uint8_t*) p_pong_data,
			pong_data_length);
	if (status != QAPI_OK) {
		WEBSOCKET_PRINTF("websocket pong failed with status code: %d\n",
				status);
		return QCLI_STATUS_ERROR_E;
	}
	WEBSOCKET_PRINTF("websocket pong sent.\n");

	return QCLI_STATUS_SUCCESS_E;
}

int websocketc_demo_sslconfig(uint32_t cmd_arg_count,
		QCLI_Parameter_t *p_cmd_args, websocketc_demo_ctxt_t *p_demo_ctxt) {

	qapi_Net_SSL_Config_t *p_ssl_config = NULL;
	int ret = QCLI_STATUS_SUCCESS_E;

	/* Create SSL config to store the SSL connection parameters */
	p_ssl_config = (qapi_Net_SSL_Config_t*) malloc(
			sizeof(qapi_Net_SSL_Config_t));
	if (p_ssl_config == NULL) {
		WEBSOCKET_PRINTF("websocket sslconfig allocation failure.\n");
		return QCLI_STATUS_ERROR_E;
	}
	memset(p_ssl_config, 0, sizeof(qapi_Net_SSL_Config_t));

	int ssl_parse_status = ssl_parse_config_parameters(cmd_arg_count,
			p_cmd_args, p_ssl_config, 0, false);
	if (ssl_parse_status == QCLI_STATUS_ERROR_E) {
		WEBSOCKET_PRINTF("websocket sslconfig failed to parse arguments\n");
		ret = QCLI_STATUS_ERROR_E;
		goto cleanup;
	}

	/* Websocket always uses TLS. If configured for DTLS, set it to TLS */
	if ((p_ssl_config->protocol == QAPI_NET_SSL_PROTOCOL_DTLS_1_0)
			|| (p_ssl_config->protocol
					== QAPI_NET_SSL_PROTOCOL_DTLS_1_2)
			|| (p_ssl_config->protocol == QAPI_NET_SSL_DTLS_E)) {
		WEBSOCKET_PRINTF(
				"websocket sslconfig warning, DTLS is not supported changing to TLS\n");
		p_ssl_config->protocol = QAPI_NET_SSL_PROTOCOL_TLS_1_2;
	}

	/* QAPI_NET_SSL_TLS_E is used by the SSL demo. Select TLS1.2 if not
	 * explicitly specified by the user.
	 */
	if (p_ssl_config->protocol == QAPI_NET_SSL_TLS_E) {
		p_ssl_config->protocol = QAPI_NET_SSL_PROTOCOL_TLS_1_2;
	}

	/* Configure websocket SSL parameters */
	int ssl_config_status = qapi_Net_Websocket_Configure_SSL(
			p_demo_ctxt->websocket_handle, p_ssl_config);
	if (ssl_config_status != QAPI_OK) {
		WEBSOCKET_PRINTF(
				"websocket configure SSL failed with status code: %d\n",
				ssl_config_status);
		ret = QCLI_STATUS_ERROR_E;
		goto cleanup;
	}

	WEBSOCKET_PRINTF("websocket sslconfig successful\n");
	ret = QCLI_STATUS_SUCCESS_E;

cleanup:

	if(p_ssl_config) {
		ssl_free_config_parameters(p_ssl_config);
		free(p_ssl_config);
	}

	return ret;
}

int websocketc_demo_addhttpheader(uint32_t cmd_arg_count,
		QCLI_Parameter_t *p_cmd_args, websocketc_demo_ctxt_t *p_demo_ctxt) {
	if (cmd_arg_count != 2) {
		websocketc_demo_help();
		return QCLI_STATUS_ERROR_E;
	}
	if (qapi_Net_Websocket_Client_Add_Handshake_HTTP_Header_Field(
			p_demo_ctxt->websocket_handle, p_cmd_args[0].String_Value,
			p_cmd_args[1].String_Value) != QAPI_OK) {
		WEBSOCKET_PRINTF("websocket error adding handshake header.\n");
		return QCLI_STATUS_ERROR_E;
	}
	return QCLI_STATUS_SUCCESS_E;
}

int websocketc_demo_resethttpheaders(uint32_t cmd_arg_count,
		QCLI_Parameter_t *p_cmd_args, websocketc_demo_ctxt_t *p_demo_ctxt) {
	if (qapi_Net_Websocket_Client_Clear_Handshake_HTTP_Headers(
			p_demo_ctxt->websocket_handle) != QAPI_OK) {
		WEBSOCKET_PRINTF("websocket error resetting handshake header.\n");
		return QCLI_STATUS_ERROR_E;
	}
	return QCLI_STATUS_SUCCESS_E;
}

QCLI_Command_Status_t websocketc_command_handler(uint32_t Parameter_Count,
		QCLI_Parameter_t *Parameter_List) {
	char *cmd = NULL;

	if (Parameter_Count == 0 || Parameter_List == NULL) {
		websocketc_demo_help();
		return QCLI_STATUS_ERROR_E;
	}

	cmd = Parameter_List[0].String_Value;

	if (strncmp(cmd, "new", 3) == 0) {
		return websocketc_demo_client_new(Parameter_Count - 1,
				&Parameter_List[1]);
	}

	// The remaining commands require a client id
	if (Parameter_Count <= 1) {
		websocketc_demo_help();
		return QCLI_STATUS_ERROR_E;
	}

	uint32_t client_id = Parameter_List[1].Integer_Value;

	if (client_id == 0 || client_id > WEBSOCKETC_DEMO_MAX_CLIENTS) {
		WEBSOCKET_PRINTF("Invalid client id\n");
		return QCLI_STATUS_ERROR_E;
	}

	uint32_t cmd_arg_count = Parameter_Count - 2;
	QCLI_Parameter_t *p_cmd_args = NULL;
	if (cmd_arg_count > 0) {
		p_cmd_args = &Parameter_List[2];
	}

	// Note that client ID is in the range of [1..WEBSOCKETC_DEMO_MAX_CLIENTS]
	websocketc_demo_ctxt_t *p_demo_ctxt = &g_websocket_client_demo[client_id - 1];

	if(p_demo_ctxt->websocket_handle == 0) {
		WEBSOCKET_PRINTF("Error must call new command before calling %s\n", cmd);
		return QCLI_STATUS_ERROR_E;
	}

	if (strncmp(cmd, "destroy", 7) == 0) {
		return websocketc_demo_client_destroy(cmd_arg_count, p_cmd_args,
				p_demo_ctxt);
	} else if (strncmp(cmd, "connect", 7) == 0) {
		return websocketc_demo_client_connect(cmd_arg_count, p_cmd_args,
				p_demo_ctxt);
	} else if (strncmp(cmd, "close", 5) == 0) {
		return websocketc_demo_client_close(cmd_arg_count, p_cmd_args,
				p_demo_ctxt);
	} else if (strncmp(cmd, "send", 4) == 0) {
		return websocketc_demo_send(cmd_arg_count, p_cmd_args, p_demo_ctxt);
	} else if (strncmp(cmd, "echo", 4) == 0) {
		return websocketc_demo_echo(cmd_arg_count, p_cmd_args, p_demo_ctxt);
	} else if (strncmp(cmd, "dump", 4) == 0) {
		return websocketc_demo_dump(cmd_arg_count, p_cmd_args, p_demo_ctxt);
	} else if (strncmp(cmd, "ping", 4) == 0) {
		return websocketc_demo_ping(cmd_arg_count, p_cmd_args, p_demo_ctxt);
	} else if (strncmp(cmd, "pong", 4) == 0) {
		return websocketc_demo_pong(cmd_arg_count, p_cmd_args, p_demo_ctxt);
	} else if (strncmp(cmd, "sslconfig", 9) == 0) {
		return websocketc_demo_sslconfig(cmd_arg_count, p_cmd_args, p_demo_ctxt);
	} else if (strncmp(cmd, "addhttpheader", 20) == 0) {
		return websocketc_demo_addhttpheader(cmd_arg_count, p_cmd_args,
				p_demo_ctxt);
	} else if (strncmp(cmd, "resethttpheaders", 23) == 0) {
		return websocketc_demo_resethttpheaders(cmd_arg_count, p_cmd_args,
				p_demo_ctxt);
	} else {
		WEBSOCKET_PRINTF("Unknown command.\n");
		return QCLI_STATUS_ERROR_E;
	}

	return QCLI_STATUS_SUCCESS_E;
}
#endif
