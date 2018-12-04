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


#ifndef _BENCH_H_
#define _BENCH_H_

#include "qcli_api.h"
#include "qapi_ssl.h"
#include "qapi_socket.h"
#include "qapi_ns_utils.h"
#include "qapi_netbuf.h"
#include "net_utils.h"       /* time_struct_t */

#undef A_OK
#define A_OK                    QAPI_OK

#undef A_ERROR
#define A_ERROR                 -1

#undef HOST_TO_LE_LONG
#define HOST_TO_LE_LONG(n)      (n)

#define BENCH_TEST_COMPLETED    "**** IOT Throughput Test Completed ****\r\n"
#define CFG_PACKET_SIZE_MAX_TX  (1576)
#define CFG_PACKET_SIZE_MAX_RX  (1556)
#define END_OF_TEST_CODE        (0xAABBCCDD)

#define SSL_SERVER_INST         0
#define SSL_CLIENT_INST         1
#define MAX_SSL_INST            (SSL_CLIENT_INST + 1)

#define SSL_ACCEPT_TIMEOUT_IN_MS 60000
#define SSL_ACCEPT_SLEEP_IN_MS 100

#define CLIENT_WAIT_TIME        30000

#define OFFSETOF(type, field)   ((size_t)(&((type*)0)->field))

typedef qapi_Net_Buf_t * PACKET;

typedef struct udp_zc_rx_info
{
    struct udp_zc_rx_info *next;
    PACKET pkt;
    struct sockaddr from;
} UDP_ZC_RX_INFO;

/*structure to manage stats received from peer during UDP traffic test*/
typedef struct stat_packet
{
    uint32_t bytes;
    uint32_t kbytes;
    uint32_t msec;
    uint32_t numPackets;
} stat_packet_t;

typedef enum {
	START,
	STOP,
} bench_common_display_cmd_t;

typedef struct {
	void *addr;
	uint16_t port;
	uint8_t test_type;
} bench_common_start_cmd_t;

typedef struct ip_params {
	uint32_t ipv4Addr;
	uint32_t local_ipv4Addr;
	uint8_t ipv6Addr[16];
	uint8_t local_ipv6Addr[16];
	uint8_t ip_tos;
	uint32_t source_ipv4_addr;	/* To fill the 'source address' field in IPv4 header (in net order)
					 * This is for IP_RAW_TX_HDR.
					 */
	int32_t scope_id;
} IP_PARAMS;

typedef struct multicast_params {
    uint32_t ipv4Addr;
    uint32_t rcvIf;
    uint8_t ipv6Addr[16];
    uint8_t enabled;
} MULTICAST_PARAMS;

/**************************************************************************/ /*!
 * TX/RX Test parameters
 ******************************************************************************/
typedef struct transmit_params
{
    uint32_t ip_address;    /* peer's IPv4 address */
    uint8_t v6addr[16];     /* peer's IPv6 address */
    int32_t scope_id;
    uint32_t source_ipv4_addr;  /* To fill the 'source address' field in IPv4 header (in net order)
                                 * This is for IP_RAW_TX_HDR.
                                 */
    int packet_size;
    int tx_time;
    int packet_number;
    uint32_t interval_us;
    uint16_t port;          /* port for UDP/TCP, protocol for IP_RAW */
    uint8_t zerocopy_send;  /* 1 = this is zero-copy TX */
    uint8_t test_mode;      /* TIME_TEST or PACKET_TEST */
    uint8_t v6;             /* 1 = this is to TX IPv6 packets */
    uint8_t ip_tos;         /* TOS value in IPv4 header */
} TX_PARAMS;

typedef struct receive_params
{
    uint16_t port;
    uint16_t local_if;
    uint32_t local_address;
    uint8_t local_v6addr[16];
    uint32_t mcIpaddr;
    uint32_t mcRcvIf;
    uint8_t mcIpv6addr[16];
    int32_t scope_id;
    uint8_t mcEnabled;
    uint8_t v6;
} RX_PARAMS;

typedef struct stats {
    time_struct_t first_time;       /* Test start time */
    time_struct_t last_time;
    unsigned long long bytes;       /* Number of bytes received in current test */
    unsigned long long kbytes;      /* Number of kilo bytes received in current test */
    unsigned long long last_bytes;  /* Number of bytes received in the previous test */
    unsigned long long last_kbytes;
    unsigned long long sent_bytes;
    uint32_t    pkts_recvd;
    //uint32_t    pkts_expctd;
    uint32_t    last_interval;
    uint32_t    last_throughput;
    /* iperf stats */
    uint32_t    iperf_display_interval;
    uint32_t    iperf_time_sec;
    uint32_t    iperf_stream_id;
    uint32_t    iperf_udp_rate;
} STATS;

/************************************************************************
*    Benchmark server control structure.
*************************************************************************/
typedef struct throughput_cxt
{
	uint32_t protocol;				/* 1:TCP 2:UDP 4:SSL*/
	uint32_t zc;					/* zero-copy */
	uint16_t port;
    int32_t sock_local;             /* Listening socket.*/
    int32_t sock_peer;              /* Foreign socket.*/
    int32_t rxcode;                 /* event code from rx_upcall */
    char* buffer;
    STATS pktStats;
    union params_u
    {
        TX_PARAMS    tx_params;
        RX_PARAMS    rx_params;
    } params;
    uint8_t test_type;
    uint32_t iperf_stream_id;
    uint8_t is_iperf:1;
    uint8_t print_buf:1;
    uint8_t echo:1;
} THROUGHPUT_CXT;

typedef struct {
	int32_t sockfd;           /* Listening Socket */
	uint16_t port;
	IP_PARAMS ip_params;
	int busySlot;
	int exit;
} bench_tcp_server_t;

typedef struct {
	qapi_Net_SSL_Con_Hdl_t connHandle;
	int ssl_state;
	time_struct_t hs_start_time;
} bench_ssl_server_inst_t;

typedef struct {
	THROUGHPUT_CXT *ctxt;
	uint16_t port;
	int32_t rxcode;                 /* event code from rx_upcall */
	int busySlot;
	int exit;
	int ready;
	uint32_t netbuf_id;
	uint32_t buffer_offset;
	uint32_t cur_packet_number;
	int send_flag;
	int isFirst;
	int sock_peer;
	STATS pktStats;
	char *buffer;
	bench_ssl_server_inst_t sslInst;
    uint32_t iperf_display_last;
    uint32_t iperf_display_next;
} bench_tcp_session_t;

typedef struct end_of_test {
    int code;
    int packet_count;
} EOT_PACKET;

enum test_type {
	TX,
	RX,
};

enum protocol {
    UDP,   //UDP Transmit (Uplink Test)
    TCP,   //TCP Receive (Downlink Test)
    SSL,   //SSL Transmit (Uplink Test)
    IP_RAW, //SOCKET_RAW TX
    IP_RAW_HDR, //SOCKET_RAW TX with IP_HDRINCL enabled
};

enum Test_Mode
{
	TIME_TEST,
	PACKET_TEST
};

/************************************************************************
*    IPv4 header
*************************************************************************/
typedef struct ipv4_header_s {
    uint8_t      ver_ihl;
    uint8_t      tos;
    uint16_t     len;       /* length of this IP datagram */
    uint16_t     id;
    uint16_t     flags_offset;
    uint8_t      ttl;
    uint8_t      protocol;
    uint16_t     hdr_chksum;
    uint32_t     sourceip;
    uint32_t     destip;
} ipv4_header_t;

QCLI_Command_Status_t bench_common_rx4(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t bench_common_rx6(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t bench_common_tx4(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t bench_common_tx6(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t benchquit(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t uapsdtest(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

#define CERT_HEADER_LEN         sizeof(CERT_HEADER_T)
#define SSL_CERT_BUF_LEN        (1200)
typedef struct
{
    uint8_t id[4];
    uint32_t length;
    uint8_t data[0];
} CERT_HEADER_T;

typedef struct ssl_inst
{
    qapi_Net_SSL_Obj_Hdl_t sslCtx;
    qapi_Net_SSL_Con_Hdl_t ssl;
    qapi_Net_SSL_Config_t   config;
    uint8_t      config_set;
    qapi_Net_SSL_Role_t role;
} SSL_INST;

extern SSL_INST ssl_inst[MAX_SSL_INST];

QCLI_Command_Status_t ssl_command_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t ssl_start(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t ssl_stop(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
void sslconfig_help(const char *str);
void ssl_free_config_parameters(qapi_Net_SSL_Config_t *cfg);
QCLI_Command_Status_t ssl_parse_config_parameters(
        uint32_t Parameter_Count,
        QCLI_Parameter_t *Parameter_List,
        qapi_Net_SSL_Config_t *cfg,
        qapi_Net_SSL_Obj_Hdl_t ssl_hdl,
        qbool_t server);
QCLI_Command_Status_t ssl_config(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t ssl_add_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t ssl_add_psk_table(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

extern uint8_t *cert_data_buf;
extern uint16_t cert_data_buf_len;
QCLI_Command_Status_t cert_command_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t store_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t delete_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t list_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t get_cert(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
SSL_INST * bench_ssl_GetInstance(int instance);
void bench_ssl_ResetInstance(uint32_t instance);
void bench_ssl_InitInstance(uint32_t instance, uint32_t value);
int bench_ssl_IsDTLS(int instance);
int bench_ssl_GetProtocol();
qapi_Net_SSL_Role_t* bench_ssl_GetSSLRole(int instance);
qapi_Net_SSL_Con_Hdl_t bench_ssl_rx_setup(SSL_INST *ssl, int sockfd, qapi_Net_SSL_Con_Hdl_t *connHandle, int so_blocking);
int bench_ssl_CreateConnection(THROUGHPUT_CXT *p_tCxt, SSL_INST* ssl);
int bench_ssl_Con_Get_Status(bench_ssl_server_inst_t *srv);
void  bench_ssl_Print_SSL_Handshake_Status(int status);
QCLI_Command_Status_t httpc_command_handler(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);

//udp test pattern
typedef struct udp_pattern_of_test{
  unsigned int code;
  unsigned short seq;
} UDP_PATTERN_PACKET;

#define CODE_UDP    ('U'|('D'<<8)|'P'<<16)

#define IEEE80211_SEQ_SEQ_MASK          0xfff0
#define IEEE80211_SEQ_SEQ_SHIFT         4

#define IEEE80211_SN_MASK       ((IEEE80211_SEQ_SEQ_MASK) >> IEEE80211_SEQ_SEQ_SHIFT)
#define IEEE80211_MAX_SN        IEEE80211_SN_MASK
#define IEEE80211_SN_MODULO     (IEEE80211_MAX_SN + 1)

#define IEEE80211_SN_LESS(sn1, sn2) \
    ((((sn1) - (sn2)) & IEEE80211_SN_MASK) > (IEEE80211_SN_MODULO >> 1))

#define IEEE80211_SN_ADD(sn1, sn2) \
    (((sn1) + (sn2)) & IEEE80211_SN_MASK)

#define IEEE80211_SN_INC(sn)    \
    IEEE80211_SN_ADD((sn), 1)

#define IEEE80211_SN_SUB(sn1, sn2)  \
    (((sn1) - (sn2)) & IEEE80211_SN_MASK)

typedef struct stat_udp_pattern
{
    uint32_t pkts_plan;
    uint32_t pkts_recvd;
    uint32_t pkts_seq_recvd;
    uint32_t pkts_seq_less;
    unsigned short seq_last;
    unsigned short ratio_of_drop;
    unsigned short ratio_of_seq_less;
    char stat_valid;
}stat_udp_pattern_t;

/* Dump command */
extern uint8_t dump_enabled;
extern uint8_t dump_flags;

#define DUMP_ENABLE_TX		(1<<0)
#define DUMP_ENABLE_RX		(1<<1)

#define DUMP_DIRECTION_TX	(0)
#define DUMP_DIRECTION_RX	(1)


#define DUMP_IS_TX_ENABLED (dump_enabled && (dump_flags & DUMP_ENABLE_TX))
#define DUMP_IS_RX_ENABLED (dump_enabled && (dump_flags & DUMP_ENABLE_RX))

extern int32_t g_cookie_uc;
extern int32_t g_cookie_mc;
void rxreorder_udp_payload_init (stat_udp_pattern_t *stat_udp);
int rxreorder_udp_payload_valid (stat_udp_pattern_t *stat_udp);
void rxreorder_udp_payload_statistics (stat_udp_pattern_t *stat_udp, char* buffer, int32_t len);
void rxreorder_udp_payload_report (stat_udp_pattern_t *stat_udp);
void bench_common_Display(THROUGHPUT_CXT *p_rxtCxt, bench_common_display_cmd_t cmd, void* arg);
int bench_raw_rx(THROUGHPUT_CXT *p_tCxt);
int bench_raw_tx(THROUGHPUT_CXT *p_tCxt);
void send_ack(THROUGHPUT_CXT *p_tCxt, struct sockaddr *faddr, int addrlen);
void send_ack_zc(THROUGHPUT_CXT *p_tCxt, struct sockaddr *faddr, uint32_t addrlen);
unsigned short ratio(uint32_t numerator, uint32_t denominator, unsigned short base);
void bench_common_clear_stats(THROUGHPUT_CXT *p_tCxt);
void bench_tcp_rx(THROUGHPUT_CXT *p_tCxt);
void bench_tcp_rx_zc(THROUGHPUT_CXT *p_tCxt);
void bench_tcp_tx(THROUGHPUT_CXT *p_tCxt);
void bench_tcp_rx_quit(int sessionId);
void bench_udp_rx(THROUGHPUT_CXT *p_tCxt);
void bench_udp_rx_zc(THROUGHPUT_CXT *p_tCxt);
void bench_udp_tx(THROUGHPUT_CXT *p_tCxt);
void bench_common_print_test_results(THROUGHPUT_CXT *p_tCxt, STATS *pktStats);
uint32_t bench_common_check_test_time(THROUGHPUT_CXT *p_tCxt);
int bench_common_wait_for_response(THROUGHPUT_CXT *p_tCxt, struct sockaddr *to, uint32_t tolen, uint32_t cur_packet_number);
QCLI_Command_Status_t bench_uapsd_test(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
void bench_common_add_pattern(char *p, int len);
uint32_t bench_udp_IsPortInUse(uint16_t port);
uint32_t bench_tcp_IsPortInUse(uint16_t port);
char* bench_common_GetInterfaceNameFromStr(char *ipstr);
uint32_t bench_common_SetParams(THROUGHPUT_CXT *p_rxtCxt, uint32_t v6, const char *protocol, uint16_t port, enum test_type type);
void bench_print_buffer(const char *buf, uint32_t len, struct sockaddr *sock_addr, uint8_t direction);
void bench_tcp_rx_dump_servers();
QCLI_Command_Status_t bench_common_set_pattern(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
QCLI_Command_Status_t queuecfg(uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List);
void bench_config_queue_size(int32_t sock);
#endif /* _BENCH_H_ */
