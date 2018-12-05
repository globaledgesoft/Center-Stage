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

#include <stdio.h>
#include "qurt_types.h"
#include "qurt_thread.h"
#include "qurt_timer.h"
#include "qcli_api.h"   /* QCLI_Printf */
#include "netutils.h"

extern QCLI_Group_Handle_t qcli_net_handle; /* Handle for Net Command Group. */

static const char hexchar[] = "0123456789ABCDEF";

/*****************************************************************************
 * add item to the q's tail
 *****************************************************************************/
void enqueue(QUEUE_T *q, void *item)
{
    qp elt = (qp)item;

    INTR_DISABLE();

    elt->qe_next = NULL;
    if (q->q_head == NULL)
    {
        q->q_head = elt;
    }
    else
    {
        q->q_tail->qe_next = elt;
    }

    q->q_tail = elt;
    ++(q->q_len);

    INTR_ENABLE();

    return;
}

/*****************************************************************************
 * remove item from the q's head
 *****************************************************************************/
void * dequeue(QUEUE_T *q)
{
    qp elt;        /* elt for result */

    INTR_DISABLE();

    if ((elt = q->q_head) != NULL)  /* queue is not empty */
    {
        q->q_head = elt->qe_next;   /* unlink */
        elt->qe_next = NULL;        /* avoid dangling pointer */
        if (q->q_head == NULL)      /* queue is empty */
        {
            q->q_tail = NULL;       /* update tail pointer too */
        }
        q->q_len--;                 /* update queue length */
    }

    INTR_ENABLE();

    return ((void*)elt);
}

/*****************************************************************************
 *****************************************************************************/
uint32_t app_get_time(time_struct_t *time)
{
    uint32_t ticks, ms;

    ticks = (uint32_t)qurt_timer_get_ticks();
    ms = (uint32_t)qurt_timer_convert_ticks_to_time(ticks, QURT_TIME_MSEC);

    if (time)
    {
        time->seconds = ms / 1000;
        time->milliseconds = ms % 1000;
        time->ticks = ticks;
    }

    return ms;
}

/*****************************************************************************
 * Return time difference in milliseconds
 *****************************************************************************/
uint32_t app_get_time_difference(time_struct_t *time1, time_struct_t *time2)
{
    uint32_t duration;          /* in ticks */
    uint32_t last_tick = time2->ticks;
    uint32_t first_tick = time1->ticks;

    if (last_tick < first_tick)
    {
        /* Assume the systick wraps around once */
        duration = ~first_tick + 1 + last_tick;
    }
    else
    {
        duration = last_tick - first_tick;
    }

    return qurt_timer_convert_ticks_to_time(duration, QURT_TIME_MSEC);
}

/*****************************************************************************
 *****************************************************************************/
void app_msec_delay(uint32_t ms)
{
    uint32_t ticks;

    ticks = qurt_timer_convert_time_to_ticks(ms, QURT_TIME_MSEC);
    if (ms != 0 && ticks == 0)
    {
        ticks = 1;
    }
    qurt_thread_sleep(ticks);

    return;
}

/*****************************************************************************
 * FUNCTION: app_hexdump()
 *
 * Simple hexdump with optional ASCII
 *
 * PARAM2: void *           input buffer
 * PARAM3: unsigned         input buffer length
 * PARAM4: int              TRUE = include ASCII dump
 * PARAM5: int              TRUE = include [address]
 *
 * RETURNS: none
 *****************************************************************************/
void app_hexdump(void *inbuf, unsigned inlen, int ascii, int addr)
{
    uint8_t *cp = (uint8_t *)inbuf;
    uint8_t *ap = (uint8_t *)inbuf;
    int len = (int)inlen;
    int clen, alen, i;
    char outbuf[96];
    char *outp = &outbuf[0];
    int  line = 0;

    memset(outbuf, 0, sizeof(outbuf));
    while (len > 0)
    {
        if (addr)
            outp += snprintf(outp, sizeof(outbuf), "[%p] ", cp);

        clen = alen = min(HEX_BYTES_PER_LINE, len);

        /* display data in hex */
        for (i = 0; i < HEX_BYTES_PER_LINE; i++)
        {

            if (--clen >= 0)
            {
                uint8_t uc = *cp++;

                *outp++ = hexchar[(uc >> 4) & 0x0f];
                *outp++ = hexchar[(uc) & 0x0f];
                *outp++ = ' ';
            }
            else if (line != 0)
            {
                *outp++ = ' ';
                *outp++ = ' ';
                *outp++ = ' ';
            }
        }

        if (ascii)
        {
            *outp++ = ' ';
            *outp++ = ' ';

            /* display data in ascii */
            while (--alen >= 0)
            {
                uint8_t uc = *ap++;

                *outp++ = ((uc >= 0x20) && (uc < 0x7f)) ? uc : '.';
            }
        }

        /* output the line */
        *outp++ = '\n';
        //print_line(outbuf, outp - &outbuf[0]);
        QCLI_Printf(qcli_net_handle, "%s", outbuf);

        memset(outbuf, 0, sizeof(outbuf));
        outp = &outbuf[0];
        len -= HEX_BYTES_PER_LINE;
        line++;
    } /* while (len > 0) */
    return;
}

/*****************************************************************************
 * '0'  -> 0x0
 * 'a'  -> 0xa
 * 'A'  -> 0xa
 *****************************************************************************/
static int a2n(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

/*****************************************************************************
 * hwaddr_pton - Convert ASCII string to MAC address
 * @txt: MAC address as a NULL-terminated string (e.g., "00:11:22:33:44:55")
 * @addr: Buffer for the MAC address (ET_ADDRLEN = 6 bytes)
 * Returns: 0 on success, -1 on failure (e.g., string not a MAC address)
 * For example:
 * "00:11:22:33:44:55" -> 0x00,0x11,0x22,0x33,0x44,0x55
 * "00-11-22-33-44-55" -> 0x00,0x11,0x22,0x33,0x44,0x55
 * "00.11.22.33.44.55" -> 0x00,0x11,0x22,0x33,0x44,0x55
 *****************************************************************************/
int hwaddr_pton(const char *txt, uint8_t *addr, size_t buflen)
{
    int i;
    const char *pos = txt;

    if (txt == NULL || addr == NULL || buflen < ET_ADDRLEN)
    {
        return -1;
    }

    for (i = 0; i < ET_ADDRLEN; i++)
    {
        int a, b;

        while (*pos == ':' || *pos == '.' || *pos == '-')
        {
            pos++;
        }

        a = a2n(*pos++);
        if (a < 0)
            return -1;
        b = a2n(*pos++);
        if (b < 0)
            return -1;
        *addr++ = (a << 4) | b;
    }

    return 0;
}

/*
 * "ab" -> 0xab
 */
int hex2byte(const char *hex)
{
	int a, b;

	a = a2n(*hex++);
	if (a < 0)
		return -1;

	b = a2n(*hex++);
	if (b < 0)
		return -1;

	return (a << 4) | b;
}

/**
 * hexstr2bin - Convert ASCII hex string into binary data
 * @hex: ASCII hex string (e.g., "01ab")
 * @buf: Buffer for the binary data
 * @len: Length of the hex string to convert in bytes (of buf)
 * Returns: # of bytes in buf on success, -1 on failure (invalid hex string)
 * "01ab05" -> 0x01 0xab 0x05
 * "01 ab 05" -> 0x01 0xab 0x05
 */
int hexstr2bin(const char *hex, uint8_t *buf, size_t len)
{
	int a;
	const char *ipos = hex;
	uint8_t *opos = buf;

    while ((ipos - hex) < len)
    {
        /* skip delimiter */
        while (*ipos == ':' || *ipos == '.' || *ipos == '-' || *ipos == ' ')
        {
            ipos++;
            if (ipos - hex >= len)
            {
                goto end;
            }
        }

		a = hex2byte(ipos);
		if (a < 0)
			return -1;

		*opos++ = a;
		ipos += 2;
	}

end:
	return (opos - buf);
}
