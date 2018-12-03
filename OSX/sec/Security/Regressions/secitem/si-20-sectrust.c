/*
 * Copyright (c) 2006-2010,2012-2018 Apple Inc. All Rights Reserved.
 */

#include <AssertMacros.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Security/SecCertificate.h>
#include <Security/SecCertificatePriv.h>
#include <Security/SecPolicyPriv.h>
#include <Security/SecTrustPriv.h>
#include <Security/SecTrust.h>
#include <Security/SecItem.h>
#include <utilities/array_size.h>
#include <utilities/SecCFWrappers.h>
#include <stdlib.h>
#include <unistd.h>
#include <Security/SecTrustSettings.h>

#if TARGET_OS_IPHONE
#include <Security/SecInternal.h>
#include <ipc/securityd_client.h>
#endif

#include "shared_regressions.h"
#include "si-20-sectrust.h"

/* Test SecTrust API. */
static void basic_tests(void)
{
    SecTrustRef trust = NULL;
    CFArrayRef _anchors = NULL, certs = NULL, anchors = NULL, replacementPolicies;
	SecCertificateRef cert0 = NULL, cert1 = NULL, _anchor = NULL, cert_google = NULL, garthc2 = NULL;
    SecPolicyRef policy = NULL, replacementPolicy = NULL, replacementPolicy2 = NULL;
    CFDateRef date = NULL;
    CFDataRef c0_serial = NULL, serial = NULL;
    CFDictionaryRef query = NULL;

	isnt(cert0 = SecCertificateCreateWithBytes(NULL, _c0, sizeof(_c0)),
		NULL, "create cert0");
	isnt(cert1 = SecCertificateCreateWithBytes(NULL, _c1, sizeof(_c1)),
		NULL, "create cert1");
	const void *v_certs[] = {
		cert0,
		cert1
	};
    policy = SecPolicyCreateSSL(false, NULL);
    certs = CFArrayCreate(NULL, v_certs,
		array_size(v_certs), &kCFTypeArrayCallBacks);

    /* SecTrustCreateWithCertificates failures. */
    is_status(SecTrustCreateWithCertificates(kCFBooleanTrue, policy, &trust),
        errSecParam, "create trust with boolean instead of cert");
    is_status(SecTrustCreateWithCertificates(cert0, kCFBooleanTrue, &trust),
        errSecParam, "create trust with boolean instead of policy");

	/* SecTrustCreateWithCertificates using array of certs. */
	ok_status(SecTrustCreateWithCertificates(certs, policy, &trust), "create trust");

    /* NOTE: prior to <rdar://11810677 SecTrustGetCertificateCount would return 1 at this point.
     * Now, however, we do an implicit SecTrustEvaluate to build the chain if it has not yet been
     * evaluated, so we now expect the full chain length.
     */
    is(SecTrustGetCertificateCount(trust), 3, "cert count is 3");
    is(SecTrustGetCertificateAtIndex(trust, 0), cert0, "cert 0 is leaf");

	/* Apr 14 2018. */
    isnt(date = CFDateCreateForGregorianZuluMoment(NULL, 2018, 4, 14, 12, 0, 0),
         NULL, "create verify date");
    if (!date) { goto errOut; }
    ok_status(SecTrustSetVerifyDate(trust, date), "set date");

	SecTrustResultType trustResult;

#if TARGET_OS_IPHONE
SKIP: {
#ifdef NO_SERVER
    skip("Can't fail to connect to securityd in NO_SERVER mode", 4, false);
#endif
    // Test Restore OS environment
    SecServerSetTrustdMachServiceName("com.apple.security.doesn't-exist");
    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate trust without securityd running");
    is_status(trustResult, kSecTrustResultInvalid, "trustResult is kSecTrustResultInvalid");
	is(SecTrustGetCertificateCount(trust), 1, "cert count is 1 without securityd running");
    SecKeyRef pubKey = NULL;
    ok(pubKey = SecTrustCopyPublicKey(trust), "copy public key without securityd running");
    CFReleaseNull(pubKey);
    SecServerSetTrustdMachServiceName("com.apple.trustd");
    // End of Restore OS environment tests
}
#endif

    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate trust");
    is_status(trustResult, kSecTrustResultUnspecified,
		"trustResult is kSecTrustResultUnspecified");

	is(SecTrustGetCertificateCount(trust), 3, "cert count is 3");

    if (!cert0) { goto errOut; }
    c0_serial = CFDataCreate(NULL, _c0_serial, sizeof(_c0_serial));
    CFErrorRef error = NULL;
    ok(serial = SecCertificateCopySerialNumberData(cert0, &error), "copy cert0 serial");
    CFReleaseNull(error);
	ok(CFEqual(c0_serial, serial), "serial matches");
    CFReleaseNull(serial);
    CFReleaseNull(c0_serial);

    anchors = CFArrayCreate(NULL, (const void **)&cert1, 1, &kCFTypeArrayCallBacks);
    ok_status(SecTrustSetAnchorCertificates(trust, anchors), "set anchors");
    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate trust");
    is_status(trustResult, kSecTrustResultUnspecified,
		"trust is kSecTrustResultUnspecified");
	is(SecTrustGetCertificateCount(trust), 2, "cert count is 2");
#if TARGET_OS_OSX
    CFArrayRef certArray;
	ok_status(SecTrustCopyAnchorCertificates(&certArray), "copy anchors");
    CFReleaseSafe(certArray);
	ok_status(SecTrustSettingsCopyCertificates(kSecTrustSettingsDomainSystem, &certArray), "copy certificates");
    CFReleaseSafe(certArray);
#endif
	CFReleaseNull(anchors);
    anchors = CFArrayCreate(NULL, NULL, 0, NULL);
    ok_status(SecTrustSetAnchorCertificates(trust, anchors), "set empty anchors list");
    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate trust");
    is_status(trustResult, kSecTrustResultRecoverableTrustFailure,
		"trust is kSecTrustResultRecoverableTrustFailure");
    CFReleaseNull(anchors);

	ok_status(SecTrustSetAnchorCertificatesOnly(trust, false), "trust passed in anchors and system anchors");
    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate trust");
    is_status(trustResult, kSecTrustResultUnspecified,
		"trust is kSecTrustResultUnspecified");

	ok_status(SecTrustSetAnchorCertificatesOnly(trust, true), "only trust passed in anchors (default)");
    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate trust");
    is_status(trustResult, kSecTrustResultRecoverableTrustFailure,
		"trust is kSecTrustResultRecoverableTrustFailure");

    /* Test cert_1 intermediate from the keychain. */
    CFReleaseNull(trust);
    ok_status(SecTrustCreateWithCertificates(cert0, policy, &trust),
              "create trust with single cert0");
    ok_status(SecTrustSetVerifyDate(trust, date), "set date");

    // Add cert1
    query = CFDictionaryCreateForCFTypes(kCFAllocatorDefault,
        kSecClass, kSecClassCertificate, kSecValueRef, cert1, NULL);
    ok_status(SecItemAdd(query, NULL), "add cert1 to keychain");
    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate trust");
    // Cleanup added cert1.
    ok_status(SecItemDelete(query), "remove cert1 from keychain");
    CFReleaseNull(query);
    is_status(trustResult, kSecTrustResultUnspecified,
              "trust is kSecTrustResultUnspecified");
	is(SecTrustGetCertificateCount(trust), 3, "cert count is 3");

    /* Set certs to be the www.google.com leaf. */
	CFReleaseNull(certs);
	isnt(cert_google = SecCertificateCreateWithBytes(NULL, google_certificate,
        sizeof(google_certificate)), NULL, "create cert_google");
    certs = CFArrayCreate(NULL, (const void **)&cert_google, 1, &kCFTypeArrayCallBacks);

	CFReleaseNull(trust);
	CFReleaseNull(policy);
	CFReleaseNull(date);
    bool server = true;
    policy = SecPolicyCreateSSL(server, CFSTR("www.google.com"));
    ok_status(SecTrustCreateWithCertificates(certs, policy, &trust),
        "create trust for ssl server www.google.com");

    isnt(_anchor = SecCertificateCreateWithBytes(NULL, googleInternetAuthoritySubCA, sizeof(googleInternetAuthoritySubCA)),
         NULL, "create root");
    const void *v_anchors[] = { _anchor };
    isnt(_anchors = CFArrayCreate(NULL, v_anchors, array_size(v_anchors), &kCFTypeArrayCallBacks),
         NULL, "create anchors");
    if (!_anchors) { goto errOut; }
    ok_status(SecTrustSetAnchorCertificates(trust, _anchors), "set anchors");

    /* May 23, 2018. */
    date = CFDateCreate(NULL, 548800000.0);
    ok_status(SecTrustSetVerifyDate(trust, date), "set www.google.com trust date to May 23, 2018");
    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate www.google.com trust");
    is_status(trustResult, kSecTrustResultUnspecified,
		"trust is kSecTrustResultUnspecified");

	CFReleaseNull(trust);
	CFReleaseNull(policy);
    server = false;
    policy = SecPolicyCreateSSL(server, CFSTR("www.google.com"));
    ok_status(SecTrustCreateWithCertificates(certs, policy, &trust),
        "create trust for ssl client www.google.com");
    ok_status(SecTrustSetAnchorCertificates(trust, _anchors), "set anchors");
    ok_status(SecTrustSetVerifyDate(trust, date), "set google trust date to May 23, 2018");
    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate google trust");
    is_status(trustResult, kSecTrustResultRecoverableTrustFailure,
		"trust is kSecTrustResultRecoverableTrustFailure");

	CFReleaseNull(trust);
	CFReleaseNull(policy);
    server = true;
    policy = SecPolicyCreateIPSec(server, CFSTR("www.google.com"));
    ok_status(SecTrustCreateWithCertificates(certs, policy, &trust),
        "create trust for ip server www.google.com");
    ok_status(SecTrustSetAnchorCertificates(trust, _anchors), "set anchors");
    ok_status(SecTrustSetVerifyDate(trust, date), "set www.apple.com trust date to May 23, 2018");
    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate www.google.com trust");
#if 0
    /* Although this shouldn't be a valid ipsec cert, since we no longer
       check for ekus in the ipsec policy it is. */
    is_status(trustResult, kSecTrustResultRecoverableTrustFailure,
		"trust is kSecTrustResultRecoverableTrustFailure");
#else
    is_status(trustResult, kSecTrustResultUnspecified,
		"trust is kSecTrustResultUnspecified");
#endif

	CFReleaseNull(trust);
	CFReleaseNull(policy);
    server = true;
    policy = SecPolicyCreateSSL(server, CFSTR("nowhere.com"));
    ok_status(SecTrustCreateWithCertificates(certs, policy, &trust),
        "create trust for ssl server nowhere.com");
    replacementPolicy = SecPolicyCreateSSL(server, CFSTR("www.google.com"));
    SecTrustSetPolicies(trust, replacementPolicy);
    CFReleaseNull(replacementPolicy);
    ok_status(SecTrustSetAnchorCertificates(trust, _anchors), "set anchors");
    ok_status(SecTrustSetVerifyDate(trust, date), "set www.google.com trust date to May 23, 2018");
    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate www.google.com trust");
    is_status(trustResult, kSecTrustResultUnspecified,
		"trust is kSecTrustResultUnspecified");

	CFReleaseNull(trust);
	CFReleaseNull(policy);
    server = true;
    policy = SecPolicyCreateSSL(server, CFSTR("nowhere.com"));
    ok_status(SecTrustCreateWithCertificates(certs, policy, &trust),
        "create trust for ssl server nowhere.com");
    replacementPolicy2 = SecPolicyCreateSSL(server, CFSTR("www.google.com"));
    replacementPolicies = CFArrayCreate(kCFAllocatorDefault, (CFTypeRef*)&replacementPolicy2, 1, &kCFTypeArrayCallBacks);
    SecTrustSetPolicies(trust, replacementPolicies);
    CFReleaseNull(replacementPolicy2);
    CFReleaseNull(replacementPolicies);
    ok_status(SecTrustSetAnchorCertificates(trust, _anchors), "set anchors");
    ok_status(SecTrustSetVerifyDate(trust, date), "set www.google.com trust date to May 23, 2018");
    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate www.google.com trust");
    is_status(trustResult, kSecTrustResultUnspecified,
		"trust is kSecTrustResultUnspecified");

    /* Test self signed ssl cert with cert itself set as anchor. */
	CFReleaseNull(trust);
	CFReleaseNull(policy);
	CFReleaseNull(certs);
	CFReleaseNull(date);
    server = true;
	isnt(garthc2 = SecCertificateCreateWithBytes(NULL, garthc2_certificate,
        sizeof(garthc2_certificate)), NULL, "create garthc2");
    certs = CFArrayCreate(NULL, (const void **)&garthc2, 1, &kCFTypeArrayCallBacks);
    policy = SecPolicyCreateSSL(server, NULL);
    ok_status(SecTrustCreateWithCertificates(certs, policy, &trust),
        "create trust for ip server garthc2.apple.com");
    date = CFDateCreate(NULL, 269568000.0);
    ok_status(SecTrustSetVerifyDate(trust, date),
        "set garthc2 trust date to Aug 2009");
    ok_status(SecTrustSetAnchorCertificates(trust, certs),
        "set garthc2 as anchor");
    ok_status(SecTrustEvaluate(trust, &trustResult),
        "evaluate self signed cert with cert as anchor");
    is_status(trustResult, kSecTrustResultUnspecified,
		"trust is kSecTrustResultUnspecified");


errOut:
    CFReleaseSafe(garthc2);
    CFReleaseSafe(cert_google);
    CFReleaseSafe(anchors);
    CFReleaseSafe(trust);
    CFReleaseSafe(serial);
    CFReleaseSafe(c0_serial);
    CFReleaseSafe(policy);
    CFReleaseSafe(certs);
    CFReleaseSafe(cert0);
    CFReleaseSafe(cert1);
    CFReleaseSafe(date);

    CFReleaseSafe(_anchor);
    CFReleaseSafe(_anchors);
}

static void negative_integer_tests(void)
{
    /* Test that we can handle and fix up negative integer value(s) in ECDSA signature */
    const void *negIntSigLeaf;
    isnt(negIntSigLeaf = SecCertificateCreateWithBytes(NULL, _leaf_NegativeIntInSig,
                                                       sizeof(_leaf_NegativeIntInSig)), NULL, "create negIntSigLeaf");
    CFArrayRef certs = NULL;
    isnt(certs = CFArrayCreate(NULL, &negIntSigLeaf, 1, &kCFTypeArrayCallBacks), NULL, "failed to create certs array");
    SecPolicyRef policy = NULL;
    isnt(policy = SecPolicyCreateiAP(), NULL, "failed to create policy");
    SecTrustRef trust = NULL;
    ok_status(SecTrustCreateWithCertificates(certs, policy, &trust),
              "create trust for negIntSigLeaf");

    const void *rootAACA2;
    isnt(rootAACA2 = SecCertificateCreateWithBytes(NULL, _root_AACA2,
                                                   sizeof(_root_AACA2)), NULL, "create rootAACA2");
    CFArrayRef anchors = NULL;
    isnt(anchors = CFArrayCreate(NULL, &rootAACA2, 1, &kCFTypeArrayCallBacks), NULL, "failed to create anchors array");
    if (!anchors) { goto errOut; }
    ok_status(SecTrustSetAnchorCertificates(trust, anchors), "set anchor certificates");

    SecTrustResultType trustResult;
    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate trust");
    is_status(trustResult, kSecTrustResultUnspecified, "expected kSecTrustResultUnspecified");

errOut:
    CFReleaseNull(trust);
    CFReleaseNull(certs);
    CFReleaseNull(anchors);
    CFReleaseNull(negIntSigLeaf);
    CFReleaseNull(rootAACA2);
    CFReleaseNull(policy);
}

static void rsa8k_tests(void)
{
    /* Test prt_forest_fi that have a 8k RSA key */
    const void *prt_forest_fi;
    isnt(prt_forest_fi = SecCertificateCreateWithBytes(NULL, prt_forest_fi_certificate,
                                                       sizeof(prt_forest_fi_certificate)), NULL, "create prt_forest_fi");
    CFArrayRef certs = NULL;
    isnt(certs = CFArrayCreate(NULL, &prt_forest_fi, 1, &kCFTypeArrayCallBacks), NULL, "failed to create cert array");
    SecPolicyRef policy = NULL;
    isnt(policy = SecPolicyCreateSSL(true, CFSTR("owa.prt-forest.fi")), NULL, "failed to create policy");
    SecTrustRef trust = NULL;
    ok_status(SecTrustCreateWithCertificates(certs, policy, &trust),
              "create trust for ip client owa.prt-forest.fi");
    CFDateRef date = CFDateCreate(NULL, 391578321.0);
    ok_status(SecTrustSetVerifyDate(trust, date),
              "set owa.prt-forest.fi trust date to May 2013");
    
    SecKeyRef pubkey = SecTrustCopyPublicKey(trust);
    isnt(pubkey, NULL, "pubkey returned");
    
    CFReleaseNull(certs);
    CFReleaseNull(prt_forest_fi);
    CFReleaseNull(policy);
    CFReleaseNull(trust);
    CFReleaseNull(pubkey);
    CFReleaseNull(date);
}

static void date_tests(void)
{
    /* Test long-lived cert chain that expires in 9999 */
    CFDateRef date = NULL;
    const void *leaf, *root;
    isnt(leaf = SecCertificateCreateWithBytes(NULL, longleaf, sizeof(longleaf)), NULL, "create leaf");
    isnt(root = SecCertificateCreateWithBytes(NULL, longroot, sizeof(longroot)), NULL, "create root");

    CFArrayRef certs = NULL;
    isnt(certs = CFArrayCreate(NULL, &leaf, 1, &kCFTypeArrayCallBacks), NULL, "failed to create cert array");
    CFArrayRef anchors = NULL;
    isnt(anchors = CFArrayCreate(NULL, &root, 1, &kCFTypeArrayCallBacks), NULL, "failed to create anchors array");

    SecPolicyRef policy = NULL;
    isnt(policy = SecPolicyCreateBasicX509(), NULL, "failed to create policy");
    SecTrustRef trust = NULL;
    SecTrustResultType trustResult;
    ok_status(SecTrustCreateWithCertificates(certs, policy, &trust), "create trust");
    if (!anchors) { goto errOut; }
    ok_status(SecTrustSetAnchorCertificates(trust, anchors), "set anchors");

    /* September 4, 2013 (prior to "notBefore" date of 2 April 2014, should fail) */
    isnt(date = CFDateCreate(NULL, 400000000), NULL, "failed to create date");
    if (!date) { goto errOut; }
    ok_status(SecTrustSetVerifyDate(trust, date), "set trust date to 23 Sep 2013");
    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate trust on 23 Sep 2013");
    is_status(trustResult, kSecTrustResultRecoverableTrustFailure, "expected kSecTrustResultRecoverableTrustFailure");
    CFReleaseNull(date);

    /* January 17, 2016 (recent date within validity period, should succeed) */
    isnt(date = CFDateCreate(NULL, 474747474), NULL, "failed to create date");
    if (!date) { goto errOut; }
    ok_status(SecTrustSetVerifyDate(trust, date), "set trust date to 17 Jan 2016");
    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate trust on 17 Jan 2016");
    is_status(trustResult, kSecTrustResultUnspecified, "expected kSecTrustResultUnspecified");
    CFReleaseNull(date);

    /* December 20, 9999 (far-future date within validity period, should succeed) */
    isnt(date = CFDateCreate(NULL, 252423000000), NULL, "failed to create date");
    if (!date) { goto errOut; }
    ok_status(SecTrustSetVerifyDate(trust, date), "set trust date to 20 Dec 9999");
    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate trust on 20 Dec 9999");
    is_status(trustResult, kSecTrustResultUnspecified, "expected kSecTrustResultUnspecified");
    CFReleaseNull(date);

    /* January 12, 10000 (after the "notAfter" date of 31 Dec 9999, should fail) */
    isnt(date = CFDateCreate(NULL, 252425000000), NULL, "failed to create date");
    if (!date) { goto errOut; }
    ok_status(SecTrustSetVerifyDate(trust, date), "set trust date to 12 Jan 10000");
    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate trust on 12 Jan 10000");
    is_status(trustResult, kSecTrustResultRecoverableTrustFailure, "expected kSecTrustResultRecoverableTrustFailure");
    CFReleaseNull(date);

errOut:
    CFReleaseNull(trust);
    CFReleaseNull(policy);
    CFReleaseNull(anchors);
    CFReleaseNull(certs);
    CFReleaseNull(root);
    CFReleaseNull(leaf);
}

static bool test_chain_of_three(uint8_t *cert0, size_t cert0len,
                                uint8_t *cert1, size_t cert1len,
                                uint8_t *root,  size_t rootlen,
                                bool should_succeed, CF_RETURNS_RETAINED CFStringRef *failureReason)
{
    bool ok = false;

    const void *secCert0, *secCert1, *secRoot;
    isnt(secCert0 = SecCertificateCreateWithBytes(NULL, cert0, cert0len), NULL, "create leaf");
    isnt(secCert1 = SecCertificateCreateWithBytes(NULL, cert1, cert1len), NULL, "create subCA");
    isnt(secRoot  = SecCertificateCreateWithBytes(NULL, root,  rootlen),  NULL, "create root");

    const void *v_certs[] = { secCert0, secCert1 };
    CFArrayRef certs = NULL;
    isnt(certs = CFArrayCreate(NULL, v_certs, sizeof(v_certs)/sizeof(*v_certs), &kCFTypeArrayCallBacks),
         NULL, "failed to create cert array");
    CFArrayRef anchors = NULL;
    isnt(anchors = CFArrayCreate(NULL, &secRoot, 1, &kCFTypeArrayCallBacks), NULL, "failed to create anchors array");

    SecPolicyRef policy = NULL;
    isnt(policy = SecPolicyCreateBasicX509(), NULL, "failed to create policy");
    CFDateRef date = NULL;
    isnt(date = CFDateCreate(NULL, 472100000.0), NULL, "failed to create date"); // 17 Dec 2015

    SecTrustRef trust = NULL;
    SecTrustResultType trustResult;
    ok_status(SecTrustCreateWithCertificates(certs, policy, &trust), "failed to create trust");
    if (!date) { goto errOut; }
    ok_status(SecTrustSetVerifyDate(trust, date), "failed to set verify date");
    if (!anchors) { goto errOut; }
    ok_status(SecTrustSetAnchorCertificates(trust, anchors), "failed to set anchors");

    ok_status(SecTrustEvaluate(trust, &trustResult), "evaluate chain");
    is(SecTrustGetCertificateCount(trust), 3, "expected chain of 3");
    bool did_succeed = (trustResult == kSecTrustResultUnspecified || trustResult == kSecTrustResultProceed);

    if (failureReason && should_succeed && !did_succeed) {
        *failureReason = SecTrustCopyFailureDescription(trust);
    } else if (failureReason && !should_succeed && did_succeed) {
        *failureReason = CFSTR("expected kSecTrustResultFatalTrustFailure");
    }

    if ((should_succeed && did_succeed) || (!should_succeed && !did_succeed)) {
        ok = true;
    }

errOut:
    CFReleaseNull(secCert0);
    CFReleaseNull(secCert1);
    CFReleaseNull(secRoot);
    CFReleaseNull(certs);
    CFReleaseNull(anchors);
    CFReleaseNull(date);
    CFReleaseNull(policy);
    CFReleaseNull(trust);

    return ok;
}

static void rsa_key_size_tests() {

    ok(test_chain_of_three(_leaf2048A, sizeof(_leaf2048A),_int2048A, sizeof(_int2048A), _root512, sizeof(_root512),
                           false, NULL), "SECURITY: failed to detect weak root");
    ok(test_chain_of_three(_leaf2048B, sizeof(_leaf2048B), _int512, sizeof(_int512), _root2048, sizeof(_root2048),
                           false, NULL), "SECURITY: failed to detect weak intermediate");
    ok(test_chain_of_three(_leaf512, sizeof(_leaf512), _int2048B, sizeof(_int2048B), _root2048, sizeof(_root2048),
                           false, NULL), "SECURITY: failed to detect weak leaf");

    CFStringRef failureReason = NULL;
    ok(test_chain_of_three(_leaf1024, sizeof(_leaf1024), _int2048B, sizeof(_int2048B), _root2048, sizeof(_root2048),
                           true, &failureReason), "REGRESSION: key size test 1024-bit leaf: %@", failureReason);
    CFReleaseNull(failureReason);
    ok(test_chain_of_three(_leaf2048C, sizeof(_leaf2048C), _int2048B, sizeof(_int2048B), _root2048, sizeof(_root2048),
                           true, &failureReason), "REGRESSION: key size test 2048-bit leaf: %@", failureReason);
    CFReleaseNull(failureReason);

}

static void ec_key_size_tests() {

    /* Because CoreCrypto does not support P128, we fail to chain if any CAs use weakly sized curves */
    ok(test_chain_of_three(_leaf128, sizeof(_leaf128), _int384B, sizeof(_int384B), _root384, sizeof(_root384),
                           false, NULL), "SECURITY: failed to detect weak leaf");

    CFStringRef failureReason = NULL;
    ok(test_chain_of_three(_leaf192, sizeof(_leaf192), _int384B, sizeof(_int384B), _root384, sizeof(_root384),
                           true, &failureReason), "REGRESSION: key size test 192-bit leaf: %@", failureReason);
    CFReleaseNull(failureReason);
    ok(test_chain_of_three(_leaf384C, sizeof(_leaf384C), _int384B, sizeof(_int384B), _root384, sizeof(_root384),
                           true, &failureReason), "REGRESSION: key size test 384-bit leaf: %@", failureReason);
    CFReleaseNull(failureReason);

}

static void test_input_certificates() {
    SecCertificateRef cert0 = NULL, cert1 = NULL;
    SecPolicyRef policy = NULL;
    SecTrustRef trust = NULL;
    CFArrayRef certificates = NULL;

    require(cert0 = SecCertificateCreateWithBytes(NULL, _c0, sizeof(_c0)), errOut);
    require(cert1 = SecCertificateCreateWithBytes(NULL, _c1, sizeof(_c1)), errOut);
    require(policy = SecPolicyCreateBasicX509(), errOut);
    require_noerr(SecTrustCreateWithCertificates(cert0, policy, &trust), errOut);

    ok_status(SecTrustCopyInputCertificates(trust, &certificates), "SecTrustCopyInputCertificates failed");
    is(CFArrayGetCount(certificates), 1, "got too many input certs back");
    is(CFArrayGetValueAtIndex(certificates, 0), cert0, "wrong input cert");
    CFReleaseNull(certificates);

    ok_status(SecTrustAddToInputCertificates(trust, cert1), "SecTrustAddToInputCertificates failed");
    ok_status(SecTrustCopyInputCertificates(trust, &certificates), "SecTrustCopyInputCertificates failed");
    is(CFArrayGetCount(certificates), 2, "got wrong number of input certs back");
    is(CFArrayGetValueAtIndex(certificates, 0), cert0, "wrong input cert0");
    is(CFArrayGetValueAtIndex(certificates, 1), cert1, "wrong input cert1");
    is(SecTrustGetCertificateCount(trust), 3, "output number of certs is 3");

errOut:
    CFReleaseNull(cert0);
    CFReleaseNull(cert1);
    CFReleaseNull(policy);
    CFReleaseNull(trust);
    CFReleaseNull(certificates);
}

static void test_async_trust() {
    SecCertificateRef cert0 = NULL, cert1 = NULL;
    SecPolicyRef policy = NULL;
    SecTrustRef trust = NULL;
    CFArrayRef certificates = NULL;
    CFDateRef date = NULL;

    require(cert0 = SecCertificateCreateWithBytes(NULL, _c0, sizeof(_c0)), errOut);
    require(cert1 = SecCertificateCreateWithBytes(NULL, _c1, sizeof(_c1)), errOut);
    const void *v_certs[] = {
        cert0,
        cert1
    };
    certificates = CFArrayCreate(NULL, v_certs,
                                 array_size(v_certs),
                                 &kCFTypeArrayCallBacks);

    require(policy = SecPolicyCreateBasicX509(), errOut);
    require_noerr(SecTrustCreateWithCertificates(certificates, policy, &trust), errOut);

    /* Jul 30 2014. */
    require(date = CFDateCreateForGregorianZuluMoment(NULL, 2014, 7, 30, 12, 0, 0), errOut);
    require_noerr(SecTrustSetVerifyDate(trust, date), errOut);

    /* This shouldn't crash. */
    ok_status(SecTrustEvaluateAsync(trust, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                                    ^(SecTrustRef  _Nonnull trustRef, SecTrustResultType trustResult) {
        if ((trustResult == kSecTrustResultProceed) ||
            (trustResult == kSecTrustResultUnspecified))
        {
            // Evaluation succeeded!
            SecKeyRef publicKey = SecTrustCopyPublicKey(trustRef);

            CFReleaseSafe(publicKey);

        } else if (trustResult == kSecTrustResultRecoverableTrustFailure) {
            // Evaluation failed, but may be able to recover . . .
        } else {
            // Evaluation failed
        }
    }), "evaluate trust asynchronously");
    CFReleaseNull(trust);

errOut:
    CFReleaseNull(cert0);
    CFReleaseNull(cert1);
    CFReleaseNull(policy);
    CFReleaseNull(certificates);
    CFReleaseNull(date);
}

static void test_expired_only() {
    SecCertificateRef cert0 = NULL, cert1 = NULL, cert2 = NULL;
    SecPolicyRef policy = NULL;
    SecTrustRef trust = NULL;
    CFArrayRef certificates = NULL, roots = NULL;
    CFDateRef date = NULL;

    require(cert0 = SecCertificateCreateWithBytes(NULL, _expired_badssl, sizeof(_expired_badssl)), errOut);
    require(cert1 = SecCertificateCreateWithBytes(NULL, _comodo_rsa_dvss, sizeof(_comodo_rsa_dvss)), errOut);
    require(cert2 = SecCertificateCreateWithBytes(NULL, _comodo_rsa_root, sizeof(_comodo_rsa_root)), errOut);

    const void *v_certs[] = {
        cert0,
        cert1
    };
    certificates = CFArrayCreate(NULL, v_certs,
                                 array_size(v_certs),
                                 &kCFTypeArrayCallBacks);

    const void *v_roots[] = {
        cert2
    };
    roots = CFArrayCreate(NULL, v_roots,
                          array_size(v_roots),
                          &kCFTypeArrayCallBacks);

    require(policy = SecPolicyCreateSSL(true, CFSTR("expired.badssl.com")), errOut);
    require_noerr(SecTrustCreateWithCertificates(certificates, policy, &trust), errOut);
    require_noerr(SecTrustSetAnchorCertificates(trust, roots), errOut);

    /* Mar 21 2017 (cert expired in 2015, so this will cause a validity error.) */
    require(date = CFDateCreateForGregorianZuluMoment(NULL, 2017, 3, 21, 12, 0, 0), errOut);
    require_noerr(SecTrustSetVerifyDate(trust, date), errOut);

    /* SecTrustIsExpiredOnly implicitly evaluates the trust */
    ok(SecTrustIsExpiredOnly(trust), "REGRESSION: has new error as well as expiration");

    CFReleaseNull(policy);
    require(policy = SecPolicyCreateSSL(true, CFSTR("expired.terriblessl.com")), errOut);
    require_noerr(SecTrustSetPolicies(trust, policy), errOut);
    /* expect a hostname mismatch as well as expiration */
    ok(!SecTrustIsExpiredOnly(trust), "REGRESSION: should have found multiple errors");

errOut:
    CFReleaseNull(trust);
    CFReleaseNull(cert0);
    CFReleaseNull(cert1);
    CFReleaseNull(cert2);
    CFReleaseNull(policy);
    CFReleaseNull(certificates);
    CFReleaseNull(roots);
    CFReleaseNull(date);
}

static void test_evaluate_with_error(void) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability-new"
    
    SecCertificateRef cert0 = NULL, cert1 = NULL, cert2 = NULL;
    SecPolicyRef policy = NULL;
    SecTrustRef trust = NULL;
    CFArrayRef certificates = NULL, roots = NULL;
    CFDateRef date = NULL, validDate = NULL;
    CFErrorRef error = NULL;

    require(cert0 = SecCertificateCreateWithBytes(NULL, _expired_badssl, sizeof(_expired_badssl)), errOut);
    require(cert1 = SecCertificateCreateWithBytes(NULL, _comodo_rsa_dvss, sizeof(_comodo_rsa_dvss)), errOut);
    require(cert2 = SecCertificateCreateWithBytes(NULL, _comodo_rsa_root, sizeof(_comodo_rsa_root)), errOut);

    const void *v_certs[] = {
        cert0,
        cert1
    };
    certificates = CFArrayCreate(NULL, v_certs,
                                 array_size(v_certs),
                                 &kCFTypeArrayCallBacks);

    const void *v_roots[] = {
        cert2
    };
    roots = CFArrayCreate(NULL, v_roots,
                          array_size(v_roots),
                          &kCFTypeArrayCallBacks);

    require(policy = SecPolicyCreateSSL(true, CFSTR("expired.badssl.com")), errOut);
    require_noerr(SecTrustCreateWithCertificates(certificates, policy, &trust), errOut);
    require_noerr(SecTrustSetAnchorCertificates(trust, roots), errOut);

    /* April 10 2015 (cert expired in 2015) */
    require(validDate = CFDateCreateForGregorianZuluMoment(NULL, 2015, 4, 10, 12, 0, 0), errOut);
    require_noerr(SecTrustSetVerifyDate(trust, validDate), errOut);

    is(SecTrustEvaluateWithError(trust, &error), true, "wrong result for valid cert");
    is(error, NULL, "set error for passing trust evaluation");
    CFReleaseNull(error);

    /* Mar 21 2017 (cert expired in 2015, so this will cause a validity error.) */
    require(date = CFDateCreateForGregorianZuluMoment(NULL, 2017, 3, 21, 12, 0, 0), errOut);
    require_noerr(SecTrustSetVerifyDate(trust, date), errOut);

    /* expect expiration error */
    is(SecTrustEvaluateWithError(trust, &error), false, "wrong result for expired cert");
    isnt(error, NULL, "failed to set error for failing trust evaluation");
    is(CFErrorGetCode(error), errSecCertificateExpired, "Got wrong error code for evaluation");
    CFReleaseNull(error);

    CFReleaseNull(policy);
    require(policy = SecPolicyCreateSSL(true, CFSTR("expired.terriblessl.com")), errOut);
    require_noerr(SecTrustSetPolicies(trust, policy), errOut);

    /* expect a hostname mismatch as well as expiration; hostname mismatch must be a higher priority */
    is(SecTrustEvaluateWithError(trust, &error), false, "wrong result for expired cert with hostname mismatch");
    isnt(error, NULL, "failed to set error for failing trust evaluation");
    is(CFErrorGetCode(error), errSecHostNameMismatch, "Got wrong error code for evaluation");
    CFReleaseNull(error);

    /* expect only a hostname mismatch*/
    require_noerr(SecTrustSetVerifyDate(trust, validDate), errOut);
    is(SecTrustEvaluateWithError(trust, &error), false, "wrong result for valid cert with hostname mismatch");
    isnt(error, NULL, "failed to set error for failing trust evaluation");
    is(CFErrorGetCode(error), errSecHostNameMismatch, "Got wrong error code for evaluation");
    CFReleaseNull(error);

    /* pinning failure */
    CFReleaseNull(policy);
    require(policy = SecPolicyCreateAppleSSLPinned(CFSTR("test"), CFSTR("expired.badssl.com"),
                                                   NULL, CFSTR("1.2.840.113635.100.6.27.1")), errOut);
    require_noerr(SecTrustSetPolicies(trust, policy), errOut);

    is(SecTrustEvaluateWithError(trust, &error), false, "wrong result for valid cert with pinning failure");
    isnt(error, NULL, "failed to set error for failing trust evaluation");
    CFIndex errorCode = CFErrorGetCode(error);
    // failed checks: AnchorApple, LeafMarkerOid, or IntermediateMarkerOid
    ok(errorCode == errSecMissingRequiredExtension || errorCode == errSecInvalidRoot, "Got wrong error code for evaluation");
    CFReleaseNull(error);

    /* trust nothing, trust errors higher priority than hostname mismatch */
    CFReleaseNull(policy);
    require(policy = SecPolicyCreateSSL(true, CFSTR("expired.terriblessl.com")), errOut);
    require_noerr(SecTrustSetPolicies(trust, policy), errOut);

    CFReleaseNull(roots);
    roots = CFArrayCreate(NULL, NULL, 0, &kCFTypeArrayCallBacks);
    require_noerr(SecTrustSetAnchorCertificates(trust, roots), errOut);
    is(SecTrustEvaluateWithError(trust, &error), false, "wrong result for expired cert with hostname mismatch");
    isnt(error, NULL, "failed to set error for failing trust evaluation");
    is(CFErrorGetCode(error), errSecNotTrusted, "Got wrong error code for evaluation");
    CFReleaseNull(error);

errOut:
    CFReleaseNull(trust);
    CFReleaseNull(cert0);
    CFReleaseNull(cert1);
    CFReleaseNull(cert2);
    CFReleaseNull(policy);
    CFReleaseNull(certificates);
    CFReleaseNull(roots);
    CFReleaseNull(date);
    CFReleaseNull(validDate);
    CFReleaseNull(error);

#pragma clang diagnostic pop
}

static void test_optional_policy_check(void) {
    SecCertificateRef cert0 = NULL, cert1 = NULL, root = NULL;
    SecTrustRef trust = NULL;
    SecPolicyRef policy = NULL;
    CFArrayRef certs = NULL, anchors = NULL;
    CFDateRef date = NULL;

    require_action(cert0 = SecCertificateCreateWithBytes(NULL, _leaf384C, sizeof(_leaf384C)), errOut,
                   fail("unable to create cert"));
    require_action(cert1 = SecCertificateCreateWithBytes(NULL, _int384B, sizeof(_int384B)), errOut,
                   fail("unable to create cert"));
    require_action(root = SecCertificateCreateWithBytes(NULL, _root384, sizeof(_root384)), errOut,
                   fail("unable to create cert"));

    const void *v_certs[] = { cert0, cert1 };
    require_action(certs = CFArrayCreate(NULL, v_certs, array_size(v_certs), &kCFTypeArrayCallBacks), errOut,
                   fail("unable to create array"));
    require_action(anchors = CFArrayCreate(NULL, (const void **)&root, 1, &kCFTypeArrayCallBacks), errOut,
                   fail("unable to create anchors array"));
    require_action(date = CFDateCreate(NULL, 472100000.0), errOut, fail("unable to create date"));

    require_action(policy = SecPolicyCreateBasicX509(), errOut, fail("unable to create policy"));
    SecPolicySetOptionsValue(policy, CFSTR("not-a-policy-check"), kCFBooleanTrue);

    ok_status(SecTrustCreateWithCertificates(certs, policy, &trust), "failed to create trust");
    require_noerr_action(SecTrustSetAnchorCertificates(trust, anchors), errOut,
                         fail("unable to set anchors"));
    require_noerr_action(SecTrustSetVerifyDate(trust, date), errOut, fail("unable to set verify date"));

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability-new"
#if NDEBUG
    ok(SecTrustEvaluateWithError(trust, NULL), "Trust evaluation failed");
#else
    is(SecTrustEvaluateWithError(trust, NULL), false, "Expect failure in Debug config");
#endif
#pragma clang diagnostic pop

errOut:
    CFReleaseNull(cert0);
    CFReleaseNull(cert1);
    CFReleaseNull(root);
    CFReleaseNull(certs);
    CFReleaseNull(anchors);
    CFReleaseNull(date);
    CFReleaseNull(policy);
    CFReleaseNull(trust);
}

static void test_serialization(void) {
    SecCertificateRef cert0 = NULL, cert1 = NULL, root = NULL;
    SecTrustRef trust = NULL, deserializedTrust = NULL;
    SecPolicyRef policy = NULL;
    CFArrayRef certs = NULL, anchors = NULL, deserializedCerts = NULL;
    CFDateRef date = NULL;
    CFDataRef serializedTrust = NULL;
    CFErrorRef error = NULL;

    require_action(cert0 = SecCertificateCreateWithBytes(NULL, _leaf384C, sizeof(_leaf384C)), errOut,
                   fail("unable to create cert"));
    require_action(cert1 = SecCertificateCreateWithBytes(NULL, _int384B, sizeof(_int384B)), errOut,
                   fail("unable to create cert"));
    require_action(root = SecCertificateCreateWithBytes(NULL, _root384, sizeof(_root384)), errOut,
                   fail("unable to create cert"));

    const void *v_certs[] = { cert0, cert1 };
    require_action(certs = CFArrayCreate(NULL, v_certs, array_size(v_certs), &kCFTypeArrayCallBacks), errOut,
                   fail("unable to create array"));
    require_action(anchors = CFArrayCreate(NULL, (const void **)&root, 1, &kCFTypeArrayCallBacks), errOut,
                   fail("unable to create anchors array"));
    require_action(date = CFDateCreate(NULL, 472100000.0), errOut, fail("unable to create date"));

    require_action(policy = SecPolicyCreateBasicX509(), errOut, fail("unable to create policy"));

    ok_status(SecTrustCreateWithCertificates(certs, policy, &trust), "failed to create trust");
    require_noerr_action(SecTrustSetAnchorCertificates(trust, anchors), errOut,
                         fail("unable to set anchors"));
    require_noerr_action(SecTrustSetVerifyDate(trust, date), errOut, fail("unable to set verify date"));

    ok(serializedTrust = SecTrustSerialize(trust, NULL), "failed to serialize trust");
    ok(deserializedTrust = SecTrustDeserialize(serializedTrust, NULL), "Failed to deserialize trust");
    CFReleaseNull(serializedTrust);

    require_noerr_action(SecTrustCopyCustomAnchorCertificates(deserializedTrust, &deserializedCerts), errOut,
                         fail("unable to get anchors from deserialized trust"));
    ok(CFEqual(anchors, deserializedCerts), "Failed to get the same anchors after serialization/deserialization");
    CFReleaseNull(deserializedCerts);

    require_noerr_action(SecTrustCopyInputCertificates(trust, &deserializedCerts), errOut,
                         fail("unable to get input certificates from deserialized trust"));
    ok(CFEqual(certs, deserializedCerts), "Failed to get same input certificates after serialization/deserialization");
    CFReleaseNull(deserializedCerts);

    /* correct API behavior */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnonnull"
    is(SecTrustSerialize(NULL, &error), NULL, "serialize succeeded with null input");
    is(CFErrorGetCode(error), errSecParam, "Incorrect error code for bad serialization input");
    CFReleaseNull(error);
    is(SecTrustDeserialize(NULL, &error), NULL, "deserialize succeeded with null input");
    is(CFErrorGetCode(error), errSecParam, "Incorrect error code for bad deserialization input");
    CFReleaseNull(error);
#pragma clang diagnostic pop

errOut:
    CFReleaseNull(cert0);
    CFReleaseNull(cert1);
    CFReleaseNull(root);
    CFReleaseNull(certs);
    CFReleaseNull(anchors);
    CFReleaseNull(date);
    CFReleaseNull(policy);
    CFReleaseNull(trust);
    CFReleaseNull(deserializedTrust);
}

static void test_tls_analytics_report(void)
{
    xpc_object_t metric = xpc_dictionary_create(NULL, NULL, 0);
    ok(metric != NULL);

    const char *TLS_METRIC_PROCESS_IDENTIFIER = "process";
    const char *TLS_METRIC_CIPHERSUITE = "cipher_name";
    const char *TLS_METRIC_PROTOCOL_VERSION = "version";
    const char *TLS_METRIC_SESSION_RESUMED = "resumed";

    xpc_dictionary_set_string(metric, TLS_METRIC_PROCESS_IDENTIFIER, "super awesome unit tester");
    xpc_dictionary_set_uint64(metric, TLS_METRIC_CIPHERSUITE, 0x0304);
    xpc_dictionary_set_uint64(metric, TLS_METRIC_PROTOCOL_VERSION, 0x0304);
    xpc_dictionary_set_bool(metric, TLS_METRIC_SESSION_RESUMED, false);
    // ... TLS would fill in the rest

    // Invoke the callback
    CFErrorRef error = NULL;
    bool reported = SecTrustReportTLSAnalytics(CFSTR("TLSConnectionEvent"), metric, &error);
    ok(reported, "Failed to report analytics with error %@", error);
    xpc_release(metric);
}

static void test_weak_signature(void) {
    SecCertificateRef md5_root = NULL, md5_leaf = NULL, sha256_root = NULL;
    SecPolicyRef policy = NULL;
    SecTrustRef trust = NULL;
    CFArrayRef certs = NULL, anchors = NULL;
    CFDateRef verifyDate = NULL;
    CFErrorRef error = NULL;

    require_action(md5_root = SecCertificateCreateWithBytes(NULL, _md5_root, sizeof(_md5_root)), errOut,
                   fail("failed to create md5 root cert"));
    require_action(md5_leaf = SecCertificateCreateWithBytes(NULL, _md5_leaf, sizeof(_md5_leaf)), errOut,
                   fail("failed to create md5 leaf cert"));
    require_action(sha256_root = SecCertificateCreateWithBytes(NULL, _sha256_root, sizeof(_sha256_root)), errOut,
                   fail("failed to create sha256 root cert"));

    require_action(certs = CFArrayCreate(NULL, (const void **)&md5_root, 1, &kCFTypeArrayCallBacks), errOut,
                   fail("failed to create certs array"));
    require_action(anchors = CFArrayCreate(NULL, (const void **)&md5_root, 1, &kCFTypeArrayCallBacks), errOut,
                   fail("failed to create anchors array"));
    require_action(policy = SecPolicyCreateBasicX509(), errOut, fail("failed to make policy"));
    require_action(verifyDate = CFDateCreate(NULL, 550600000), errOut, fail("failed to make verification date")); // June 13, 2018

    /* Test self-signed MD5 cert. Should work since cert is a trusted anchor - rdar://39152516 */
    require_noerr_action(SecTrustCreateWithCertificates(certs, policy, &trust), errOut,
                         fail("failed to create trust object"));
    require_noerr_action(SecTrustSetAnchorCertificates(trust, anchors), errOut,
                         fail("faild to set anchors"));
    require_noerr_action(SecTrustSetVerifyDate(trust, verifyDate), errOut,
                         fail("failed to set verify date"));
    ok(SecTrustEvaluateWithError(trust, &error), "self-signed MD5 cert failed");
    is(error, NULL, "got a trust error for self-signed MD5 cert: %@", error);

    /* clean up and set up for next test */
    CFReleaseNull(error);
    CFReleaseNull(trust);
    CFReleaseNull(anchors);
    CFReleaseNull(certs);

    require_action(certs = CFArrayCreate(NULL, (const void **)&md5_leaf, 1, &kCFTypeArrayCallBacks), errOut,
                   fail("failed to create certs array"));
    require_action(anchors = CFArrayCreate(NULL, (const void **)&sha256_root, 1, &kCFTypeArrayCallBacks), errOut,
                   fail("failed to create anchors array"));


    /* Test non-self-signed MD5 cert. Should fail. */
    require_noerr_action(SecTrustCreateWithCertificates(certs, policy, &trust), errOut,
                         fail("failed to create trust object"));
    require_noerr_action(SecTrustSetAnchorCertificates(trust, anchors), errOut,
                         fail("faild to set anchors"));
    require_noerr_action(SecTrustSetVerifyDate(trust, verifyDate), errOut,
                         fail("failed to set verify date"));
    is(SecTrustEvaluateWithError(trust, &error), false, "non-self-signed MD5 cert succeeded");
    if (error) {
        is(CFErrorGetCode(error), errSecInvalidDigestAlgorithm, "got wrong error code for MD5 leaf cert, got %ld, expected %d",
           (long)CFErrorGetCode(error), errSecInvalidDigestAlgorithm);
    } else {
        fail("expected trust evaluation to fail and it did not.");
    }

errOut:
    CFReleaseNull(md5_root);
    CFReleaseNull(md5_leaf);
    CFReleaseNull(sha256_root);
    CFReleaseNull(certs);
    CFReleaseNull(anchors);
    CFReleaseNull(policy);
    CFReleaseNull(verifyDate);
    CFReleaseNull(trust);
    CFReleaseNull(error);
}

int si_20_sectrust(int argc, char *const *argv)
{
#if TARGET_OS_IPHONE
    plan_tests(101+9+(8*13)+9+1+2+17+2+9+2+4);
#else
    plan_tests(97+9+(8*13)+9+1+2+2+17+2+9+2+4);
#endif

    basic_tests();
    negative_integer_tests();
    rsa8k_tests();
    date_tests();
    rsa_key_size_tests();
    ec_key_size_tests();
    test_input_certificates();
    test_async_trust();
    test_expired_only();
    test_evaluate_with_error();
    test_optional_policy_check();
    test_serialization();
    test_tls_analytics_report();
    test_weak_signature();

    return 0;
}
