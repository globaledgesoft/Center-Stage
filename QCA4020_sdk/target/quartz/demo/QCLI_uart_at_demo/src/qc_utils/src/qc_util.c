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
#include <stdlib.h>     /* atoi() */
#include <string.h>     /* strchr() */
#include "qc_util.h"
#include "qcli_api.h"
#include "qurt_thread.h"
#include "stringl.h"

extern QCLI_Context_t QCLI_Context;

int32_t check_empty_ip_addr_string(const char *src)
{
    const char *cp;
    int32_t val;

    cp = src;
    val = 0;
    while(*cp)
    {
        if (*cp++ != ':')
        {
            val++;
            break;
        }
    }
    if(val == 0)
        return QCLI_STATUS_SUCCESS_E;
    return QCLI_STATUS_ERROR_E;

}

/*****************************************************************************
 * Function Name  : qc_util_strtoul
 * Returned Value : unsigned long
 * Comments       : coverts string to unsigned long
 *****************************************************************************/
uint32_t qc_util_strtoul(const char* arg, const char* endptr, int base)
{
    uint32_t res = 0;
    int i;

    if(arg){
        if(arg[0] == '0' && (arg[1] == 'x' || arg[1] == 'X')) arg+=2;

        i=0;
        while(arg[i] != '\0' && &arg[i] != endptr){
            if(arg[i] >= '0' && arg[i] <= '9'){
                res *= base;
                res += arg[i] - '0';
            }else if(arg[i] >= 'a' && arg[i] <= 'f' && base == 16){
                res *= base;
                res += arg[i] - 'a' + 10;
            }else if(arg[i] >= 'A' && arg[i] <= 'F' && base == 16){
                res *= base;
                res += arg[i] - 'A' + 10;
            }else{
                //error
                break;
            }

            i++;
        }

    }

    return res;
}

/*****************************************************************************/
#ifndef __ICCARM__           /* IAR */

int ishexdigit(char digit)
{
    if((digit >= '0' ) && (digit <= '9'))
        return 1;

    digit |= 0x20;       /* mask letters to lowercase */
    if ((digit >= 'a') && (digit <= 'f'))
        return QCLI_STATUS_ERROR_E;
    else
        return QCLI_STATUS_SUCCESS_E;
}

unsigned hexnibble(char digit)
{
    if (digit <= '9')
        return (digit-'0'    );

    digit &= ~0x20;   /* letter make uppercase */
    return (digit-'A')+10 ;
}


unsigned atoh(char * buf)
{
    unsigned retval = 0;
    char *   cp;
    char  digit;

    cp = buf;

    /* skip past spaces and tabs */
    while (*cp == ' ' || *cp == 9)
        cp++;

    /* while we see digits and the optional 'x' */
    while (ishexdigit(digit = *cp++) || (digit == 'x'))
    {
        /* its questionable what we should do with input like '1x234',
         * or for that matter '1x2x3', what this does is ignore all
         */
        if (digit == 'x')
            retval = 0;
        else
            retval = (retval << 4) + hexnibble(digit);
    }

    return retval;
}

#endif

/*****************************************************************************/
int32_t parse_ipv4_ad(unsigned long * ip_addr,   /* pointer to IP address returned */
        unsigned *  sbits,      /* default subnet bit number */
        char *   stringin)
{
    int error = -1;
    char *   cp;
    int   dots  =  0; /* periods imbedded in input string */
    int   number;
    union
    {
        unsigned char   c[4];
        unsigned long   l;
    } retval;

    cp = stringin;
    while (*cp)
    {
        if (*cp > '9' || *cp < '.' || *cp == '/')
            return(error);
        if (*cp == '.')dots++;
        cp++;
    }

    if ( dots < 1 || dots > 3 )
        return(error);

    cp = stringin;
    if ((number = atoi(cp)) > 255)
        return(error);

    retval.c[0] = (unsigned char)number;

    while (*cp != '.')cp++; /* find dot (end of number) */
    cp++;             /* point past dot */

    if (dots == 1 || dots == 2) retval.c[1] = 0;
    else
    {
        number = atoi(cp);
        while (*cp != '.')cp++; /* find dot (end of number) */
        cp++;             /* point past dot */
        if (number > 255) return(error);
        retval.c[1] = (unsigned char)number;
    }

    if (dots == 1) retval.c[2] = 0;
    else
    {
        number = atoi(cp);
        while (*cp != '.')cp++; /* find dot (end of number) */
        cp++;             /* point past dot */
        if (number > 255) return(error);
        retval.c[2] = (unsigned char)number;
    }

    if ((number = atoi(cp)) > 255)
        return(error);
    retval.c[3] = (unsigned char)number;

    if (retval.c[0] < 128) *sbits = 8;
    else if(retval.c[0] < 192) *sbits = 16;
    else *sbits = 24;

    *ip_addr = retval.l;
    return(QCLI_STATUS_SUCCESS_E);
}

/*****************************************************************************/

#ifndef __ICCARM__   /* IAR */

#define isdigit(c)  ( (((c) >= '0') && ((c) <= '9')) ? (1) : (0) )
/*
 * Input an Ethernet address and convert to binary.
 */

#endif

uint32_t ether_aton(const char *orig, uint8_t *eth)
{
    const char *bufp;
    int i;

    i = 0;
    for(bufp = orig; *bufp != '\0'; ++bufp) {
        unsigned int val;
        unsigned char c = *bufp++;
        if (isdigit(c)) val = c - '0';
        else if (c >= 'a' && c <= 'f') val = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') val = c - 'A' + 10;
        else break;

        val <<= 4;
        c = *bufp++;
        if (isdigit(c)) val |= c - '0';
        else if (c >= 'a' && c <= 'f') val |= c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') val |= c - 'A' + 10;
        else break;

        eth[i] = (unsigned char) (val & 0377);
        if(++i == 6) //MAC_LEN
        {
            /* That's it.  Any trailing junk? */
            if (*bufp != '\0') {
                //QCLI_Printf("iw_ether_aton(%s): trailing junk!\r\n", orig);
                return(-QCLI_STATUS_ERROR_E);
            }
            return(QCLI_STATUS_SUCCESS_E);
        }
        if (*bufp != ':')
            break;
    }
    return(-QCLI_STATUS_ERROR_E);
}

/*****************************************************************************
 * Function Name   : ascii_to_hex()
 * Returned Value  : hex counterpart for ascii
 * Comments	: Converts ascii character to correesponding hex value
 *****************************************************************************/
uint8_t ascii_to_hex(char val)
{
    if('0' <= val && '9' >= val){
        return (uint8_t)(val - '0');
    }else if('a' <= val && 'f' >= val){
        return (uint8_t)((val - 'a') + 0x0a);
    }else if('A' <= val && 'F' >= val){
        return (uint8_t)((val - 'A') + 0x0a);
    }
    return 0xff;/* Error */
}

/*****************************************************************************
 * Function Name   : hex_to_ascii()
 * Returned Value  : ascii counterpart for hex.
 * Comments	: Converts ascii character to correesponding hex value
 *****************************************************************************/
char hex_to_ascii(uint8_t val)
{
    if(0 <= val && 9 >= val) {
        return (val + '0');
    }
    else if (0xa <= val && 0xf >= val){
        return (val - 0xa + 'a');
    }
    else {
        return '\0' /* Error */;
    }
}

/*****************************************************************************
 * Function Name   : qc_util_isdigit()
 * Returned Value  : 0 on success, 1 on error
 * Comments	       : checks whether the number is numeral
 *****************************************************************************/
int qc_util_isdigit( int c)
{
    if ( c >= '0' && c <='9')
        return QCLI_STATUS_ERROR_E;
    else
        return QCLI_STATUS_SUCCESS_E;
}

int qc_util_isspace( int c)
{
    if ( c == ' ')
        return QCLI_STATUS_ERROR_E;
    else
        return QCLI_STATUS_SUCCESS_E;
}

unsigned long long qc_util_atoll(const char *p)
{
    unsigned long long n = 0;
    int c = 0, neg = 0;
    unsigned char *up = (unsigned char *)p;

    if (!qc_util_isdigit(c = *up)) {
        while (qc_util_isspace(c))
            c = *++up;
        switch (c) {
            case '-':
                neg++;
                /* FALLTHROUGH */
            case '+':
                c = *++up;
        }
        if (!qc_util_isdigit(c))
            return (0);
    }
    for (n = '0' - c; qc_util_isdigit(c = *++up); ) {
        n *= 10; /* two steps to avoid unnecessary overflow */
        n += '0' - c; /* accum neg to avoid surprises at MAX */
    }

    return (neg ? n : -n);
}

/**
  @brief This function is a wrapper for commands which start in their own
  thread.

  @param Thread_Parameter is the parameter specified when the thread was
  started. It is expected to be a pointer to a Thread_Param_t
  structure.
  */
static void Uart_Command_Thread(void *Thread_Parameter)
{
    uint32_t              Parameter_Count;
    QCLI_Parameter_t      Parameter_List[MAXIMUM_NUMBER_OF_PARAMETERS];
    char                  Input_String[MAXIMUM_QCLI_COMMAND_STRING_LENGTH + 1];
    Thread_Param_t        *Thread_Info;
    int32_t               Index;

    if(Thread_Parameter)
    {
        /* Copy the thread info to local storage. */
        Thread_Info = (Thread_Param_t*)Thread_Parameter;
        memscpy(&Input_String, sizeof(Input_String), QCLI_Context.Input_String, sizeof(QCLI_Context.Input_String));
        memscpy(&Parameter_List, sizeof(Parameter_List), Thread_Info->Parameter_List, Thread_Info->Parameter_Count * sizeof(QCLI_Parameter_t));
        Parameter_Count = Thread_Info->Parameter_Count;
        
        /* Adjust the pointers in the paramter list for the local input string. */
        for(Index = 0; Index < Thread_Info->Parameter_Count; Index ++)
        {
            Parameter_List[Index].String_Value += (Input_String - QCLI_Context.Input_String);
        }
        
        /* Execute the command. */
        (*(Thread_Info->cmd_fun))(Parameter_Count, Parameter_List);
        /* Decrement the number of active threads. */
        QCLI_Context.Thread_Count --;
    }
    
    /* Terminate the thread. */
    qurt_thread_stop();
}

//QCLI_Command_Status_t Uart_Thread_Create(Thread_Param_t Thread_Info)
QCLI_Command_Status_t Uart_Thread_Create(Uart_Cmd_Cb cmd_fun, uint32_t Parameter_Count, QCLI_Parameter_t *Parameter_List)
{
    qurt_thread_attr_t Thread_Attribte;
    qurt_thread_t      Thread_Handle;
    int                Thread_Result;
    int                ret = QCLI_STATUS_SUCCESS_E;
    Thread_Param_t     *Thread_Info;

    /* Take the mutex before modifying any global variables. */
    if(TAKE_LOCK(QCLI_Context.CLI_Mutex))
    {
        /* Make sure we haven't maxed out the number of supported threads. */
        if (QCLI_Context.Thread_Count < MAXIMUM_THREAD_COUNT)
        {
            Thread_Info = (Thread_Param_t *)malloc(sizeof(Thread_Param_t));                         
            QCLI_Context.Thread_Count ++;
            /* Make sure the running event semaphore is taken. */
    
            /* Pass the function to the thread pool. */
            Thread_Info->cmd_fun         = cmd_fun;
            Thread_Info->Parameter_Count = Parameter_Count;
            Thread_Info->Parameter_List  = Parameter_List;

            /* Create a thread for the command. */
            qurt_thread_attr_init(&Thread_Attribte);
            qurt_thread_attr_set_name(&Thread_Attribte, "Uart Commad Thread");
            qurt_thread_attr_set_priority(&Thread_Attribte, COMMAND_THREAD_PRIORITY);
            qurt_thread_attr_set_stack_size(&Thread_Attribte, THREAD_STACK_SIZE);
            Thread_Result = qurt_thread_create(&Thread_Handle, &Thread_Attribte, Uart_Command_Thread, (void *)Thread_Info);
            free(Thread_Info);
            if (Thread_Result != QURT_EOK)
            {
                QCLI_Printf("Failed to create thread for command (%d).\r\n", Thread_Result);
                QCLI_Context.Thread_Count --;                 
                RELEASE_LOCK(QCLI_Context.CLI_Mutex);        
                ret = QCLI_STATUS_ERROR_E;   
            }
        }
        else
        {
            QCLI_Printf("Max threads reached.\r\n");
            RELEASE_LOCK(QCLI_Context.CLI_Mutex);
            ret = QCLI_STATUS_ERROR_E;
        }
        RELEASE_LOCK(QCLI_Context.CLI_Mutex);
    }

    return ret;
}
