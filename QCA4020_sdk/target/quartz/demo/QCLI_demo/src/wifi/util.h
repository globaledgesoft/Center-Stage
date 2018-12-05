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

#define BE2CPU16(x)   ((uint16_t)(((x)<<8)&0xff00) | (uint16_t)(((x)>>8)&0x00ff))
#define BE2CPU32(x)   ((((x)<<24)&0xff000000) | (((x)&0x0000ff00)<<8) | (((x)&0x00ff0000)>>8) | (((x)>>24)&0x000000ff))
#define LE2CPU16(x)   (x)
#define LE2CPU32(x)   (x)
#define CPU2BE16(x)   ((uint16_t)(((x)<<8)&0xff00) | (uint16_t)(((x)>>8)&0x00ff))
#define CPU2BE32(x)   ((((x)<<24)&0xff000000) | (((x)&0x0000ff00)<<8) | (((x)&0x00ff0000)>>8) | (((x)>>24)&0x000000ff))
#define CPU2LE32(x)   (x)
#define CPU2LE16(x)   (x)



#define htonl(l) (((((l) >> 24) & 0x000000ff)) | \
                 ((((l) >>  8) & 0x0000ff00)) | \
                 (((l) & 0x0000ff00) <<  8) | \
                 (((l) & 0x000000ff) << 24))
#define ntohl(l) htonl(l)
#define htons(s) ((((s) >> 8) & 0xff) | \
                 (((s) << 8) & 0xff00))
#define ntohs(s) htons(s)


#define AF_INET              2     /* internetwork: UDP, TCP, etc. */
#define AF_INET6             3     /* IPv6 */

int32_t check_empty_ip_addr_string(const char *src);
uint8_t ascii_to_hex(char val);
char hex_to_ascii(uint8_t val);
uint32_t mystrtoul(const char* arg, const char* endptr, int base);
int32_t parse_ipv4_ad(unsigned long *ip_addr, unsigned *sbits, char *stringin);
uint32_t ether_aton(const char *orig, uint8_t *eth);

#ifndef __ICCARM__           /* IAR */
int ishexdigit(char digit);
#endif
