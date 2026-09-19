#if defined(DM_PLATFORM_IOS)

#import "DEFAFSDKDelegate.h"

@implementation DEFAFSDKDelegate

- (void)onConversionDataSuccess:(NSDictionary *)installData {
    NSData* data = [NSJSONSerialization dataWithJSONObject:installData options:0 error:nil];
    NSString* json = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] autorelease];
    dmAppsflyer::AddToQueueCallback(dmAppsflyer::CONVERSION_DATA_SUCCESS, [json UTF8String]);
}

- (void)onConversionDataFail:(NSError *)error {
    NSDictionary* payload = @{@"error": error.localizedDescription, @"code": @(error.code)};
    NSData* data = [NSJSONSerialization dataWithJSONObject:payload options:0 error:nil];
    NSString* json = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] autorelease];
    dmAppsflyer::AddToQueueCallback(dmAppsflyer::CONVERSION_DATA_FAIL, [json UTF8String]);
}

@end

#endif
