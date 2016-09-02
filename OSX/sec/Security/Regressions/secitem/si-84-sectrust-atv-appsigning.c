/*
 *  si-84-sectrust-atv-appsigning.c
 *  Security
 *
 *  Copyright (c) 2015 Apple Inc. All Rights Reserved.
 *
 */

#include <CoreFoundation/CoreFoundation.h>
#include <Security/SecCertificate.h>
#include <Security/SecCertificatePriv.h>
#include <Security/SecCertificateInternal.h>
#include <Security/SecItem.h>
#include <Security/SecItemPriv.h>
#include <Security/SecIdentityPriv.h>
#include <Security/SecIdentity.h>
#include <Security/SecPolicy.h>
#include <Security/SecPolicyPriv.h>
#include <Security/SecPolicyInternal.h>
#include <Security/SecCMS.h>
#include <utilities/SecCFWrappers.h>
#include <stdlib.h>
#include <unistd.h>

#include "Security_regressions.h"

/* subject:/C=US/O=Apple Inc./OU=Apple Worldwide Developer Relations/CN=Apple Worldwide Developer Relations Certification Authority */
/* issuer :/C=US/O=Apple Inc./OU=Apple Certification Authority/CN=Apple Root CA */
static const UInt8 kWwdrAnchorCertificate[] = {
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

// Issuer: C=US, O=Apple Inc., OU=Apple Worldwide Developer Relations, CN=Apple Worldwide Developer Relations Certification Authority
// Subject: CN=TEST Apple TVOS Application Signing TEST, O=Apple Inc., C=US
static const UInt8 kTestATVAppSigningCert[] = {
    0x30, 0x82, 0x05, 0x52, 0x30, 0x82, 0x04, 0x3a, 0xa0, 0x03, 0x02, 0x01,
    0x02, 0x02, 0x08, 0x29, 0x7b, 0x51, 0x36, 0x47, 0xa4, 0x6f, 0x23, 0x30,
    0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b,
    0x05, 0x00, 0x30, 0x81, 0x96, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55,
    0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03,
    0x55, 0x04, 0x0a, 0x0c, 0x0a, 0x41, 0x70, 0x70, 0x6c, 0x65, 0x20, 0x49,
    0x6e, 0x63, 0x2e, 0x31, 0x2c, 0x30, 0x2a, 0x06, 0x03, 0x55, 0x04, 0x0b,
    0x0c, 0x23, 0x41, 0x70, 0x70, 0x6c, 0x65, 0x20, 0x57, 0x6f, 0x72, 0x6c,
    0x64, 0x77, 0x69, 0x64, 0x65, 0x20, 0x44, 0x65, 0x76, 0x65, 0x6c, 0x6f,
    0x70, 0x65, 0x72, 0x20, 0x52, 0x65, 0x6c, 0x61, 0x74, 0x69, 0x6f, 0x6e,
    0x73, 0x31, 0x44, 0x30, 0x42, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x3b,
    0x41, 0x70, 0x70, 0x6c, 0x65, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x77,
    0x69, 0x64, 0x65, 0x20, 0x44, 0x65, 0x76, 0x65, 0x6c, 0x6f, 0x70, 0x65,
    0x72, 0x20, 0x52, 0x65, 0x6c, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x73, 0x20,
    0x43, 0x65, 0x72, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6f,
    0x6e, 0x20, 0x41, 0x75, 0x74, 0x68, 0x6f, 0x72, 0x69, 0x74, 0x79, 0x30,
    0x1e, 0x17, 0x0d, 0x31, 0x35, 0x30, 0x32, 0x30, 0x36, 0x32, 0x33, 0x35,
    0x33, 0x31, 0x36, 0x5a, 0x17, 0x0d, 0x32, 0x33, 0x30, 0x32, 0x30, 0x37,
    0x32, 0x31, 0x34, 0x38, 0x34, 0x37, 0x5a, 0x30, 0x55, 0x31, 0x31, 0x30,
    0x2f, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x28, 0x54, 0x45, 0x53, 0x54,
    0x20, 0x41, 0x70, 0x70, 0x6c, 0x65, 0x20, 0x54, 0x56, 0x4f, 0x53, 0x20,
    0x41, 0x70, 0x70, 0x6c, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20,
    0x53, 0x69, 0x67, 0x6e, 0x69, 0x6e, 0x67, 0x20, 0x54, 0x45, 0x53, 0x54,
    0x31, 0x13, 0x30, 0x11, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x0a, 0x41,
    0x70, 0x70, 0x6c, 0x65, 0x20, 0x49, 0x6e, 0x63, 0x2e, 0x31, 0x0b, 0x30,
    0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x30, 0x82,
    0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,
    0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00, 0x30, 0x82,
    0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xc9, 0x74, 0x71, 0x4a, 0x58,
    0x65, 0xdf, 0x19, 0x27, 0x08, 0x97, 0x9b, 0xf3, 0x12, 0x14, 0x8e, 0xa2,
    0xd0, 0xa2, 0x1e, 0x1d, 0x46, 0xae, 0xdf, 0xc4, 0xef, 0x57, 0xc0, 0x82,
    0x5f, 0xb9, 0xe5, 0x63, 0x53, 0x57, 0xad, 0xaa, 0x32, 0x84, 0x6f, 0xbe,
    0xdf, 0x65, 0x1f, 0x73, 0x0a, 0x85, 0x55, 0x3a, 0xb3, 0xcf, 0x43, 0x02,
    0x18, 0xe4, 0xad, 0x04, 0xa0, 0x83, 0x89, 0x3d, 0x6f, 0xfa, 0xdf, 0xb3,
    0x82, 0xa2, 0xb2, 0x6d, 0x46, 0x63, 0x4d, 0x88, 0x0a, 0xe7, 0x96, 0x68,
    0x3b, 0x6f, 0x96, 0xf8, 0xa9, 0x92, 0x18, 0x15, 0x0d, 0xf4, 0xe9, 0x44,
    0xf5, 0x62, 0xf1, 0x50, 0x4d, 0x86, 0x60, 0x5b, 0x89, 0x72, 0x3c, 0x53,
    0x8a, 0xda, 0x3a, 0x4f, 0x1d, 0x58, 0x1a, 0xc2, 0xaf, 0x46, 0x0c, 0x6d,
    0x53, 0x6d, 0xa3, 0x4d, 0x36, 0xa0, 0xfe, 0x54, 0xc6, 0xdd, 0x94, 0x01,
    0x43, 0xc1, 0xdf, 0x62, 0xd2, 0x2e, 0x76, 0x96, 0x10, 0x29, 0x30, 0x4f,
    0x51, 0x35, 0x5d, 0x5f, 0x10, 0x32, 0x0f, 0xec, 0xad, 0xd0, 0x0a, 0xc1,
    0xde, 0x7f, 0x7d, 0xcc, 0xa7, 0x4b, 0x67, 0x5e, 0x97, 0xbf, 0x45, 0x9f,
    0x0b, 0x68, 0x93, 0x0b, 0x42, 0x7b, 0x49, 0xf9, 0xda, 0x3d, 0xa3, 0x5e,
    0x22, 0x6b, 0x48, 0x2d, 0x86, 0x96, 0x25, 0xc1, 0x78, 0x11, 0xad, 0x7f,
    0x70, 0x43, 0x49, 0x05, 0x8d, 0x59, 0xe2, 0x80, 0x51, 0x79, 0x58, 0x5c,
    0xfb, 0x75, 0x6c, 0xa0, 0x7f, 0x62, 0xf5, 0x7d, 0xc1, 0xe7, 0xf8, 0x06,
    0x85, 0x9f, 0xb3, 0xaa, 0x90, 0x98, 0x53, 0x8d, 0x7b, 0x40, 0x04, 0x71,
    0xf4, 0xa4, 0xce, 0xa0, 0x20, 0x3d, 0x77, 0x32, 0xf5, 0x94, 0x20, 0x54,
    0xa2, 0xe2, 0x98, 0x8c, 0x38, 0x63, 0x94, 0xe5, 0x73, 0xa1, 0xcc, 0xcc,
    0xe4, 0x11, 0x34, 0xfb, 0xff, 0x41, 0x63, 0x2c, 0x39, 0xaf, 0x39, 0x02,
    0x03, 0x01, 0x00, 0x01, 0xa3, 0x82, 0x01, 0xe2, 0x30, 0x82, 0x01, 0xde,
    0x30, 0x47, 0x06, 0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x01, 0x01,
    0x04, 0x3b, 0x30, 0x39, 0x30, 0x37, 0x06, 0x08, 0x2b, 0x06, 0x01, 0x05,
    0x05, 0x07, 0x30, 0x01, 0x86, 0x2b, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f,
    0x2f, 0x6f, 0x63, 0x73, 0x70, 0x2e, 0x61, 0x70, 0x70, 0x6c, 0x65, 0x2e,
    0x63, 0x6f, 0x6d, 0x2f, 0x6f, 0x63, 0x73, 0x70, 0x30, 0x34, 0x2d, 0x61,
    0x70, 0x70, 0x6c, 0x65, 0x77, 0x77, 0x64, 0x72, 0x63, 0x61, 0x32, 0x30,
    0x34, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14,
    0x0a, 0x14, 0xfb, 0x9f, 0x6f, 0x4e, 0x79, 0xc0, 0xbb, 0xc8, 0xa5, 0x35,
    0xeb, 0x06, 0x6a, 0xe7, 0x45, 0x6a, 0x61, 0xad, 0x30, 0x0c, 0x06, 0x03,
    0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x02, 0x30, 0x00, 0x30, 0x1f,
    0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0x88,
    0x27, 0x17, 0x09, 0xa9, 0xb6, 0x18, 0x60, 0x8b, 0xec, 0xeb, 0xba, 0xf6,
    0x47, 0x59, 0xc5, 0x52, 0x54, 0xa3, 0xb7, 0x30, 0x82, 0x01, 0x1d, 0x06,
    0x03, 0x55, 0x1d, 0x20, 0x04, 0x82, 0x01, 0x14, 0x30, 0x82, 0x01, 0x10,
    0x30, 0x82, 0x01, 0x0c, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x63,
    0x64, 0x05, 0x01, 0x30, 0x81, 0xfe, 0x30, 0x81, 0xc3, 0x06, 0x08, 0x2b,
    0x06, 0x01, 0x05, 0x05, 0x07, 0x02, 0x02, 0x30, 0x81, 0xb6, 0x0c, 0x81,
    0xb3, 0x52, 0x65, 0x6c, 0x69, 0x61, 0x6e, 0x63, 0x65, 0x20, 0x6f, 0x6e,
    0x20, 0x74, 0x68, 0x69, 0x73, 0x20, 0x63, 0x65, 0x72, 0x74, 0x69, 0x66,
    0x69, 0x63, 0x61, 0x74, 0x65, 0x20, 0x62, 0x79, 0x20, 0x61, 0x6e, 0x79,
    0x20, 0x70, 0x61, 0x72, 0x74, 0x79, 0x20, 0x61, 0x73, 0x73, 0x75, 0x6d,
    0x65, 0x73, 0x20, 0x61, 0x63, 0x63, 0x65, 0x70, 0x74, 0x61, 0x6e, 0x63,
    0x65, 0x20, 0x6f, 0x66, 0x20, 0x74, 0x68, 0x65, 0x20, 0x74, 0x68, 0x65,
    0x6e, 0x20, 0x61, 0x70, 0x70, 0x6c, 0x69, 0x63, 0x61, 0x62, 0x6c, 0x65,
    0x20, 0x73, 0x74, 0x61, 0x6e, 0x64, 0x61, 0x72, 0x64, 0x20, 0x74, 0x65,
    0x72, 0x6d, 0x73, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x63, 0x6f, 0x6e, 0x64,
    0x69, 0x74, 0x69, 0x6f, 0x6e, 0x73, 0x20, 0x6f, 0x66, 0x20, 0x75, 0x73,
    0x65, 0x2c, 0x20, 0x63, 0x65, 0x72, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61,
    0x74, 0x65, 0x20, 0x70, 0x6f, 0x6c, 0x69, 0x63, 0x79, 0x20, 0x61, 0x6e,
    0x64, 0x20, 0x63, 0x65, 0x72, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74,
    0x69, 0x6f, 0x6e, 0x20, 0x70, 0x72, 0x61, 0x63, 0x74, 0x69, 0x63, 0x65,
    0x20, 0x73, 0x74, 0x61, 0x74, 0x65, 0x6d, 0x65, 0x6e, 0x74, 0x73, 0x2e,
    0x30, 0x36, 0x06, 0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x02, 0x01,
    0x16, 0x2a, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x77, 0x77, 0x77,
    0x2e, 0x61, 0x70, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x2f, 0x63,
    0x65, 0x72, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74, 0x65, 0x61, 0x75,
    0x74, 0x68, 0x6f, 0x72, 0x69, 0x74, 0x79, 0x2f, 0x30, 0x0e, 0x06, 0x03,
    0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04, 0x04, 0x03, 0x02, 0x07, 0x80,
    0x30, 0x14, 0x06, 0x0b, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x63, 0x64, 0x06,
    0x01, 0x18, 0x01, 0x01, 0x01, 0xff, 0x04, 0x02, 0x05, 0x00, 0x30, 0x0d,
    0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05,
    0x00, 0x03, 0x82, 0x01, 0x01, 0x00, 0x58, 0xef, 0x32, 0x6c, 0x48, 0x29,
    0xfa, 0x5e, 0x5e, 0x32, 0xa6, 0xbe, 0xe4, 0xd2, 0x3e, 0x72, 0xcf, 0xb9,
    0x74, 0x62, 0x84, 0x90, 0xa1, 0x5f, 0xbb, 0xd3, 0x3d, 0x67, 0x19, 0xf4,
    0x1b, 0xa1, 0x31, 0x38, 0xe0, 0xdb, 0xe4, 0x14, 0x6d, 0x9e, 0x99, 0x34,
    0xd3, 0x53, 0x97, 0xb4, 0xaa, 0x63, 0x61, 0x56, 0xac, 0x1e, 0x70, 0x54,
    0x98, 0x18, 0x2d, 0xc9, 0xa8, 0x31, 0x21, 0x95, 0x64, 0x25, 0xc1, 0x3e,
    0xfa, 0xbb, 0xc8, 0x13, 0x9b, 0x0c, 0xa5, 0xa5, 0xc2, 0x8e, 0x4e, 0xad,
    0x25, 0xef, 0xbe, 0x94, 0xe6, 0x0e, 0x91, 0x36, 0x44, 0xad, 0x93, 0x12,
    0x20, 0x3c, 0x3a, 0xc0, 0xfe, 0x6d, 0x47, 0xbe, 0xa1, 0x29, 0xde, 0x53,
    0xee, 0x6c, 0xee, 0x56, 0xec, 0xae, 0xeb, 0x08, 0x24, 0x3e, 0x43, 0xef,
    0x92, 0x6b, 0x2a, 0x66, 0x5c, 0x9f, 0x25, 0x77, 0x4e, 0x96, 0x45, 0x4d,
    0xd7, 0xac, 0xc0, 0xc8, 0xfe, 0xd2, 0x37, 0x52, 0xc8, 0xcb, 0xe3, 0x26,
    0xad, 0xb2, 0xd9, 0x90, 0x3f, 0x68, 0x93, 0xb5, 0x3f, 0x10, 0xd3, 0x61,
    0xb7, 0x09, 0x35, 0x42, 0xd4, 0xf4, 0xde, 0x3b, 0x42, 0x3e, 0x8c, 0xe1,
    0xe8, 0xa7, 0xcb, 0x24, 0x2c, 0x38, 0xd1, 0xa0, 0x99, 0x22, 0xd9, 0xab,
    0x3a, 0x39, 0xda, 0x78, 0x22, 0x2a, 0x01, 0xe2, 0xda, 0x30, 0x0b, 0x82,
    0xca, 0x7d, 0xe0, 0xca, 0xd0, 0x95, 0x13, 0x50, 0x4f, 0x85, 0x86, 0x83,
    0x3d, 0x3d, 0xa2, 0x2c, 0xeb, 0x46, 0x7c, 0x50, 0xc0, 0x5a, 0x60, 0x7b,
    0x70, 0xb5, 0x5f, 0xb7, 0xa8, 0x54, 0x81, 0xe7, 0xb0, 0xf2, 0x91, 0xc6,
    0xd6, 0xc1, 0xc4, 0xd6, 0xdb, 0xea, 0xfa, 0xf4, 0xf0, 0x6c, 0x00, 0xbf,
    0x0f, 0x71, 0xff, 0xb3, 0x6c, 0x59, 0x08, 0x2f, 0x28, 0xd3, 0xaf, 0xc3,
    0xd2, 0xde, 0xe1, 0x1a, 0x54, 0x76, 0xfe, 0x2c, 0x98, 0xf1,
};

// Issuer: C=US, O=Apple Inc., OU=Apple Worldwide Developer Relations, CN=Apple Worldwide Developer Relations Certification Authority
// Subject: CN=Apple TVOS Application Signing, O=Apple Inc., C=US
static const UInt8 kATVAppSigningCert[] = {
    0x30, 0x82, 0x05, 0x5f, 0x30, 0x82, 0x04, 0x47, 0xa0, 0x03, 0x02, 0x01,
    0x02, 0x02, 0x08, 0x34, 0xc4, 0xe1, 0x74, 0xfd, 0x82, 0xed, 0x21, 0x30,
    0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b,
    0x05, 0x00, 0x30, 0x81, 0x96, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55,
    0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03,
    0x55, 0x04, 0x0a, 0x0c, 0x0a, 0x41, 0x70, 0x70, 0x6c, 0x65, 0x20, 0x49,
    0x6e, 0x63, 0x2e, 0x31, 0x2c, 0x30, 0x2a, 0x06, 0x03, 0x55, 0x04, 0x0b,
    0x0c, 0x23, 0x41, 0x70, 0x70, 0x6c, 0x65, 0x20, 0x57, 0x6f, 0x72, 0x6c,
    0x64, 0x77, 0x69, 0x64, 0x65, 0x20, 0x44, 0x65, 0x76, 0x65, 0x6c, 0x6f,
    0x70, 0x65, 0x72, 0x20, 0x52, 0x65, 0x6c, 0x61, 0x74, 0x69, 0x6f, 0x6e,
    0x73, 0x31, 0x44, 0x30, 0x42, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x3b,
    0x41, 0x70, 0x70, 0x6c, 0x65, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x77,
    0x69, 0x64, 0x65, 0x20, 0x44, 0x65, 0x76, 0x65, 0x6c, 0x6f, 0x70, 0x65,
    0x72, 0x20, 0x52, 0x65, 0x6c, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x73, 0x20,
    0x43, 0x65, 0x72, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6f,
    0x6e, 0x20, 0x41, 0x75, 0x74, 0x68, 0x6f, 0x72, 0x69, 0x74, 0x79, 0x30,
    0x1e, 0x17, 0x0d, 0x31, 0x35, 0x30, 0x36, 0x32, 0x36, 0x32, 0x32, 0x34,
    0x30, 0x31, 0x37, 0x5a, 0x17, 0x0d, 0x32, 0x33, 0x30, 0x32, 0x30, 0x37,
    0x32, 0x31, 0x34, 0x38, 0x34, 0x37, 0x5a, 0x30, 0x4b, 0x31, 0x27, 0x30,
    0x25, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x1e, 0x41, 0x70, 0x70, 0x6c,
    0x65, 0x20, 0x54, 0x56, 0x4f, 0x53, 0x20, 0x41, 0x70, 0x70, 0x6c, 0x69,
    0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x53, 0x69, 0x67, 0x6e, 0x69,
    0x6e, 0x67, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c,
    0x0a, 0x41, 0x70, 0x70, 0x6c, 0x65, 0x20, 0x49, 0x6e, 0x63, 0x2e, 0x31,
    0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53,
    0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
    0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00,
    0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xba, 0x8f, 0xd0,
    0x2b, 0xfb, 0x04, 0x41, 0x7e, 0xef, 0x73, 0xf1, 0x86, 0x5b, 0xce, 0xe8,
    0x0d, 0xb5, 0xec, 0x5f, 0xd9, 0x24, 0x49, 0x6d, 0x5c, 0x97, 0xeb, 0xb2,
    0xa6, 0xfb, 0x7c, 0x9f, 0xcf, 0xd0, 0x18, 0xfa, 0xa1, 0xdf, 0x9f, 0x4a,
    0x42, 0xc3, 0xc3, 0xd3, 0x46, 0x91, 0x8c, 0x74, 0x3b, 0x6e, 0x54, 0xb8,
    0xe7, 0xec, 0x10, 0x8b, 0xc0, 0x2f, 0xe8, 0x96, 0x86, 0xaa, 0x8b, 0xb7,
    0x8f, 0xee, 0x2a, 0x31, 0xf3, 0xaf, 0x04, 0x77, 0x16, 0x09, 0x9e, 0xf9,
    0x9d, 0x30, 0x74, 0x5d, 0x9e, 0xb1, 0x11, 0x66, 0xef, 0x0d, 0x61, 0x1c,
    0xc2, 0xfe, 0x6b, 0x75, 0x80, 0x0e, 0x42, 0x14, 0x4e, 0xdc, 0x38, 0xfd,
    0x18, 0x22, 0x03, 0xe0, 0x51, 0xbd, 0xd0, 0xf3, 0x52, 0x36, 0xff, 0x83,
    0x90, 0xde, 0xbe, 0x60, 0xec, 0x82, 0x66, 0xad, 0x49, 0x54, 0x71, 0x39,
    0xdd, 0x48, 0xc3, 0x13, 0x99, 0xc2, 0xcc, 0x77, 0x55, 0x5e, 0x48, 0xeb,
    0xee, 0x34, 0x31, 0x04, 0xef, 0x7e, 0xe1, 0x42, 0x54, 0x10, 0xcf, 0x09,
    0x9c, 0x0d, 0xc4, 0x55, 0x3d, 0x30, 0x98, 0x78, 0xfb, 0x38, 0xac, 0xdb,
    0xd8, 0x63, 0x3f, 0x64, 0x07, 0x7f, 0x53, 0x4d, 0xc8, 0xbc, 0x60, 0x3e,
    0x89, 0x49, 0x88, 0x07, 0xb4, 0x80, 0x15, 0xd5, 0xc2, 0x13, 0x8b, 0xff,
    0x0c, 0x90, 0xb6, 0x67, 0x0c, 0xaf, 0xf4, 0xef, 0x5c, 0x9d, 0xba, 0xf3,
    0x95, 0x5b, 0xd2, 0x9a, 0x7e, 0x80, 0x8d, 0xc9, 0x6f, 0xcd, 0x75, 0xe5,
    0xb6, 0xfb, 0x61, 0x8b, 0x9c, 0x3b, 0xce, 0xc2, 0x4c, 0xba, 0xb7, 0xf6,
    0x48, 0xa6, 0x79, 0x4a, 0x34, 0xf1, 0xe1, 0x47, 0xba, 0x29, 0x5d, 0x04,
    0x26, 0x64, 0xee, 0x5e, 0x8e, 0x0c, 0x9d, 0xa7, 0x05, 0xe3, 0x58, 0xd7,
    0xe4, 0xb5, 0x4e, 0x7b, 0xdc, 0x2a, 0xab, 0xc1, 0xea, 0x82, 0x7d, 0xcb,
    0x93, 0x02, 0x03, 0x01, 0x00, 0x01, 0xa3, 0x82, 0x01, 0xf9, 0x30, 0x82,
    0x01, 0xf5, 0x30, 0x47, 0x06, 0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07,
    0x01, 0x01, 0x04, 0x3b, 0x30, 0x39, 0x30, 0x37, 0x06, 0x08, 0x2b, 0x06,
    0x01, 0x05, 0x05, 0x07, 0x30, 0x01, 0x86, 0x2b, 0x68, 0x74, 0x74, 0x70,
    0x3a, 0x2f, 0x2f, 0x6f, 0x63, 0x73, 0x70, 0x2e, 0x61, 0x70, 0x70, 0x6c,
    0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x2f, 0x6f, 0x63, 0x73, 0x70, 0x30, 0x34,
    0x2d, 0x61, 0x70, 0x70, 0x6c, 0x65, 0x77, 0x77, 0x64, 0x72, 0x63, 0x61,
    0x32, 0x30, 0x33, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16,
    0x04, 0x14, 0x49, 0xaa, 0xae, 0x84, 0x57, 0x14, 0x56, 0x8f, 0x0b, 0xeb,
    0x63, 0x6b, 0x62, 0x75, 0x68, 0xfc, 0x5b, 0x8c, 0x77, 0xa1, 0x30, 0x0c,
    0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x02, 0x30, 0x00,
    0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80,
    0x14, 0x88, 0x27, 0x17, 0x09, 0xa9, 0xb6, 0x18, 0x60, 0x8b, 0xec, 0xeb,
    0xba, 0xf6, 0x47, 0x59, 0xc5, 0x52, 0x54, 0xa3, 0xb7, 0x30, 0x82, 0x01,
    0x1d, 0x06, 0x03, 0x55, 0x1d, 0x20, 0x04, 0x82, 0x01, 0x14, 0x30, 0x82,
    0x01, 0x10, 0x30, 0x82, 0x01, 0x0c, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
    0xf7, 0x63, 0x64, 0x05, 0x01, 0x30, 0x81, 0xfe, 0x30, 0x81, 0xc3, 0x06,
    0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x02, 0x02, 0x30, 0x81, 0xb6,
    0x0c, 0x81, 0xb3, 0x52, 0x65, 0x6c, 0x69, 0x61, 0x6e, 0x63, 0x65, 0x20,
    0x6f, 0x6e, 0x20, 0x74, 0x68, 0x69, 0x73, 0x20, 0x63, 0x65, 0x72, 0x74,
    0x69, 0x66, 0x69, 0x63, 0x61, 0x74, 0x65, 0x20, 0x62, 0x79, 0x20, 0x61,
    0x6e, 0x79, 0x20, 0x70, 0x61, 0x72, 0x74, 0x79, 0x20, 0x61, 0x73, 0x73,
    0x75, 0x6d, 0x65, 0x73, 0x20, 0x61, 0x63, 0x63, 0x65, 0x70, 0x74, 0x61,
    0x6e, 0x63, 0x65, 0x20, 0x6f, 0x66, 0x20, 0x74, 0x68, 0x65, 0x20, 0x74,
    0x68, 0x65, 0x6e, 0x20, 0x61, 0x70, 0x70, 0x6c, 0x69, 0x63, 0x61, 0x62,
    0x6c, 0x65, 0x20, 0x73, 0x74, 0x61, 0x6e, 0x64, 0x61, 0x72, 0x64, 0x20,
    0x74, 0x65, 0x72, 0x6d, 0x73, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x63, 0x6f,
    0x6e, 0x64, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x73, 0x20, 0x6f, 0x66, 0x20,
    0x75, 0x73, 0x65, 0x2c, 0x20, 0x63, 0x65, 0x72, 0x74, 0x69, 0x66, 0x69,
    0x63, 0x61, 0x74, 0x65, 0x20, 0x70, 0x6f, 0x6c, 0x69, 0x63, 0x79, 0x20,
    0x61, 0x6e, 0x64, 0x20, 0x63, 0x65, 0x72, 0x74, 0x69, 0x66, 0x69, 0x63,
    0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x70, 0x72, 0x61, 0x63, 0x74, 0x69,
    0x63, 0x65, 0x20, 0x73, 0x74, 0x61, 0x74, 0x65, 0x6d, 0x65, 0x6e, 0x74,
    0x73, 0x2e, 0x30, 0x36, 0x06, 0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07,
    0x02, 0x01, 0x16, 0x2a, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x77,
    0x77, 0x77, 0x2e, 0x61, 0x70, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d,
    0x2f, 0x63, 0x65, 0x72, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74, 0x65,
    0x61, 0x75, 0x74, 0x68, 0x6f, 0x72, 0x69, 0x74, 0x79, 0x2f, 0x30, 0x0e,
    0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04, 0x04, 0x03, 0x02,
    0x07, 0x80, 0x30, 0x16, 0x06, 0x03, 0x55, 0x1d, 0x25, 0x01, 0x01, 0xff,
    0x04, 0x0c, 0x30, 0x0a, 0x06, 0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07,
    0x03, 0x03, 0x30, 0x13, 0x06, 0x0a, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x63,
    0x64, 0x06, 0x01, 0x18, 0x01, 0x01, 0xff, 0x04, 0x02, 0x05, 0x00, 0x30,
    0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b,
    0x05, 0x00, 0x03, 0x82, 0x01, 0x01, 0x00, 0x9c, 0x07, 0xde, 0xde, 0xc2,
    0xfc, 0x6c, 0x94, 0xb1, 0x1a, 0x6a, 0x38, 0x75, 0xe0, 0x74, 0x70, 0xe9,
    0x9d, 0x47, 0xd6, 0xde, 0xcd, 0xd0, 0xdb, 0xed, 0x2f, 0x50, 0xfa, 0x0d,
    0xe3, 0xb9, 0x3d, 0x36, 0xc9, 0x4b, 0xee, 0x4e, 0xc4, 0x83, 0xb9, 0x7d,
    0x40, 0x01, 0x92, 0x3f, 0x18, 0x8a, 0x19, 0xe8, 0xac, 0x5d, 0xb1, 0xc1,
    0xd2, 0x30, 0x98, 0x85, 0x28, 0x91, 0x0c, 0x92, 0x71, 0x79, 0xec, 0x4b,
    0x51, 0xcc, 0xdf, 0x99, 0x71, 0x87, 0x04, 0x60, 0x09, 0x3e, 0xfa, 0x56,
    0x9f, 0x99, 0xa3, 0xef, 0x0c, 0x02, 0xd2, 0xdf, 0xcf, 0x18, 0xf2, 0x34,
    0x6e, 0x93, 0xd0, 0x0e, 0x81, 0xe4, 0x4e, 0x37, 0x7b, 0x1d, 0xe7, 0x8c,
    0xa6, 0x71, 0x6d, 0x95, 0x66, 0x7d, 0xc0, 0x80, 0x74, 0x71, 0xe1, 0xd7,
    0x97, 0x35, 0x9b, 0x26, 0xe9, 0x84, 0x4a, 0x96, 0x30, 0xfc, 0xf1, 0x26,
    0x23, 0x1d, 0xec, 0x71, 0x2f, 0x39, 0x40, 0x14, 0xaf, 0x34, 0x0e, 0x85,
    0x3c, 0xd0, 0x9e, 0x8d, 0x4e, 0xf8, 0x04, 0x0a, 0xc2, 0x3f, 0x44, 0x7d,
    0x19, 0x2d, 0xe7, 0xc0, 0xf1, 0xce, 0xa9, 0x2f, 0x6c, 0x79, 0xbd, 0x65,
    0x69, 0x3e, 0xf6, 0x76, 0x59, 0xeb, 0x70, 0x0c, 0xaf, 0x04, 0x44, 0x82,
    0x02, 0x15, 0x24, 0x3e, 0xc3, 0xe0, 0x9e, 0x5d, 0xa0, 0xe3, 0x66, 0x72,
    0x59, 0x6e, 0x51, 0x41, 0xd6, 0x72, 0xdd, 0x4d, 0xca, 0x96, 0xb0, 0x1a,
    0xc1, 0x47, 0x5a, 0xef, 0xc9, 0xc4, 0x11, 0x11, 0x7a, 0xec, 0x9c, 0x1c,
    0x12, 0x19, 0x72, 0xb8, 0xc3, 0x98, 0x3e, 0x3b, 0xe7, 0x4a, 0x3f, 0xb8,
    0x48, 0x40, 0xd6, 0x68, 0xa9, 0xce, 0x07, 0xe7, 0x0e, 0x5e, 0x56, 0x33,
    0xf8, 0xb0, 0x4c, 0xc2, 0xb6, 0x25, 0xcc, 0x5f, 0xbd, 0xdb, 0xe5, 0x78,
    0xb6, 0x5f, 0x99, 0x3e, 0xdc, 0xaf, 0x20, 0x3d, 0x5a, 0x0f, 0x13
};

static void test_atv_appsigning_cert_policy()
{
    CFDateRef date=NULL;
    CFArrayRef policies=NULL;
    SecPolicyRef policy=NULL;
    SecTrustRef trust=NULL;
    SecCertificateRef testCert0=NULL, testCert1=NULL;
    SecCertificateRef prodCert0=NULL, prodCert1=NULL;
    CFMutableArrayRef testCerts=NULL, prodCerts=NULL;
    SecTrustResultType trustResult;
    CFIndex chainLen;
    
    /* Test hierarchy */
    isnt(testCert0 = SecCertificateCreateWithBytes(NULL, kTestATVAppSigningCert, sizeof(kTestATVAppSigningCert)),
         NULL, "create testCert0");
    isnt(testCert1 = SecCertificateCreateWithBytes(NULL, kWwdrAnchorCertificate, sizeof(kWwdrAnchorCertificate)),
         NULL, "create testCert1");
    
    isnt(testCerts = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks), NULL, "create test cert array");
    CFArrayAppendValue(testCerts, testCert0);
    CFArrayAppendValue(testCerts, testCert1);
    
    /* Production hierarchy */
    isnt(prodCert0 = SecCertificateCreateWithBytes(NULL, kATVAppSigningCert, sizeof(kATVAppSigningCert)),
         NULL, "create prodCert0");
    isnt(prodCert1 = SecCertificateCreateWithBytes(NULL, kWwdrAnchorCertificate, sizeof(kWwdrAnchorCertificate)),
         NULL, "create prodCert1");
    
    isnt(prodCerts = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks), NULL, "create prod cert array");
    CFArrayAppendValue(prodCerts, prodCert0);
    CFArrayAppendValue(prodCerts, prodCert1);
    
    /* Case 1: production policy with production certs (should succeed) */
    isnt(policy = SecPolicyCreateAppleATVAppSigning(), NULL, "create policy");
    policies = CFArrayCreate(NULL, (const void **)&policy, 1, &kCFTypeArrayCallBacks);
    CFRelease(policy);
    policy = NULL;
    ok_status(SecTrustCreateWithCertificates(prodCerts, policies, &trust),
              "create trust");
    CFRelease(policies);
    policies = NULL;
    isnt(date = CFDateCreateForGregorianZuluMoment(NULL, 2015, 7, 1, 12, 0, 0),
         NULL, "create verify date");
    //%%% policy currently doesn't care about expiration dates
    //ok_status(SecTrustSetVerifyDate(trust, date), "set date");
    CFReleaseSafe(date);
    date = NULL;
    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate trust");
    ok(trustResult == kSecTrustResultUnspecified, "trustResult 4 expected (got %d)",
       (int)trustResult);
    chainLen = SecTrustGetCertificateCount(trust);
    ok(chainLen == 3, "chain length 3 expected (got %d)", (int)chainLen);
    CFRelease(trust);
    trust = NULL;
    
    /* Case 2: test policy with test certs (should succeed) */
    isnt(policy = SecPolicyCreateTestAppleATVAppSigning(), NULL, "create policy");
    policies = CFArrayCreate(NULL, (const void **)&policy, 1, &kCFTypeArrayCallBacks);
    CFRelease(policy);
    policy = NULL;
    ok_status(SecTrustCreateWithCertificates(testCerts, policies, &trust),
              "create trust");
    CFRelease(policies);
    policies = NULL;
    isnt(date = CFDateCreateForGregorianZuluMoment(NULL, 2015, 7, 1, 12, 0, 0),
         NULL, "create verify date");
    ok_status(SecTrustSetVerifyDate(trust, date), "set date");
    CFReleaseSafe(date);
    date = NULL;
    
    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate trust");
    ok(trustResult == kSecTrustResultUnspecified, "trustResult 4 expected (got %d)",
       (int)trustResult);
    chainLen = SecTrustGetCertificateCount(trust);
    ok(chainLen == 3, "chain length 3 expected (got %d)", (int)chainLen);
    CFRelease(trust);
    trust = NULL;
    
    /* Case 3: production policy with test certs (should fail) */
    isnt(policy = SecPolicyCreateAppleATVAppSigning(), NULL, "create policy");
    policies = CFArrayCreate(NULL, (const void **)&policy, 1, &kCFTypeArrayCallBacks);
    CFRelease(policy);
    policy = NULL;
    ok_status(SecTrustCreateWithCertificates(testCerts, policies, &trust),
              "create trust");
    CFRelease(policies);
    policies = NULL;
    isnt(date = CFDateCreateForGregorianZuluMoment(NULL, 2015, 7, 1, 12, 0, 0),
         NULL, "create verify date");
    ok_status(SecTrustSetVerifyDate(trust, date), "set date");
    CFReleaseSafe(date);
    date = NULL;
    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate trust");
    ok(trustResult == kSecTrustResultRecoverableTrustFailure, "trustResult 5 expected (got %d)",
       (int)trustResult);
    CFRelease(trust);
    trust = NULL;
    
    /* Case 4: test policy with production certs (should fail) */
    isnt(policy = SecPolicyCreateTestAppleATVAppSigning(), NULL, "create policy");
    policies = CFArrayCreate(NULL, (const void **)&policy, 1, &kCFTypeArrayCallBacks);
    CFRelease(policy);
    policy = NULL;
    ok_status(SecTrustCreateWithCertificates(prodCerts, policies, &trust),
              "create trust");
    CFRelease(policies);
    policies = NULL;
    isnt(date = CFDateCreateForGregorianZuluMoment(NULL, 2015, 7, 1, 12, 0, 0),
         NULL, "create verify date");
    //%%% policy currently doesn't care about expiration dates
    //ok_status(SecTrustSetVerifyDate(trust, date), "set date");
    CFReleaseSafe(date);
    date = NULL;
    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate trust");
    ok(trustResult == kSecTrustResultRecoverableTrustFailure, "trustResult 5 expected (got %d)",
       (int)trustResult);
    CFRelease(trust);
    trust = NULL;
    
    CFReleaseSafe(testCert0);
    CFReleaseSafe(testCert1);
    CFReleaseSafe(prodCert0);
    CFReleaseSafe(prodCert1);
    
    CFReleaseSafe(testCerts);
    CFReleaseSafe(prodCerts);
}

static void tests(void)
{
    test_atv_appsigning_cert_policy();
}

int si_84_atv_appsigning(int argc, char *const *argv)
{
    plan_tests(30);
    
    tests();
    
    return 0;
}