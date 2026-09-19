#if defined(DM_PLATFORM_IOS)

#import "DEFAFSDKDelegate.h"

@implementation DEFAFSDKDelegate

- (void)didResolveDeepLink:(AppsFlyerDeepLinkResult *)result {
    NSMutableDictionary* payload = [NSMutableDictionary dictionary];
    switch (result.status) {
        case AFSDKDeepLinkResultStatusFound:
            payload[@"status"] = @"FOUND";
            payload[@"deep_link"] = result.deepLink.clickEvent;
            payload[@"is_deferred"] = @(result.deepLink.isDeferred);
            break;
        case AFSDKDeepLinkResultStatusNotFound:
            payload[@"status"] = @"NOT_FOUND";
            break;
        case AFSDKDeepLinkResultStatusFailure:
            payload[@"status"] = @"ERROR";
            payload[@"error"] = result.error.localizedDescription ?: @"Deep link resolution failed";
            break;
    }
    NSData* data = [NSJSONSerialization dataWithJSONObject:payload options:0 error:nil];
    NSString* json = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] autorelease];
    dmAppsflyer::AddToQueueCallback(dmAppsflyer::DEEP_LINK_RESULT, [json UTF8String]);
}

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
