#if defined(DM_PLATFORM_IOS)

#import "DEFAFSDKDelegate.h"


@implementation DEFAFSDKDelegate

void ForwardIOSEventAppsf(int type, NSString * json)
{
    const char *JsonCString = [json UTF8String];
    AddToQueueCallback((dmAppsflyer::MessageId)type, JsonCString);
}

- (instancetype)init {
    self = [super init];
    if (self) {
        
    }
    return self;
}

/**
 `installData` contains information about install.
 Organic/non-organic, etc.
 */
- (void)onConversionDataSuccess:(NSDictionary *)installData {
    NSLog(@"AppsFlyer onConversionDataSuccess");
    NSData *data = [NSJSONSerialization dataWithJSONObject: installData options: 0 error: nil];
    NSString *serializedParameters = [[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding];
    ForwardIOSEventAppsf(1, serializedParameters);
}

/**
 Any errors that occurred during the conversion request.
 */
- (void)onConversionDataFail:(NSError *)error {
    NSLog(@"AppsFlyer conversion data error %@", error);
}

/**
 `attributionData` contains information about OneLink, deeplink.
 */
- (void)onAppOpenAttribution:(NSDictionary *)attributionData {

    NSLog(@"AppsFlyer onAppOpenAttribution");
    NSData *data = [NSJSONSerialization dataWithJSONObject: attributionData options: 0 error: nil];
    NSString *serializedParameters = [[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding];
    ForwardIOSEventAppsf(3, serializedParameters);
}

/**
 Any errors that occurred during the attribution request.
 */
- (void)onAppOpenAttributionFailure:(NSError *)error {
    NSLog(@"Appsflyer onAppOpenAttributionFailure %@", error);
}

- (void)didResolveDeepLink:(AppsFlyerDeepLinkResult *)result{
    NSLog(@"Appsflyer didResolveDeepLink");
}

+ (NSDictionary *)dictionaryByReplacingNullsWithStrings:(NSDictionary *)dict {
   const NSMutableDictionary *replaced = [dict mutableCopy];
   const id nul = [NSNull null];
   const NSString *blank = @"";

   for (NSString *key in dict) {
      const id object = [dict objectForKey:key];
      if (object == nul) {
         // pointer comparison is way faster than -isKindOfClass:
         // since [NSNull null] is a singleton, they'll all point to the same
         // location in memory.
         [replaced setObject:blank
                      forKey:key];
      }
   }
   return [replaced copy];
}

@end

#endif
