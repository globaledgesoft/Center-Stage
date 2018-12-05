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
#include "adss_mem.h"

#include "netutils.h"


extern QCLI_Group_Handle_t qcli_adss_group;              /* Handle for our QCLI Command Group. */

#define ADSS_INFO_PRINTF(args...) QCLI_Printf(qcli_adss_group,args)

#define DEBUG_ADSS_MEM_PRINTF
#if defined(DEBUG_ADSS_MEM_PRINTF)
#define ADSS_DEBUG_PRINTF(args...) QCLI_Printf(qcli_adss_group,args)
#else
#define ADSS_DEBUG_PRINTF(args...)
#endif

/*
 *  buffer management
 */

ADSS_MEM_SESSION_t  *m_pAdssMem=NULL;

ADSS_RET_STATUS  adss_init_buf_link(int head_count)
{
	ADSS_DATA_BUF_LINK_t  *p;
	uint32_t   i;

    ADSS_DEBUG_PRINTF("Mem Init head count:%d\r\n", head_count);
	if (m_pAdssMem != NULL)
	{
		ADSS_DEBUG_PRINTF("more than one time Mem Init\r\n");
		return ADSS_NO_MEMORY;
	}
	
    m_pAdssMem = (ADSS_MEM_SESSION_t *)malloc(sizeof(ADSS_MEM_SESSION_t));
    if (m_pAdssMem == NULL) {
        ADSS_DEBUG_PRINTF("No Mem\r\n");
        return ADSS_NO_MEMORY;
    }
	memset(m_pAdssMem, 0, sizeof(ADSS_MEM_SESSION_t));
	
    m_pAdssMem->m_pLinkBuffAddr = (ADSS_DATA_BUF_LINK_t *)malloc(sizeof(ADSS_DATA_BUF_LINK_t) * head_count);

    if (m_pAdssMem->m_pLinkBuffAddr == NULL) {
        ADSS_DEBUG_PRINTF("No Mem\r\n");
		free(m_pAdssMem);
		m_pAdssMem = NULL;
        return ADSS_NO_MEMORY;
    }
	
	qurt_signal_init(&m_pAdssMem->buf_signal);	
	qurt_mutex_init(&m_pAdssMem->buf_empty_mutex);
	qurt_signal_init(&m_pAdssMem->buf_empty_signal);	
	qurt_mutex_init(&m_pAdssMem->buf_data_mutex);
	qurt_signal_init(&m_pAdssMem->buf_data_signal);
	qurt_mutex_init(&m_pAdssMem->buf_free_link_mutex);

//	p = m_pAdssMem->m_pFreeLink;
//	p->next = NULL;
//	p++;

	p = m_pAdssMem->m_pLinkBuffAddr;
	m_pAdssMem->m_pFreeLink = NULL;
	for (i=0; i < head_count; i++, p++)
	{
        ADSS_DEBUG_PRINTF("link head addr:%p\r\n", p);		
		adss_release_buf_link(p);
	}	

	return 0;
}

ADSS_RET_STATUS  adss_Deinit_buf_link()
{
    ADSS_DEBUG_PRINTF("Mem Deinit\r\n");
	if (m_pAdssMem == NULL)
	{
		ADSS_DEBUG_PRINTF("mem no initializing\r\n");
		return ADSS_NO_MEMORY;
	}
	
    if (m_pAdssMem->m_pLinkBuffAddr != NULL) {
		free(m_pAdssMem->m_pLinkBuffAddr);
    }
	
	qurt_signal_delete(&m_pAdssMem->buf_signal);	
	qurt_signal_delete(&m_pAdssMem->buf_empty_signal);	
	qurt_signal_delete(&m_pAdssMem->buf_data_signal);
	
	qurt_mutex_delete(&m_pAdssMem->buf_empty_mutex);
	qurt_mutex_delete(&m_pAdssMem->buf_data_mutex);
	qurt_mutex_delete(&m_pAdssMem->buf_free_link_mutex);

	free(m_pAdssMem);
	m_pAdssMem = NULL;
	
	return 0;
}

ADSS_DATA_BUF_LINK_t  *adss_get_buf_link()
{
	ADSS_DATA_BUF_LINK_t  *p;
	
	qurt_mutex_lock(&m_pAdssMem->buf_free_link_mutex);	
	p = m_pAdssMem->m_pFreeLink;
	if (p)	
		m_pAdssMem->m_pFreeLink = p->next;	
	qurt_mutex_unlock(&m_pAdssMem->buf_free_link_mutex);	/* unlock */
	return p;
}

void adss_release_buf_link(ADSS_DATA_BUF_LINK_t *p)
{
	qurt_mutex_lock(&m_pAdssMem->buf_free_link_mutex);	
	p->next = m_pAdssMem->m_pFreeLink;
	m_pAdssMem->m_pFreeLink = p;	
	qurt_mutex_unlock(&m_pAdssMem->buf_free_link_mutex);	/* unlock */
} 

uint8_t *adss_get_tcp_data_buf()
{
	uint32_t num = 0;
	uint32_t  signals;
	ADSS_DATA_BUF_LINK_t  *p;
	uint8_t *pdata;
	
	if(m_pAdssMem == NULL)
	{
		ADSS_DEBUG_PRINTF("m_pAdssMem == NULL\r\n");
		
		do {
		} while(1);
	}

	do {
		qurt_mutex_lock(&m_pAdssMem->buf_data_mutex);
		if (m_pAdssMem->m_pDataLink != NULL)
		{
			p = m_pAdssMem->m_pDataLink;
			m_pAdssMem->m_pDataLink = p->next;
			num = 1;
			
#if  defined(STREAM_SPEED_CONTROL)
			m_pAdssMem->buf_data_count --;
#endif
		}	
		qurt_mutex_unlock(&m_pAdssMem->buf_data_mutex);	/* unlock */
		if (num != 0)
		{
			pdata = p->buf_ptr;
			adss_release_buf_link(p);

			return pdata;			
		}
				
		signals = qurt_signal_wait(&m_pAdssMem->buf_data_signal, ADSS_DATA_BUF_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);	
	} while (signals & ADSS_DATA_BUF_AVAIL_SIG_MASK);

    return NULL;	
}

uint8_t *adss_peek_get_tcp_data_buf()
{
	uint32_t num = 0;
	ADSS_DATA_BUF_LINK_t  *p;
	uint8_t *pdata;
	
	do {
		qurt_mutex_lock(&m_pAdssMem->buf_data_mutex);
		if (m_pAdssMem->m_pDataLink != NULL)
		{
			p = m_pAdssMem->m_pDataLink;
			m_pAdssMem->m_pDataLink = p->next;
			num = 1;
		}	
		qurt_mutex_unlock(&m_pAdssMem->buf_data_mutex);	/* unlock */
		if (num != 0)
		{
			pdata = p->buf_ptr;
			adss_release_buf_link(p);

			return pdata;			
		}				
	} while (0);
	
    return NULL;	
}

uint8_t *adss_put_tcp_data_buf(uint8_t *pbuf)
{
	ADSS_DATA_BUF_LINK_t  *p;

	p = adss_get_buf_link();	
    p->buf_ptr = pbuf;
	
	p->next = NULL;			
	qurt_mutex_lock(&m_pAdssMem->buf_data_mutex);
	if (m_pAdssMem->m_pDataLink != NULL)
	{
		m_pAdssMem->m_pDataLink_Trail->next = p;
		m_pAdssMem->m_pDataLink_Trail = p;	
	}
	else 
	{
		m_pAdssMem->m_pDataLink = p;		
		m_pAdssMem->m_pDataLink_Trail = p;		
	}
#if  defined(STREAM_SPEED_CONTROL)
	m_pAdssMem->buf_data_count ++;
#endif
		
	qurt_mutex_unlock(&m_pAdssMem->buf_data_mutex);	/* unlock */
				
	qurt_signal_set(&m_pAdssMem->buf_data_signal, ADSS_DATA_BUF_AVAIL_SIG_MASK);
	return  NULL;
}

uint8_t *adss_get_tcp_empty_buf()
{
	uint32_t num = 0;
	uint32_t  signals;
	ADSS_DATA_BUF_LINK_t  *p;
	uint8_t  *pempty;
	
	do {
		qurt_mutex_lock(&m_pAdssMem->buf_empty_mutex);
		if (m_pAdssMem->m_pEmptyLink != NULL)
		{
			p = m_pAdssMem->m_pEmptyLink;
			m_pAdssMem->m_pEmptyLink = p->next;
#if  defined(STREAM_SPEED_CONTROL)
			m_pAdssMem->buf_empty_count --;
#endif
			num = 1;
		}	
		qurt_mutex_unlock(&m_pAdssMem->buf_empty_mutex);	/* unlock */
		if (num != 0)
		{
			pempty = p->buf_ptr;
			adss_release_buf_link(p);
			return 	pempty;		
		}
				
		signals = qurt_signal_wait(&m_pAdssMem->buf_empty_signal, ADSS_EMPTY_BUF_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);	
	} while (signals & ADSS_EMPTY_BUF_AVAIL_SIG_MASK);

    return NULL;	
}

uint8_t *adss_peek_get_tcp_empty_buf()
{
	uint32_t num = 0;
	ADSS_DATA_BUF_LINK_t  *p;
	uint8_t  *pempty;
	
	do {
		qurt_mutex_lock(&m_pAdssMem->buf_empty_mutex);
		if (m_pAdssMem->m_pEmptyLink != NULL)
		{
			p = m_pAdssMem->m_pEmptyLink;
			m_pAdssMem->m_pEmptyLink = p->next;
			num = 1;
		}	
		qurt_mutex_unlock(&m_pAdssMem->buf_empty_mutex);	/* unlock */
		if (num != 0)
		{
			pempty = p->buf_ptr;
			adss_release_buf_link(p);
			return 	pempty;		
		}
				
//		qurt_signal_wait(&m_pAdssMem->buf_empty_signal, ADSS_USR_SEND_DONE_SIG_MASK, QURT_SIGNAL_ATTR_CLEAR_MASK);	
	} while (0);
	
	return NULL;
}

void adss_put_tcp_empty_buf(uint8_t *pbuf)
{
	ADSS_DATA_BUF_LINK_t  *p;
	
    p = adss_get_buf_link();	
	p->buf_ptr = pbuf;
	
	qurt_mutex_lock(&m_pAdssMem->buf_empty_mutex);
	if (m_pAdssMem->m_pEmptyLink != NULL)
	{
		p->next = m_pAdssMem->m_pEmptyLink;
	}
	else
	{
		p->next = NULL;
	}
	m_pAdssMem->m_pEmptyLink = p;

#if  defined(STREAM_SPEED_CONTROL)
	m_pAdssMem->buf_empty_count ++;
#endif
	
	qurt_mutex_unlock(&m_pAdssMem->buf_empty_mutex);	/* unlock */
		
	qurt_signal_set(&m_pAdssMem->buf_empty_signal, ADSS_EMPTY_BUF_AVAIL_SIG_MASK);	
}

#if  defined(STREAM_SPEED_CONTROL)
uint32_t adss_get_empty_buf_count()
{
	uint32_t   count;
	
	qurt_mutex_lock(&m_pAdssMem->buf_empty_mutex);
	count = m_pAdssMem->buf_empty_count;
	m_pAdssMem->buf_empty_count = 0;
	qurt_mutex_unlock(&m_pAdssMem->buf_empty_mutex);	/* unlock */
	
	return count;
}
#endif
