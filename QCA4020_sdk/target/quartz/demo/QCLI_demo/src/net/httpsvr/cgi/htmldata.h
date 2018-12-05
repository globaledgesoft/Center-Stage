/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
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

/* htmldata.h

   This file was built from 7 entities in input.txt.

*/

#ifndef _web_H_
#define _web_H_ 1

#ifndef NULL
#define NULL 0
#endif

struct vfs_file
{
	struct vfs_file *next;
	char name[32];
	uint16_t flags;
	uint8_t *data;
	uint32_t real_size;
	uint32_t comp_size;
	uint32_t buf_size;
	int (*cgi_func)(void *, void *, char **);
	void *method;
};

/* Status returned from cgi_func(void *, void *, char **) */
#define	FP_ERR		0x110
#define	FP_FILE		0x140
#define	FP_DONE		0x200
#define	FP_BADREQ	400
/* CGI routines */
extern int	cgi_setintf(void *, void *, char **);
extern int	cgi_showintf(void *, void *, char **);
extern int	cgi_demo(void *, void *, char **);
extern int	cgi_demo(void *, void *, char **);
extern int	cgi_demo(void *, void *, char **);

extern void webfiles_setup(void);
extern const unsigned char index_246iws[];
extern const unsigned char iot_295banner_246png[];
extern struct vfs_file webfiles_array[7];
extern struct vfs_file *vfsfiles;

#endif /* _web_H_ */

/********* End of file *************/
