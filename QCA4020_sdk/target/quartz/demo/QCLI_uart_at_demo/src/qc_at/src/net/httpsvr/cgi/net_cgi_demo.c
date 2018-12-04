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

#include <stdint.h>
#include <stdlib.h>     /* malloc */
#include <stdio.h>
#include <string.h>
#include "net_htmldata.h"
#include "qapi_webs.h"
#include "net_utils.h"
#include "qcli_api.h"

//extern int strnicmp(const char * s1, const char * s2, int len);

#define PRINTF(fmt, args...)                QCLI_Printf(fmt, ## args)
#define HEXDUMP(inbuf, inlen, ascii, addr)  app_hexdump(inbuf, inlen, ascii, addr)

#define RESP_SIZE   512
static char *resp;

/****************************************************************************
 ***************************************************************************/
static int
demo_set(void *hp, void *f, char **filetext, qbool_t is_post)
{
    qapi_Net_Web_Form_t *form = (qapi_Net_Web_Form_t *)f;
    uint32_t i;
    uint32_t len;
    char buf[64];
    char *p = NULL;
    char *cmd;

    cmd = is_post ? "demo_post" : "demo_put";

    if (form->count == 0)
    {
        /* Get message length */
        if (qapi_Net_Webs_Get_Message_Body(hp, NULL, &len) != QAPI_OK)
        {
            return FP_ERR;
        }

        if (len == 0)
        {
            PRINTF("%s: ERROR: body length is zero.\n", cmd);
            return FP_BADREQ;
        }

        p = buf;

        /* Only allocate a memory if message length is large */
        if (len > sizeof(buf))
        {
            p = malloc(len);
            if (p == NULL)
            {
                return FP_ERR;
            }
        }
        qapi_Net_Webs_Get_Message_Body(hp, p, &len);

        /* Process received data */
        PRINTF("%s: len %u\n", cmd, len);
        HEXDUMP((char *)p, len, true, true);

        /* Return status to client */
        if (is_post) /* POST */
        {
            /* send headers */
            /* HTTP/1.1 200 OK */
            snprintf(resp, RESP_SIZE, "%s: Received %lu bytes.\n", cmd, len);

            qapi_Net_Webs_Send_HTTP_headers(
                    hp,
                    "application/demo_set",
                    strlen(resp) + len,
                    200,
                    "OK",
                    NULL);

            /* send received data back */
            qapi_Net_Webs_Send_String(hp, resp);
            qapi_Net_Webs_Send_Data(hp, p, len);
        }
        else /* PUT */
        {
            /* send headers */
            /* HTTP/1.1 204 No Content */
            qapi_Net_Webs_Send_HTTP_headers(
                    hp,
                    NULL,
                    0,
                    204,
                    "No Content",
                    NULL);
        }

        if (p != buf)
        {
            free(p);
        }
    }
    else
    {
        /* Process received data */
        len = snprintf(resp, RESP_SIZE, "%s: %s  Number of name/value pairs: %lu\n",
                        cmd, form->request_Line, form->count);

        for (i = 0; i < form->count && RESP_SIZE > len; ++i)
        {
            len += snprintf(&resp[len], RESP_SIZE - len, " %lu %s = %s\n",
                            i+1, form->name_Value[i].name, form->name_Value[i].value);
        }

        /* print to console */
        PRINTF("%s", resp);

        /* Return status to client */
        /* send headers */
        qapi_Net_Webs_Send_HTTP_headers(
                hp,
                "application/demo_set",
                len,
                200,
                "OK",
                NULL);

        /* send body */
        qapi_Net_Webs_Send_String(hp, resp);
    }

	return (FP_DONE); /* generic return */
}

/****************************************************************************
 ***************************************************************************/
static int
demo_head(void *hp, void *f, char **filetext)
{
    qapi_Net_Web_Form_t *form = (qapi_Net_Web_Form_t *)f;
    char user_headers[] =
        {"ETag: \"737060cd8c284d8af7ad3082f209582d\"\r\n"
         "Expires: Mon, 01 Jan 2018 06:00:00 GMT\r\n"
         "Pragma: no-cache\r\n"};

    PRINTF("%s: %s\n", __func__, form->request_Line);

    /* send headers */
    qapi_Net_Webs_Send_HTTP_headers(
            hp,
            "application/demo_head",
            0,
            200,
            "OK",
            user_headers);

    return (FP_DONE); /* generic return */
}

/****************************************************************************
 ***************************************************************************/
static int
demo_get(void *hp, void *f, char **filetext)
{
    qapi_Net_Web_Form_t *form = (qapi_Net_Web_Form_t *)f;
    uint32_t i, len;

    if (form->count == 0)
    {
        len = snprintf(resp, RESP_SIZE, "%s: %s  No form data after URI\n",
                        __func__, form->request_Line);

        /* send headers */
        qapi_Net_Webs_Send_HTTP_headers(
                hp,
                "application/demo_get",
                len,
                200,
                "OK",
                NULL);
    }
    else
    {
        /* Process received data */
        len = snprintf(resp, RESP_SIZE, "%s: %s  Number of name/value pairs: %lu\n",
                        __func__, form->request_Line, form->count);

        for (i = 0; i < form->count && RESP_SIZE > len; ++i)
        {
            len += snprintf(&resp[len], RESP_SIZE - len, " %lu %s = %s\n",
                        i+1, form->name_Value[i].name, form->name_Value[i].value);
        }

        /* Return status to client */
        /* send headers */
        qapi_Net_Webs_Send_HTTP_headers(
                hp,
                "application/demo_get",
                len,
                200,
                "OK",
                NULL);
    }
    /* send body */
    qapi_Net_Webs_Send_String(hp, resp);

    /* Also print to console */
    PRINTF("%s", resp);

    return (FP_DONE); /* generic return */
}

/****************************************************************************
 ***************************************************************************/
int
cgi_demo(void *hp, void *f, char **filetext)
{
    int rc = FP_BADREQ;
    qapi_Net_Web_Form_t *form = (qapi_Net_Web_Form_t *)f;

    if (form == NULL)
    {
        PRINTF("%s: ERROR: NULL form\n", __func__);
        return (FP_BADREQ);
    }

    resp = malloc(RESP_SIZE);
    if (resp == NULL)
    {
        return FP_ERR;
    }
    memset(resp, 0, RESP_SIZE);

    if ((strncmp(form->request_Line, "GET", 3) == 0) ||
    	(strncmp(form->request_Line, "get", 3) == 0))
    {
        rc = demo_get(hp, f, filetext);
    }
    else if ((strncmp(form->request_Line, "HEAD", 4) == 0) ||
		(strncmp(form->request_Line, "head", 4) == 0))
    {
        rc = demo_head(hp, f, filetext);
    }
    else if ((strncmp(form->request_Line, "PUT", 3) == 0) ||
		(strncmp(form->request_Line, "put", 3) == 0))
    {
        rc = demo_set(hp, f, filetext, 0);
    }
    else if ((strncmp(form->request_Line, "POST", 4) == 0) ||
		(strncmp(form->request_Line, "post", 4) == 0))
    {
        rc = demo_set(hp, f, filetext, 1);
    }
    else
    {
        PRINTF("%s: ERROR: '%s' not supported\n", __func__, form->request_Line);
        rc = FP_BADREQ;
    }

    free(resp);

    return rc;
}
