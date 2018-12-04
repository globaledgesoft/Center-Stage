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
#include <stdlib.h>     /* atoi() */
#include <string.h>     /* strchr() */
#include "util.h"

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
   		return 0;
   return 1;

}

/*****************************************************************************
 * Function Name  : mystrtoul
 * Returned Value : unsigned long
 * Comments       : coverts string to unsigned long
 *****************************************************************************/
uint32_t mystrtoul(const char* arg, const char* endptr, int base)
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
      return 1;
   else
      return 0;
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
   return(0);
}

/*****************************************************************************/

#ifndef __ICCARM__   /* IAR */

char *pton_error = "";
int inet_pton(int af, const char *src, void *dst)
{
   const char *cp;      /* char after previous colon */
   uint16_t    *dest;    /* word pointer to dst */
   int         colons;  /* number of colons in src */
   int         words;   /* count of words written to dest */

   /* RFC 2133 wants us to support both types of address */
   if (af == AF_INET)    /* wants a v4 address */
   {
      unsigned long ip4addr;
      unsigned sbits;
      int32_t err;
       
      err = parse_ipv4_ad(&ip4addr, &sbits, (char *)src);
      if (err == (int32_t)NULL)
      {
         /* copy the parsed address into caller's buffer, and 
          * return success
          */
         memcpy(dst, &ip4addr, sizeof (unsigned long));
         return (0);
      }
      else
      {
         /* return failure */
         pton_error = "IPv4 address parse failure";
         return (1);
      }
   }

   if (af != AF_INET6)
   {
      pton_error = "bad domain";
      return -1;
   }

   /* count the number of colons in the address */
   cp = src;
   colons = 0;
   while(*cp)
   {
      if (*cp++ == ':') 
         colons++;
   }
   if (colons < 2 || colons > 7)
   {
      pton_error = "must have 2-7 colons";
      return 1;
   }

   /* loop through address text, parseing 16-bit chunks */
   cp = src;
   dest = dst;
   words = 0;

   if (*cp == ':') /* leading colon has implied zero, e.g. "::1" */
   {
      *dest++ = 0;
      words++;
      cp++;       /* bump past leading colon, eg ":!" */
   }

   while (*cp > ' ')
   {
      if (words >= 8)
      {
	 //     dprintf("***  inet_pton: logic error?\n");
         //DTRAP();    /* logic error? */
         pton_error = "internal";
         return 1;
      }
      if (*cp == ':')   /* found a double-colon? */
      {
         int i;
         for (i = (8 - colons); i > 0; i--)
         {
            *dest++ = 0;   /* add zeros to dest address */
            words++;       /* count total words */
         }
         cp++;             /* bump past double colon */
         if (*cp <= ' ')    /* last colon was last char? */
         {
            *dest++ = 0;   /* add one final zero */
            words++;       /* count total words */
         }
      }
      else
      {
         uint16_t wordval;

         wordval = htons(atoh( (char *)cp));    /* get next 16 bit word */
         if ((wordval == 0) && (*cp != '0'))  /* check format */
         {
            pton_error = "must be hex numbers or colons";
            return 1;
         }
         *dest++ = wordval;
         words++;       /* count total words set in dest */
         cp = strchr((char *)cp, ':');   /* find next colon */
         if (cp)                  /* bump past colon */
            cp++;
         else                 /* no more colons? */
            break;            /* done with parsing */
      }
   }
   if (words != 8)
   {
      pton_error = "too short - missing colon?";
      return 1;
   }
   return 0;
}

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
            //QCLI_Printf(qcli_wlan_group, "iw_ether_aton(%s): trailing junk!\r\n", orig);
            return(-1);
        }
        return(0);
    }
    if (*bufp != ':')
        break;
  }
  return(-1);
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