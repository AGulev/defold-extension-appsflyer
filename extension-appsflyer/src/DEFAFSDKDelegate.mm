#if defined(DM_PLATFORM_IOS)

#import "DEFAFSDKDelegate.h"


@implementation DEFAFSDKDelegate

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
- (void)onConversionDataSuccess:(NSDictionary *)conversionInfo {
    _onConversionDataSuccess([DEFAFSDKDelegate dictionaryByReplacingNullsWithStrings:conversionInfo]);
}

/**
 Any errors that occurred during the conversion request.
 */
- (void)onConversionDataFail:(NSError *)error {
    _onConversionDataFail(error.localizedDescription);
}

/**
 `attributionData` contains information about OneLink, deeplink.
 */
- (void)onAppOpenAttribution:(NSDictionary *)attributionData {
    _onAppOpenAttribution(attributionData);
}

/**
 Any errors that occurred during the attribution request.
 */
- (void)onAppOpenAttributionFailure:(NSError *)error {
    _onAppOpenAttributionFailure(error.localizedDescription);
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
