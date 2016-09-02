/*
 * Copyright (c) 2000-2001 Apple Computer, Inc. All Rights Reserved.
 * 
 * The contents of this file constitute Original Code as defined in and are
 * subject to the Apple Public Source License Version 1.2 (the 'License').
 * You may not use this file except in compliance with the License. Please obtain
 * a copy of the License at http://www.apple.com/publicsource and read it before
 * using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS
 * OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. Please see the License for the
 * specific language governing rights and limitations under the License.
 */


/*
 	File:		HMACSHA1.c
 	Contains:	Apple Data Security Services HMACSHA1 function definition.
 	Copyright:	(C) 1999 by Apple Computer, Inc., all rights reserved
 	Written by:	Michael Brouwer <mb@apple.com>
*/
#include "HMACSHA1.h"
#include <CommonCrypto/CommonHMAC.h>
#include "cspdebugging.h"

void
hmacsha1 (const void *keyPtr, uint32 keyLen,
		  const void *textPtr, uint32 textLen,
		  void *resultPtr)
{
	CCHmac(kCCHmacAlgSHA1, keyPtr, keyLen,
		textPtr, textLen, resultPtr);
}

