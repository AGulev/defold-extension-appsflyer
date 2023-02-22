#if defined(DM_PLATFORM_IOS)

#import <Foundation/Foundation.h>
#if __has_include(<AppsFlyerLib/AppsFlyerLib.h>)
#import <AppsFlyerLib/AppsFlyerLib.h>
#else
#import "AppsFlyerLib.h"
#endif
#include "appsflyer_private.h"
#import "appsflyer_callback_private.h"
#import "AppsFlyerAttribution.h"

NS_ASSUME_NONNULL_BEGIN

@interface DEFAFSDKDelegate : NSObject <AppsFlyerLibDelegate, AppsFlyerDeepLinkDelegate>
@end

NS_ASSUME_NONNULL_END

#endif
