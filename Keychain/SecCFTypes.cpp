/*
 * Copyright (c) 2002 Apple Computer, Inc. All Rights Reserved.
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

//
// SecCFTypes.cpp - CF runtime interface
//

#include <Security/SecCFTypes.h>
#include <Security/SecRuntime.h>

namespace Security
{

namespace KeychainCore
{

SecCFTypes &
gTypes()
{
	static ModuleNexus<SecCFTypes> nexus;

	return nexus();
}

} // end namespace KeychainCore

} // end namespace Security

using namespace KeychainCore;

SecCFTypes::SecCFTypes() :
	Access("SecAccess"),
	ACL("SecACL"),
	Certificate("SecCertificate"),
	CertificateRequest("SecCertificateRequest"),
	Identity("SecIdentity"),
	IdentityCursor("SecIdentitySearch"),
	ItemImpl("SecKeychainItem"),
	KCCursorImpl("SecKeychainSearch"),
	KeychainImpl("SecKeychain"),
	KeyItem("SecKey"),
	Policy("SecPolicy"),
	PolicyCursor("SecPolicySearch"),
	Trust("SecTrust"),
	TrustedApplication("SecTrustedApplication")
{
}

//
// CFClass
//
CFClass::CFClass(const char *name)
{
	// initialize the CFRuntimeClass structure
	version = 0;
	className = name;
	init = NULL;
	copy = NULL;
	finalize = finalizeType;
	equal = equalType;
	hash = hashType;
	copyFormattingDesc = copyFormattingDescType;
	copyDebugDesc = copyDebugDescType;
	
	// register
	typeID = _CFRuntimeRegisterClass(this);
	assert(typeID != _kCFRuntimeNotATypeID);
}
    
void
CFClass::finalizeType(CFTypeRef cf)
{
	SecCFObject *obj = SecCFObject::optional(cf);
	if (!obj->isNew())
		obj->~SecCFObject();
}

Boolean
CFClass::equalType(CFTypeRef cf1, CFTypeRef cf2)
{
	// CF checks for pointer equality and ensures type equality already
	return SecCFObject::optional(cf1)->equal(*SecCFObject::optional(cf2));
}

CFHashCode
CFClass::hashType(CFTypeRef cf)
{
	return SecCFObject::optional(cf)->hash();
}

CFStringRef
CFClass::copyFormattingDescType(CFTypeRef cf, CFDictionaryRef dict)
{
	return SecCFObject::optional(cf)->copyFormattingDesc(dict);
}

CFStringRef
CFClass::copyDebugDescType(CFTypeRef cf)
{
	return SecCFObject::optional(cf)->copyDebugDesc();
}