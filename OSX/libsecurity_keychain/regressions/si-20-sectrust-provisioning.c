/*
 * Copyright (c) 2015-2017 Apple Inc. All Rights Reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <Security/SecCertificatePriv.h>
#include <Security/SecPolicyPriv.h>
#include <Security/CMSDecoder.h>

#include "keychain_regressions.h"
#include "utilities/SecCFRelease.h"
#include "utilities/SecCFWrappers.h"

#include "si-20-sectrust-provisioning.h"

/* SHA1 Fingerprint=45:27:70:FE:5F:E9:C2:DD:F9:77:17:29:F7:2B:71:DC:23:37:D1:1B */
/* subject:/CN=Mac OS X Provisioning Profile Signing/O=Apple Inc./C=US */
/* issuer :/C=US/O=Apple Inc./OU=Apple Worldwide Developer Relations/CN=Apple Worldwide Developer Relations Certification Authority */

static unsigned char c0[1334]={
	0x30,0x82,0x05,0x32,0x30,0x82,0x04,0x1A,0xA0,0x03,0x02,0x01,0x02,0x02,0x08,0x1A,
	0xA6,0x77,0xFE,0x20,0xB7,0x68,0x2E,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,
	0x0D,0x01,0x01,0x05,0x05,0x00,0x30,0x81,0x96,0x31,0x0B,0x30,0x09,0x06,0x03,0x55,
	0x04,0x06,0x13,0x02,0x55,0x53,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x0A,0x0C,
	0x0A,0x41,0x70,0x70,0x6C,0x65,0x20,0x49,0x6E,0x63,0x2E,0x31,0x2C,0x30,0x2A,0x06,
	0x03,0x55,0x04,0x0B,0x0C,0x23,0x41,0x70,0x70,0x6C,0x65,0x20,0x57,0x6F,0x72,0x6C,
	0x64,0x77,0x69,0x64,0x65,0x20,0x44,0x65,0x76,0x65,0x6C,0x6F,0x70,0x65,0x72,0x20,
	0x52,0x65,0x6C,0x61,0x74,0x69,0x6F,0x6E,0x73,0x31,0x44,0x30,0x42,0x06,0x03,0x55,
	0x04,0x03,0x0C,0x3B,0x41,0x70,0x70,0x6C,0x65,0x20,0x57,0x6F,0x72,0x6C,0x64,0x77,
	0x69,0x64,0x65,0x20,0x44,0x65,0x76,0x65,0x6C,0x6F,0x70,0x65,0x72,0x20,0x52,0x65,
	0x6C,0x61,0x74,0x69,0x6F,0x6E,0x73,0x20,0x43,0x65,0x72,0x74,0x69,0x66,0x69,0x63,
	0x61,0x74,0x69,0x6F,0x6E,0x20,0x41,0x75,0x74,0x68,0x6F,0x72,0x69,0x74,0x79,0x30,
	0x1E,0x17,0x0D,0x31,0x31,0x30,0x34,0x30,0x38,0x32,0x32,0x31,0x32,0x32,0x35,0x5A,
	0x17,0x0D,0x31,0x36,0x30,0x32,0x30,0x36,0x32,0x32,0x31,0x32,0x32,0x35,0x5A,0x30,
	0x52,0x31,0x2E,0x30,0x2C,0x06,0x03,0x55,0x04,0x03,0x0C,0x25,0x4D,0x61,0x63,0x20,
	0x4F,0x53,0x20,0x58,0x20,0x50,0x72,0x6F,0x76,0x69,0x73,0x69,0x6F,0x6E,0x69,0x6E,
	0x67,0x20,0x50,0x72,0x6F,0x66,0x69,0x6C,0x65,0x20,0x53,0x69,0x67,0x6E,0x69,0x6E,
	0x67,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x0A,0x0C,0x0A,0x41,0x70,0x70,0x6C,
	0x65,0x20,0x49,0x6E,0x63,0x2E,0x31,0x0B,0x30,0x09,0x06,0x03,0x55,0x04,0x06,0x13,
	0x02,0x55,0x53,0x30,0x82,0x01,0x22,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,
	0x0D,0x01,0x01,0x01,0x05,0x00,0x03,0x82,0x01,0x0F,0x00,0x30,0x82,0x01,0x0A,0x02,
	0x82,0x01,0x01,0x00,0xA6,0x4C,0x9D,0xD8,0xC4,0xF8,0x64,0x71,0xBB,0x53,0xAE,0xD6,
	0x76,0x93,0x70,0x22,0xA0,0xD1,0xB9,0x18,0x85,0x90,0x4A,0x50,0xB9,0x5A,0x68,0x59,
	0xCA,0x9C,0x71,0x40,0xD3,0x21,0xCA,0x0E,0x99,0xD5,0x44,0x1C,0xD8,0xE3,0x2B,0x77,
	0x21,0x6B,0x0D,0x92,0x19,0xEA,0x7C,0xE5,0x05,0xB9,0x1E,0x95,0xD8,0xAD,0xB4,0x1F,
	0xE6,0xAE,0xBB,0xF3,0x0B,0x29,0x44,0x40,0x4D,0x10,0xA5,0x37,0x48,0x26,0x56,0x37,
	0xD8,0x50,0xC1,0x5F,0x87,0x4E,0xE2,0x4D,0xD6,0xD6,0x7F,0x0D,0x39,0xA7,0xBB,0xB0,
	0x06,0x90,0x39,0xAB,0xB2,0x96,0x2C,0x4A,0x07,0x2F,0x17,0xEA,0x3C,0x00,0xBF,0x8F,
	0xEB,0xD3,0xE7,0x5E,0x5F,0x05,0x59,0x42,0xC2,0x24,0x59,0x29,0x81,0xEF,0x4E,0xB1,
	0x1F,0x82,0xB5,0x57,0x66,0xC7,0x37,0xBD,0xA9,0xED,0x21,0xB9,0xCB,0xC4,0x27,0xC2,
	0x58,0x37,0x8D,0x8A,0xF4,0x4B,0xBD,0x3F,0xFC,0x41,0x08,0x67,0x42,0x4B,0x3A,0xCA,
	0x72,0xFA,0x38,0xA8,0x77,0xF3,0xD3,0x6C,0x46,0xF7,0x73,0x5D,0x83,0xBA,0xD3,0x86,
	0x6A,0xEB,0x4E,0x61,0x6D,0x8A,0xCE,0x90,0xEC,0x0E,0xE7,0x39,0x69,0xDD,0x49,0xA0,
	0x7E,0xB3,0xD9,0x7E,0x2B,0x4C,0x51,0x5A,0x1D,0xDA,0x54,0x16,0xE5,0xA6,0xF1,0xB0,
	0x04,0x80,0xAC,0x87,0x77,0x11,0x2C,0x6D,0x5B,0x78,0x38,0x9C,0x71,0x4E,0xF6,0x0E,
	0xCD,0x78,0x2C,0x03,0x42,0xAC,0x4C,0x3B,0x3E,0xE2,0xBE,0xD2,0xBC,0x70,0x5B,0x00,
	0x6A,0xAA,0xA3,0x66,0xAB,0xBA,0x44,0x33,0x96,0x76,0xEC,0x37,0xA3,0x33,0xC8,0x2C,
	0xED,0x6E,0x37,0xB5,0x02,0x03,0x01,0x00,0x01,0xA3,0x82,0x01,0xC5,0x30,0x82,0x01,
	0xC1,0x30,0x3D,0x06,0x08,0x2B,0x06,0x01,0x05,0x05,0x07,0x01,0x01,0x04,0x31,0x30,
	0x2F,0x30,0x2D,0x06,0x08,0x2B,0x06,0x01,0x05,0x05,0x07,0x30,0x01,0x86,0x21,0x68,
	0x74,0x74,0x70,0x3A,0x2F,0x2F,0x6F,0x63,0x73,0x70,0x2E,0x61,0x70,0x70,0x6C,0x65,
	0x2E,0x63,0x6F,0x6D,0x2F,0x6F,0x63,0x73,0x70,0x2D,0x77,0x77,0x64,0x72,0x30,0x33,
	0x30,0x1D,0x06,0x03,0x55,0x1D,0x0E,0x04,0x16,0x04,0x14,0x16,0x40,0x54,0xF8,0x17,
	0x37,0x2C,0x46,0xE4,0x5F,0x75,0x8C,0xF9,0x55,0x70,0x0E,0xEF,0x1E,0xE7,0xF1,0x30,
	0x0C,0x06,0x03,0x55,0x1D,0x13,0x01,0x01,0xFF,0x04,0x02,0x30,0x00,0x30,0x1F,0x06,
	0x03,0x55,0x1D,0x23,0x04,0x18,0x30,0x16,0x80,0x14,0x88,0x27,0x17,0x09,0xA9,0xB6,
	0x18,0x60,0x8B,0xEC,0xEB,0xBA,0xF6,0x47,0x59,0xC5,0x52,0x54,0xA3,0xB7,0x30,0x82,
	0x01,0x0F,0x06,0x03,0x55,0x1D,0x20,0x04,0x82,0x01,0x06,0x30,0x82,0x01,0x02,0x30,
	0x81,0xFF,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x63,0x64,0x05,0x01,0x30,0x81,0xF1,
	0x30,0x81,0xC3,0x06,0x08,0x2B,0x06,0x01,0x05,0x05,0x07,0x02,0x02,0x30,0x81,0xB6,
	0x0C,0x81,0xB3,0x52,0x65,0x6C,0x69,0x61,0x6E,0x63,0x65,0x20,0x6F,0x6E,0x20,0x74,
	0x68,0x69,0x73,0x20,0x63,0x65,0x72,0x74,0x69,0x66,0x69,0x63,0x61,0x74,0x65,0x20,
	0x62,0x79,0x20,0x61,0x6E,0x79,0x20,0x70,0x61,0x72,0x74,0x79,0x20,0x61,0x73,0x73,
	0x75,0x6D,0x65,0x73,0x20,0x61,0x63,0x63,0x65,0x70,0x74,0x61,0x6E,0x63,0x65,0x20,
	0x6F,0x66,0x20,0x74,0x68,0x65,0x20,0x74,0x68,0x65,0x6E,0x20,0x61,0x70,0x70,0x6C,
	0x69,0x63,0x61,0x62,0x6C,0x65,0x20,0x73,0x74,0x61,0x6E,0x64,0x61,0x72,0x64,0x20,
	0x74,0x65,0x72,0x6D,0x73,0x20,0x61,0x6E,0x64,0x20,0x63,0x6F,0x6E,0x64,0x69,0x74,
	0x69,0x6F,0x6E,0x73,0x20,0x6F,0x66,0x20,0x75,0x73,0x65,0x2C,0x20,0x63,0x65,0x72,
	0x74,0x69,0x66,0x69,0x63,0x61,0x74,0x65,0x20,0x70,0x6F,0x6C,0x69,0x63,0x79,0x20,
	0x61,0x6E,0x64,0x20,0x63,0x65,0x72,0x74,0x69,0x66,0x69,0x63,0x61,0x74,0x69,0x6F,
	0x6E,0x20,0x70,0x72,0x61,0x63,0x74,0x69,0x63,0x65,0x20,0x73,0x74,0x61,0x74,0x65,
	0x6D,0x65,0x6E,0x74,0x73,0x2E,0x30,0x29,0x06,0x08,0x2B,0x06,0x01,0x05,0x05,0x07,
	0x02,0x01,0x16,0x1D,0x68,0x74,0x74,0x70,0x3A,0x2F,0x2F,0x77,0x77,0x77,0x2E,0x61,
	0x70,0x70,0x6C,0x65,0x2E,0x63,0x6F,0x6D,0x2F,0x61,0x70,0x70,0x6C,0x65,0x63,0x61,
	0x2F,0x30,0x0E,0x06,0x03,0x55,0x1D,0x0F,0x01,0x01,0xFF,0x04,0x04,0x03,0x02,0x07,
	0x80,0x30,0x0F,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x63,0x64,0x04,0x0B,0x04,0x02,
	0x05,0x00,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x05,0x05,
	0x00,0x03,0x82,0x01,0x01,0x00,0x41,0x76,0x9C,0x4B,0x42,0x36,0x40,0x75,0xF4,0x68,
	0x51,0x76,0x3F,0x77,0xBE,0x7A,0x66,0x04,0x08,0xA3,0xA8,0xDA,0xD9,0x60,0x30,0xA4,
	0x3A,0x5E,0x2D,0xF8,0x10,0x06,0x96,0x9B,0xD7,0x10,0x14,0x8C,0x95,0x71,0x26,0xC9,
	0x01,0x83,0xB2,0x27,0xA9,0x74,0xA6,0xDB,0x5F,0xB5,0xA9,0x39,0x67,0x54,0x6F,0x08,
	0x43,0x9E,0x4C,0x46,0xA1,0xA8,0x22,0xBF,0x58,0x49,0xB3,0x1C,0xC6,0xF0,0xAA,0xF3,
	0x67,0x89,0x5E,0xA6,0x79,0x3E,0x25,0xB7,0xE9,0x00,0x2B,0xD9,0xEE,0xED,0x6F,0xF8,
	0x48,0x3B,0x97,0x4A,0x54,0x27,0x38,0x54,0xC2,0x4A,0xBF,0x35,0x36,0x6F,0x92,0x02,
	0x65,0x35,0x2A,0x63,0x3D,0x13,0xA8,0x06,0x5D,0x51,0x7E,0x61,0x10,0xF7,0xF5,0x56,
	0x22,0xFB,0x28,0xA3,0x8F,0xAE,0xE6,0x28,0x4B,0xEA,0x7C,0x22,0x70,0x49,0x61,0x76,
	0x51,0xFC,0x9C,0x64,0x9A,0x88,0x8B,0x6C,0x4B,0x1A,0x22,0xF0,0xE8,0xB3,0xD2,0xF6,
	0x2C,0x31,0xD7,0xC4,0x30,0xBF,0x82,0xDD,0x22,0x93,0x14,0x20,0x73,0xAA,0xB8,0xD1,
	0x17,0x1E,0x3F,0x36,0x4F,0x94,0x9C,0xF3,0xF9,0x3B,0x9A,0xDB,0x69,0x1A,0x91,0x6D,
	0x56,0x60,0x2A,0x86,0xBD,0x25,0x68,0x24,0xCC,0x11,0x09,0x17,0x88,0xCE,0x27,0xA1,
	0xE1,0x6B,0x30,0xB2,0x8C,0xB9,0xA8,0xA0,0xB7,0xF0,0xAA,0x46,0xA4,0x95,0x21,0x13,
	0xC8,0x4F,0xE9,0xA9,0xB1,0x35,0x12,0x57,0xE6,0x04,0xD0,0x3D,0xFF,0x12,0xDC,0xEB,
	0xDA,0xC5,0xD9,0x85,0xD6,0xBC,0x96,0xCF,0x90,0x02,0xC7,0x66,0xC7,0xF7,0x78,0x77,
	0xDA,0xA6,0xD7,0x89,0x1B,0xAF,
};

/* SHA1 Fingerprint=09:50:B6:CD:3D:2F:37:EA:24:6A:1A:AA:20:DF:AA:DB:D6:FE:1F:75 */
/* subject:/C=US/O=Apple Inc./OU=Apple Worldwide Developer Relations/CN=Apple Worldwide Developer Relations Certification Authority */
/* issuer :/C=US/O=Apple Inc./OU=Apple Certification Authority/CN=Apple Root CA */

static unsigned char c1[1063]={
	0x30,0x82,0x04,0x23,0x30,0x82,0x03,0x0B,0xA0,0x03,0x02,0x01,0x02,0x02,0x01,0x19,
	0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x05,0x05,0x00,0x30,
	0x62,0x31,0x0B,0x30,0x09,0x06,0x03,0x55,0x04,0x06,0x13,0x02,0x55,0x53,0x31,0x13,
	0x30,0x11,0x06,0x03,0x55,0x04,0x0A,0x13,0x0A,0x41,0x70,0x70,0x6C,0x65,0x20,0x49,
	0x6E,0x63,0x2E,0x31,0x26,0x30,0x24,0x06,0x03,0x55,0x04,0x0B,0x13,0x1D,0x41,0x70,
	0x70,0x6C,0x65,0x20,0x43,0x65,0x72,0x74,0x69,0x66,0x69,0x63,0x61,0x74,0x69,0x6F,
	0x6E,0x20,0x41,0x75,0x74,0x68,0x6F,0x72,0x69,0x74,0x79,0x31,0x16,0x30,0x14,0x06,
	0x03,0x55,0x04,0x03,0x13,0x0D,0x41,0x70,0x70,0x6C,0x65,0x20,0x52,0x6F,0x6F,0x74,
	0x20,0x43,0x41,0x30,0x1E,0x17,0x0D,0x30,0x38,0x30,0x32,0x31,0x34,0x31,0x38,0x35,
	0x36,0x33,0x35,0x5A,0x17,0x0D,0x31,0x36,0x30,0x32,0x31,0x34,0x31,0x38,0x35,0x36,
	0x33,0x35,0x5A,0x30,0x81,0x96,0x31,0x0B,0x30,0x09,0x06,0x03,0x55,0x04,0x06,0x13,
	0x02,0x55,0x53,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x0A,0x0C,0x0A,0x41,0x70,
	0x70,0x6C,0x65,0x20,0x49,0x6E,0x63,0x2E,0x31,0x2C,0x30,0x2A,0x06,0x03,0x55,0x04,
	0x0B,0x0C,0x23,0x41,0x70,0x70,0x6C,0x65,0x20,0x57,0x6F,0x72,0x6C,0x64,0x77,0x69,
	0x64,0x65,0x20,0x44,0x65,0x76,0x65,0x6C,0x6F,0x70,0x65,0x72,0x20,0x52,0x65,0x6C,
	0x61,0x74,0x69,0x6F,0x6E,0x73,0x31,0x44,0x30,0x42,0x06,0x03,0x55,0x04,0x03,0x0C,
	0x3B,0x41,0x70,0x70,0x6C,0x65,0x20,0x57,0x6F,0x72,0x6C,0x64,0x77,0x69,0x64,0x65,
	0x20,0x44,0x65,0x76,0x65,0x6C,0x6F,0x70,0x65,0x72,0x20,0x52,0x65,0x6C,0x61,0x74,
	0x69,0x6F,0x6E,0x73,0x20,0x43,0x65,0x72,0x74,0x69,0x66,0x69,0x63,0x61,0x74,0x69,
	0x6F,0x6E,0x20,0x41,0x75,0x74,0x68,0x6F,0x72,0x69,0x74,0x79,0x30,0x82,0x01,0x22,
	0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x01,0x05,0x00,0x03,
	0x82,0x01,0x0F,0x00,0x30,0x82,0x01,0x0A,0x02,0x82,0x01,0x01,0x00,0xCA,0x38,0x54,
	0xA6,0xCB,0x56,0xAA,0xC8,0x24,0x39,0x48,0xE9,0x8C,0xEE,0xEC,0x5F,0xB8,0x7F,0x26,
	0x91,0xBC,0x34,0x53,0x7A,0xCE,0x7C,0x63,0x80,0x61,0x77,0x64,0x5E,0xA5,0x07,0x23,
	0xB6,0x39,0xFE,0x50,0x2D,0x15,0x56,0x58,0x70,0x2D,0x7E,0xC4,0x6E,0xC1,0x4A,0x85,
	0x3E,0x2F,0xF0,0xDE,0x84,0x1A,0xA1,0x57,0xC9,0xAF,0x7B,0x18,0xFF,0x6A,0xFA,0x15,
	0x12,0x49,0x15,0x08,0x19,0xAC,0xAA,0xDB,0x2A,0x32,0xED,0x96,0x63,0x68,0x52,0x15,
	0x3D,0x8C,0x8A,0xEC,0xBF,0x6B,0x18,0x95,0xE0,0x03,0xAC,0x01,0x7D,0x97,0x05,0x67,
	0xCE,0x0E,0x85,0x95,0x37,0x6A,0xED,0x09,0xB6,0xAE,0x67,0xCD,0x51,0x64,0x9F,0xC6,
	0x5C,0xD1,0xBC,0x57,0x6E,0x67,0x35,0x80,0x76,0x36,0xA4,0x87,0x81,0x6E,0x38,0x8F,
	0xD8,0x2B,0x15,0x4E,0x7B,0x25,0xD8,0x5A,0xBF,0x4E,0x83,0xC1,0x8D,0xD2,0x93,0xD5,
	0x1A,0x71,0xB5,0x60,0x9C,0x9D,0x33,0x4E,0x55,0xF9,0x12,0x58,0x0C,0x86,0xB8,0x16,
	0x0D,0xC1,0xE5,0x77,0x45,0x8D,0x50,0x48,0xBA,0x2B,0x2D,0xE4,0x94,0x85,0xE1,0xE8,
	0xC4,0x9D,0xC6,0x68,0xA5,0xB0,0xA3,0xFC,0x67,0x7E,0x70,0xBA,0x02,0x59,0x4B,0x77,
	0x42,0x91,0x39,0xB9,0xF5,0xCD,0xE1,0x4C,0xEF,0xC0,0x3B,0x48,0x8C,0xA6,0xE5,0x21,
	0x5D,0xFD,0x6A,0x6A,0xBB,0xA7,0x16,0x35,0x60,0xD2,0xE6,0xAD,0xF3,0x46,0x29,0xC9,
	0xE8,0xC3,0x8B,0xE9,0x79,0xC0,0x6A,0x61,0x67,0x15,0xB2,0xF0,0xFD,0xE5,0x68,0xBC,
	0x62,0x5F,0x6E,0xCF,0x99,0xDD,0xEF,0x1B,0x63,0xFE,0x92,0x65,0xAB,0x02,0x03,0x01,
	0x00,0x01,0xA3,0x81,0xAE,0x30,0x81,0xAB,0x30,0x0E,0x06,0x03,0x55,0x1D,0x0F,0x01,
	0x01,0xFF,0x04,0x04,0x03,0x02,0x01,0x86,0x30,0x0F,0x06,0x03,0x55,0x1D,0x13,0x01,
	0x01,0xFF,0x04,0x05,0x30,0x03,0x01,0x01,0xFF,0x30,0x1D,0x06,0x03,0x55,0x1D,0x0E,
	0x04,0x16,0x04,0x14,0x88,0x27,0x17,0x09,0xA9,0xB6,0x18,0x60,0x8B,0xEC,0xEB,0xBA,
	0xF6,0x47,0x59,0xC5,0x52,0x54,0xA3,0xB7,0x30,0x1F,0x06,0x03,0x55,0x1D,0x23,0x04,
	0x18,0x30,0x16,0x80,0x14,0x2B,0xD0,0x69,0x47,0x94,0x76,0x09,0xFE,0xF4,0x6B,0x8D,
	0x2E,0x40,0xA6,0xF7,0x47,0x4D,0x7F,0x08,0x5E,0x30,0x36,0x06,0x03,0x55,0x1D,0x1F,
	0x04,0x2F,0x30,0x2D,0x30,0x2B,0xA0,0x29,0xA0,0x27,0x86,0x25,0x68,0x74,0x74,0x70,
	0x3A,0x2F,0x2F,0x77,0x77,0x77,0x2E,0x61,0x70,0x70,0x6C,0x65,0x2E,0x63,0x6F,0x6D,
	0x2F,0x61,0x70,0x70,0x6C,0x65,0x63,0x61,0x2F,0x72,0x6F,0x6F,0x74,0x2E,0x63,0x72,
	0x6C,0x30,0x10,0x06,0x0A,0x2A,0x86,0x48,0x86,0xF7,0x63,0x64,0x06,0x02,0x01,0x04,
	0x02,0x05,0x00,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x05,
	0x05,0x00,0x03,0x82,0x01,0x01,0x00,0xDA,0x32,0x00,0x96,0xC5,0x54,0x94,0xD3,0x3B,
	0x82,0x37,0x66,0x7D,0x2E,0x68,0xD5,0xC3,0xC6,0xB8,0xCB,0x26,0x8C,0x48,0x90,0xCF,
	0x13,0x24,0x6A,0x46,0x8E,0x63,0xD4,0xF0,0xD0,0x13,0x06,0xDD,0xD8,0xC4,0xC1,0x37,
	0x15,0xF2,0x33,0x13,0x39,0x26,0x2D,0xCE,0x2E,0x55,0x40,0xE3,0x0B,0x03,0xAF,0xFA,
	0x12,0xC2,0xE7,0x0D,0x21,0xB8,0xD5,0x80,0xCF,0xAC,0x28,0x2F,0xCE,0x2D,0xB3,0x4E,
	0xAF,0x86,0x19,0x04,0xC6,0xE9,0x50,0xDD,0x4C,0x29,0x47,0x10,0x23,0xFC,0x6C,0xBB,
	0x1B,0x98,0x6B,0x48,0x89,0xE1,0x5B,0x9D,0xDE,0x46,0xDB,0x35,0x85,0x35,0xEF,0x3E,
	0xD0,0xE2,0x58,0x4B,0x38,0xF4,0xED,0x75,0x5A,0x1F,0x5C,0x70,0x1D,0x56,0x39,0x12,
	0xE5,0xE1,0x0D,0x11,0xE4,0x89,0x25,0x06,0xBD,0xD5,0xB4,0x15,0x8E,0x5E,0xD0,0x59,
	0x97,0x90,0xE9,0x4B,0x81,0xE2,0xDF,0x18,0xAF,0x44,0x74,0x1E,0x19,0xA0,0x3A,0x47,
	0xCC,0x91,0x1D,0x3A,0xEB,0x23,0x5A,0xFE,0xA5,0x2D,0x97,0xF7,0x7B,0xBB,0xD6,0x87,
	0x46,0x42,0x85,0xEB,0x52,0x3D,0x26,0xB2,0x63,0xA8,0xB4,0xB1,0xCA,0x8F,0xF4,0xCC,
	0xE2,0xB3,0xC8,0x47,0xE0,0xBF,0x9A,0x59,0x83,0xFA,0xDA,0x98,0x53,0x2A,0x82,0xF5,
	0x7C,0x65,0x2E,0x95,0xD9,0x33,0x5D,0xF5,0xED,0x65,0xCC,0x31,0x37,0xC5,0x5A,0x04,
	0xE8,0x6B,0xE1,0xE7,0x88,0x03,0x4A,0x75,0x9E,0x9B,0x28,0xCB,0x4A,0x40,0x88,0x65,
	0x43,0x75,0xDD,0xCB,0x3A,0x25,0x23,0xC5,0x9E,0x57,0xF8,0x2E,0xCE,0xD2,0xA9,0x92,
	0x5E,0x73,0x2E,0x2F,0x25,0x75,0x15,
};


static CFStringRef copyIssuerCN(SecCertificateRef certificate)
{
    if (!certificate ||
        CFGetTypeID(certificate) !=SecCertificateGetTypeID()) {
        return NULL;
    }

    CFStringRef         issuerCN        = NULL; // do not release
    CFDictionaryRef     issuerDict      = NULL; // do not release
    CFArrayRef          issuerArray     = NULL; // do not release
    CFDictionaryRef     issuerInfo      = NULL; // do not release
    CFErrorRef          error           = NULL; // do not release

    CFMutableArrayRef   certificateKeys = NULL; // must release
    CFDictionaryRef     certificateDict = NULL; // must release

    certificateKeys = CFArrayCreateMutable(kCFAllocatorDefault,
                                           1,
                                           &kCFTypeArrayCallBacks);
    if (!certificateKeys) {
        goto finish;
    }

    CFArrayAppendValue(certificateKeys, kSecOIDX509V1IssuerName);

    certificateDict = SecCertificateCopyValues(certificate,
                                               certificateKeys,
                                               &error);

    if (error != errSecSuccess ||
        !certificateDict ||
        CFGetTypeID(certificateDict) != CFDictionaryGetTypeID()) {
        goto finish;
    }

    issuerDict = (CFDictionaryRef) CFDictionaryGetValue(certificateDict,
                                                        kSecOIDX509V1IssuerName);
    if (!issuerDict ||
        CFGetTypeID(issuerDict) != CFDictionaryGetTypeID()) {
        goto finish;
    }

    issuerArray = (CFArrayRef) CFDictionaryGetValue(issuerDict,
                                                    kSecPropertyKeyValue);
    if (!issuerArray ||
        CFGetTypeID(issuerArray) != CFArrayGetTypeID()) {
        goto finish;
    }

    for (int index=0; index<CFArrayGetCount(issuerArray); index++) {
        issuerInfo = (CFDictionaryRef) CFArrayGetValueAtIndex(issuerArray,
                                                              index);
        if (!issuerInfo ||
            CFGetTypeID(issuerInfo) != CFDictionaryGetTypeID()) {
            continue;
        }

        CFStringRef label = NULL; // do not release
        label = CFDictionaryGetValue(issuerInfo,
                                     kSecPropertyKeyLabel);
        if (kCFCompareEqualTo == CFStringCompare(label,
                                                 CFSTR("2.5.4.3"),
                                                 0)) {
            issuerCN = CFDictionaryGetValue(issuerInfo,
                                            kSecPropertyKeyValue);
            if (issuerCN &&
                CFGetTypeID(issuerCN) == CFStringGetTypeID()) {
                CFRetain(issuerCN);
                goto finish;
            }
            else {
                issuerCN = NULL;
            }
        }
    }

finish:
    if (certificateKeys) { CFRelease(certificateKeys); }
    if (certificateDict) { CFRelease(certificateDict); }
    return issuerCN;
}

static void CertificateValuesTests(SecCertificateRef certificate)
{
    CFDictionaryRef     subjectDict     = NULL; // do not release
    CFArrayRef          subjectArray    = NULL; // do not release
    CFStringRef         orgNameString   = NULL; // do not release
    CFErrorRef          error           = NULL; // do not release

    CFMutableArrayRef   certificateKeys = NULL; // must release
    CFDictionaryRef     certificateDict = NULL; // must release

    bool hasAppleOrgName = false;
    bool hasWWDRIssuerCN = false;

    certificateKeys = CFArrayCreateMutable(kCFAllocatorDefault,
                                           1,
                                           &kCFTypeArrayCallBacks);
    if (!certificateKeys) {
        goto finish;
    }
    CFArrayAppendValue(certificateKeys, kSecOIDX509V1SubjectName);

    certificateDict = SecCertificateCopyValues(certificate, certificateKeys, &error);
    if (error != errSecSuccess || !certificateDict ||
        CFGetTypeID(certificateDict) != CFDictionaryGetTypeID()) {
        goto finish;
    }

    subjectDict = (CFDictionaryRef) CFDictionaryGetValue(certificateDict,
                                                         kSecOIDX509V1SubjectName);
    if (!subjectDict || CFGetTypeID(subjectDict) != CFDictionaryGetTypeID()) {
        goto finish;
    }

    subjectArray = (CFArrayRef) CFDictionaryGetValue(subjectDict,
                                                     kSecPropertyKeyValue);
    if (!subjectArray ||
        CFGetTypeID(subjectArray) != CFArrayGetTypeID()) {
        goto finish;
    }

    // look for Organization Name field ("2.5.4.10") in subject
    for (int index=0; index<CFArrayGetCount(subjectArray); index++) {
        CFDictionaryRef subjectInfo = (CFDictionaryRef) CFArrayGetValueAtIndex(subjectArray, index);
        if (!subjectInfo || CFGetTypeID(subjectInfo) != CFDictionaryGetTypeID()) {
            continue;
        }
        CFStringRef label = CFDictionaryGetValue(subjectInfo, kSecPropertyKeyLabel);
        if (kCFCompareEqualTo == CFStringCompare(label, CFSTR("2.5.4.10"), 0)) {
            orgNameString = CFDictionaryGetValue(subjectInfo, kSecPropertyKeyValue);
            if (orgNameString && CFGetTypeID(orgNameString) == CFStringGetTypeID()) {
                if (kCFCompareEqualTo == CFStringCompare(orgNameString, CFSTR("Apple Inc."), 0)) {
                    hasAppleOrgName = true;
                }
            }
        }
    }

    // look for issuer common name
    CFStringRef commonName = copyIssuerCN(certificate);
    if (commonName) {
        if (kCFCompareEqualTo == CFStringCompare(commonName,
            CFSTR("Apple Worldwide Developer Relations Certification Authority"), 0))  {
            hasWWDRIssuerCN = true;
        }
        CFRelease(commonName);
    }

finish:
    if (certificateKeys) { CFRelease(certificateKeys); }
    if (certificateDict) { CFRelease(certificateDict); }

    /* and now, the actual tests... */
    is(hasAppleOrgName, true, "O=Apple Inc.");
    is(hasWWDRIssuerCN, true, "CN=Apple Worldwide Developer Relations Certification Authority");
}

static void tests(void)
{
    SecTrustRef trust = NULL;
    SecPolicyRef policy = NULL;
    SecCertificateRef cert0, cert1;
    SecTrustResultType trustResult;

    isnt(cert0 = SecCertificateCreateWithBytes(NULL, c0, sizeof(c0)), NULL, "create cert0");
    isnt(cert1 = SecCertificateCreateWithBytes(NULL, c1, sizeof(c1)), NULL, "create cert1");

    const void *v_certs[] = { cert0, cert1 };
    CFArrayRef certs = CFArrayCreate(NULL, v_certs, sizeof(v_certs)/sizeof(*v_certs), &kCFTypeArrayCallBacks);

    /* Create Mac Provisioning Profile policy instance. */
    isnt(policy = SecPolicyCreateOSXProvisioningProfileSigning(), NULL, "create policy");

    /* Create trust reference. */
    ok_status(SecTrustCreateWithCertificates(certs, policy, &trust), "create trust");

    /* Set explicit verify date: Sep 1 2015. */
    CFDateRef date = NULL;
    isnt(date = CFDateCreate(NULL, 462823871.0), NULL, "Create verify date");
    ok_status(SecTrustSetVerifyDate(trust, date), "set date");

    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate trust");
    is_status(trustResult, kSecTrustResultUnspecified, "trustResult is kSecTrustResultUnspecified");
    is(SecTrustGetCertificateCount(trust), 3, "cert count is 3");

    /* Verify signed content with this policy. */
    CMSDecoderRef decoder = NULL;
    CMSSignerStatus signerStatus = kCMSSignerInvalidIndex;
    OSStatus verifyResult = 0;
    ok_status(CMSDecoderCreate(&decoder),
              "create decoder");
    ok_status(CMSDecoderUpdateMessage(decoder, _TestProvisioningProfile, sizeof(_TestProvisioningProfile)),
              "update message");
    ok_status(CMSDecoderFinalizeMessage(decoder),
              "finalize message");
    ok_status(CMSDecoderCopySignerStatus(decoder, 0, policy, true, &signerStatus, NULL, &verifyResult),
              "copy signer status");
    is(signerStatus, kCMSSignerValid, "signer status valid");
    is(verifyResult, errSecSuccess, "verify result valid");

    /* Add some basic subject/issuer field value tests */
    CertificateValuesTests(cert0);

    CFReleaseSafe(decoder);
    CFReleaseSafe(date);
    CFReleaseSafe(trust);
    CFReleaseSafe(policy);
    CFReleaseSafe(certs);
    CFReleaseSafe(cert1);
    CFReleaseSafe(cert0);
}

int si_20_sectrust_provisioning(int argc, char *const *argv);

int si_20_sectrust_provisioning(int argc, char *const *argv)
{
    plan_tests(17);

    tests();

    return 0;
}
