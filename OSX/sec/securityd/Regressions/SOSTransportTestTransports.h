#ifndef SEC_SOSTransportTestTransports_h
#define SEC_SOSTransportTestTransports_h

#import "Security/SecureObjectSync/SOSTransportCircleKVS.h"
#import "Security/SecureObjectSync/SOSTransportMessageKVS.h"
#import "Security/SecureObjectSync/SOSTransportMessageIDS.h"

extern CFMutableArrayRef key_transports;
extern CFMutableArrayRef circle_transports;
extern CFMutableArrayRef message_transports;

void SOSAccountUpdateTestTransports(SOSAccount* account, CFDictionaryRef gestalt);

@interface SOSMessageIDSTest : SOSMessageIDS

@property (nonatomic) CFMutableDictionaryRef changes;
@property (nonatomic) CFStringRef   accountName;

-(SOSMessageIDSTest*) initWithAccount:(SOSAccount*)acct andAccountName:(CFStringRef) aN andCircleName:(CFStringRef) cN err:(CFErrorRef *)error;
-(CFIndex) SOSTransportMessageGetTransportType;
-(CFStringRef) SOSTransportMessageGetCircleName;
-(CFTypeRef) SOSTransportMessageGetEngine;
-(SOSAccount*) SOSTransportMessageGetAccount;
void SOSTransportMessageIDSTestSetName(SOSMessageIDSTest* transport, CFStringRef acctName);
-(bool) SOSTransportMessageSendMessages:(SOSMessageIDSTest*) transport pm:(CFDictionaryRef) peer_messages err:(CFErrorRef *)error;
-(bool) SOSTransportMessageIDSGetIDSDeviceID:(SOSAccount*)acct;
-(CFDictionaryRef)CF_RETURNS_RETAINED SOSTransportMessageHandlePeerMessageReturnsHandledCopy:(SOSMessageIDSTest*) transport peerMessages:(CFMutableDictionaryRef)message err:(CFErrorRef *)error;
-(HandleIDSMessageReason) SOSTransportMessageIDSHandleMessage:(SOSAccount*)acct m:(CFDictionaryRef) message err:(CFErrorRef *)error;

@end

@interface CKKeyParameterTest : CKKeyParameter

@property (nonatomic) CFMutableDictionaryRef changes;
@property (nonatomic) CFStringRef name;
@property (nonatomic) CFStringRef circleName;
-(id) initWithAccount:(SOSAccount*) acct andName:(CFStringRef) name andCircleName:(CFStringRef) circleName;
-(bool) SOSTransportKeyParameterPublishCloudParameters:(CKKeyParameterTest*) transport data:(CFDataRef)newParameters err:(CFErrorRef*) error;
-(bool) SOSTransportKeyParameterHandleKeyParameterChanges:(CKKeyParameter*) transport  data:(CFDataRef) data err:(CFErrorRef) error;

@end

@interface SOSMessageKVSTest : SOSMessageKVS
@property (nonatomic) CFMutableDictionaryRef changes;
@property (nonatomic) CFStringRef name;

-(id) initWithAccount:(SOSAccount*) acct andName:(CFStringRef) name andCircleName:(CFStringRef) circleName;
-(bool) SOSTransportMessageSendMessages:(SOSMessageKVSTest*) transport pm:(CFDictionaryRef) peer_messages err:(CFErrorRef *)error;
-(CFIndex) SOSTransportMessageGetTransportType;
-(CFStringRef) SOSTransportMessageGetCircleName;
-(CFTypeRef) SOSTransportMessageGetEngine;
-(SOSAccount*) SOSTransportMessageGetAccount;
@end

@interface SOSCircleStorageTransportTest : SOSKVSCircleStorageTransport
{
    NSString *accountName;
}
@property (nonatomic) NSString *accountName;

-(id) init;
-(id) initWithAccount:(SOSAccount*)account andWithAccountName:(CFStringRef)accountName andCircleName:(CFStringRef)circleName;

bool SOSTransportCircleTestRemovePendingChange(SOSCircleStorageTransportTest* transport,  CFStringRef circleName, CFErrorRef *error);
CFStringRef SOSTransportCircleTestGetName(SOSCircleStorageTransportTest* transport);
bool SOSAccountEnsureFactoryCirclesTest(SOSAccount* a, CFStringRef accountName);
SOSAccount* SOSTransportCircleTestGetAccount(SOSCircleStorageTransportTest* transport);
void SOSTransportCircleTestClearChanges(SOSCircleStorageTransportTest* transport);
void SOSTransportCircleTestSetName(SOSCircleStorageTransportTest* transport, CFStringRef accountName);
bool SOSAccountInflateTestTransportsForCircle(SOSAccount* account, CFStringRef circleName, CFStringRef accountName, CFErrorRef *error);
-(void) SOSTransportCircleTestAddBulkToChanges:(CFDictionaryRef) updates;
-(void) testAddToChanges:(CFStringRef) message_key data:(CFDataRef)message_data;
-(CFMutableDictionaryRef) SOSTransportCircleTestGetChanges;
@end

void SOSTransportMessageIDSTestSetName(SOSMessageIDSTest* transport, CFStringRef n);
void SOSTransportMessageTestClearChanges(SOSMessageKVSTest* transport);

void SOSTransportMessageKVSTestSetName(SOSMessageKVSTest* transport, CFStringRef n);
CFMutableDictionaryRef SOSTransportMessageKVSTestGetChanges(SOSMessageKVSTest* transport);
CFStringRef SOSTransportMessageKVSTestGetName(SOSMessageKVSTest* transport);

CFStringRef SOSTransportKeyParameterTestGetName(CKKeyParameterTest* transport);
CFMutableDictionaryRef SOSTransportKeyParameterTestGetChanges(CKKeyParameterTest* transport);
void SOSTransportKeyParameterTestSetName(CKKeyParameterTest* transport, CFStringRef accountName);
void SOSTransportKeyParameterTestClearChanges(CKKeyParameterTest* transport);
SOSAccount* SOSTransportKeyParameterTestGetAccount(CKKeyParameterTest* transport);
SOSAccount* SOSTransportMessageKVSTestGetAccount(SOSMessageKVSTest* transport);
void SOSTransportMessageIDSTestClearChanges(SOSMessageIDSTest* transport);

CFMutableDictionaryRef SOSTransportMessageIDSTestGetChanges(SOSMessageIDSTest* transport);
SOSAccount* SOSTransportMessageIDSTestGetAccount(SOSMessageIDSTest* transport);
CFStringRef SOSTransportMessageIDSTestGetName(SOSMessageIDSTest* transport);
#endif
