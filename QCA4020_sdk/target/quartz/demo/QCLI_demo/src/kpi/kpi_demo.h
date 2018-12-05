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


#define M4_BOOT_PBL_TIME 1
#define M4_BOOT_SBL_TIME 2
#define M4_BOOT_RTOS_TIME 3
#define M4_BOOT_TOTAL_TIME 4

/* These indexes come from driver_cxt.h */
#define WLAN_BOOT_KF_INIT_INDEX 0
#define WLAN_BOOT_KF_POWER_ON_INDEX 18
#define WLAN_BOOT_KF_FW_DOWNLOAD_INDEX 19
#define WLAN_BOOT_KF_WMI_READY_TIME 7

/* These indexes come from driver_cxt.h */
#define WLAN_STORERECALL_KF_INIT_INDEX 10 //due storerecall store bug, index 0 is called delay for suspend time is triggered
#define WLAN_STORERECALL_KF_POWER_ON_INDEX 6
#define WLAN_STORERECALL_KF_FW_DOWNLOAD_INDEX 7
#define WLAN_STORERECALL_KF_DONE_TIME 17

#define FOUR_WAY_HANDSHAKE_COMPLETION 0x10
#define MAX_PASSPHRASE_LENGTH 64

#define WLAN_MAX_SSID_LEN 32
#define WLAN_WPA_KEY_MAX_LEN 63

#define DECISION_TIME_TO_SUSPEND 7
#define TIME_NEEDED_FOR_WAKE_UP 6

#define KPI_DEMO_PACKET_SIZE 1400
#define KPI_ONB_MAX_TX_SIZE  1576
#define KPI_ONB_MAX_RX_SIZE  1556

/*count based on 1400 byte packet size 
 * ((10700000/8)/1400) accounting for packet loss
 */
#define ONE_MEGABIT_RATE_PACKET_COUNT 96

#define KPI_SEC_MODE_OPEN    0
#define KPI_SEC_MODE_WPA     1

#define RTC_CLOCK_MULTIPLIER 3125
#define RTC_CLOCK_DIVIDER 100000

#define ONE_SECOND_DELAY 1000000
#define WLAN_ONE_SEC_SUSPEND 1000

#define STORERECALL_TEST_NORMAL       0
#define STORERECALL_TEST_CONNECT_TIME 1
#define STORERECALL_TEST_IP_PACKET    2

/* type 1 - create file and close it */
/* type 2 - write a file */
/* type 3 - read a file */
/* type 4 - do all the operations listed above */
#define KPI_SECUREFS_CREATE_FILE 1
#define KPI_SECUREFS_WRITE_FILE 2
#define KPI_SECUREFS_READ_FILE 3
#define KPI_SECUREFS_ALL 4

#define KPI_SECUREFS_BUFSIZE 1024

#define END_OF_TEST_CODE (0xAABBCCDD)

#define KPI_ERROR -1

enum Test_Mode
{
    TIME_TEST,
    PACKET_TEST
};

enum tput_protocol {
    UDP,   //UDP Transmit (Uplink Test)
    TCP,   //TCP Receive (Downlink Test)
    SSL,   //SSL Transmit (Uplink Test)
    IP_RAW, //SOCKET_RAW TX
    IP_RAW_HDR, //SOCKET_RAW TX with IP_HDRINCL enabled
};

#define KPI_GPIO_HIGH 1
#define KPI_GPIO_LOW  2

/* GPIO toggle mechanism */
#define KPI_TIME_CHECK_GPIO 55 // Free GPIO?

#define KPI_RESUME_DONE_EVENT_MASK     0x1
#define KPI_DHCP_EVENT_MASK            0x2
#define KPI_CONNECT_EVENT_MASK         0x4
#define KPI_ONBOARDING_SSID_FOUND_MASK 0x8
#define KPI_STA_CONN_EVENT_MASK        0x16
#define KPI_STA_DISCONN_EVENT_MASK     0x20
#define KPI_CONNECT_ASSOC_MASK         0x24

#define KPI_DEMO_TCP 0
#define KPI_DEMO_UDP 1

#define ONBOARDING_PORT 5000

typedef struct ipv4_header_kpi {
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

typedef struct
{
    uint32_t    seconds;        /* number of seconds */
    uint32_t    milliseconds;   /* number of milliseconds */
    uint32_t    ticks;          /* number of systicks */
} time_struct_t;

typedef struct onb_params
{
    uint32_t ssid_length;
    uint32_t passphrase_length;
    char ssid[WLAN_MAX_SSID_LEN];
    char passphrase[WLAN_WPA_KEY_MAX_LEN];
} onboarding_params;

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

/* 
 * main KPI demo ctx
 */
typedef struct kpi_demo_ctx
{
    char *ssid;                     /* SSID for station */
    char *bssid;                    /* BSSID for station */
    char *wpa2_psk;                 /* WPA2 passphrase for station */
    uint32_t operating_channel;
    uint32_t total_packet_count;
    uint32_t assoc_time_stamp;      /* store association tick count */
    char ip[20];                    /* storage for IP string */
    uint32_t dest_ip;               /* destination ip address */
    qurt_signal_t wlan_kpi_event;
    uint32_t security_mode;
    uint32_t protocol;              /* 1:TCP 2:UDP 4:SSL*/
    uint16_t port;
    int32_t sock_local;             /* Listening socket.*/
    int32_t sock_peer;              /* Foreign socket.*/
    int32_t rxcode;                 /* event code from rx_upcall */
    char* buffer;
    STATS pktStats;
    uint32_t is_onboarding_ssid_found;
    uint32_t device_ipv4_addr;

    union params_u
    {
        TX_PARAMS tx_params;
        RX_PARAMS rx_params;
    } params;

}kpi_demo_ctx;

typedef struct kpi_end_of_test {
    int code;
    int packet_count;
} KPI_EOT_PACKET;


void kpi_udp_cumulative_tx();
uint32_t kpi_demo_socket_connect(uint8_t type);
int32_t kpi_demo_send_packet_test(uint32_t packet_size, uint32_t packet_count);
int32_t wlan_connect(char *conn_ssid, char *passphrase);
void Initialize_KPI_Demo(); 

