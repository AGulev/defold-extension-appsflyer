#if defined(DM_PLATFORM_IOS)

#include <dmsdk/sdk.h>
#import <AppsFlyerLib/AppsFlyerLib.h>
#include "appsflyer_private.h"
#import "appsflyer_callback_private.h"
#import "DEFAFSDKDelegate.h"
#import "AppsFlyerAttribution.h"
#import "AppsflyerAppDelegate.h"

namespace dmAppsflyer {

struct AppsflyerAppDelegateRegister
{
    AppsflyerAppDelegate* m_Delegate;

    AppsflyerAppDelegateRegister() {
        m_Delegate = [[AppsflyerAppDelegate alloc] init];
        dmExtension::RegisteriOSUIApplicationDelegate(m_Delegate);
    }

    ~AppsflyerAppDelegateRegister() {
        dmExtension::UnregisteriOSUIApplicationDelegate(m_Delegate);
        [m_Delegate release];
    }
};

AppsflyerAppDelegateRegister g_appDelegate;
// SDK delegates are weak. Retain ours until extension shutdown.
static DEFAFSDKDelegate* g_sdkDelegate = nil;
// Access session state only on the main thread.
static bool g_startRequested = false;
static bool g_sessionReady = false;

static void OnMainThread(dispatch_block_t block)
{
    if ([NSThread isMainThread])
        block();
    else
        dispatch_async(dispatch_get_main_queue(), block);
}

static void ReportRequest(NSString* eventName, NSError* error)
{
    NSMutableDictionary* data = [NSMutableDictionary dictionary];
    if (eventName)
        data[@"event_name"] = eventName;
    if (error) {
        data[@"error"] = error.localizedDescription;
        data[@"code"] = @(error.code);
    }
    MessageId type = eventName ? (error ? EVENT_FAIL : EVENT_SUCCESS) : (error ? START_FAIL : START_SUCCESS);
    NSData* jsonData = [NSJSONSerialization dataWithJSONObject:data options:0 error:nil];
    NSString* json = [[[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding] autorelease];
    AddToQueueCallback(type, [json UTF8String]);
}

static void StartIfReady()
{
    if (g_sdkDelegate && g_startRequested && g_sessionReady && ![AppsFlyerLib shared].isStopped && [[AppsFlyerLib shared] isSessionReady]) {
        g_sessionReady = false;
        [[AppsFlyerLib shared] startWithCompletionHandler:^(NSDictionary* response, NSError* error) {
            ReportRequest(nil, error);
        }];
    }
}

void Initialize_Ext()
{
}

void Finalize_Ext()
{
    OnMainThread(^{
        g_startRequested = false;
        g_sessionReady = false;
        [[AppsFlyerLib shared] unregisterSessionReadyListener];
        [AppsFlyerLib shared].delegate = nil;
        [AppsFlyerAttribution shared].isBridgeReady = NO;
        [g_sdkDelegate release];
        g_sdkDelegate = nil;
    });
}

void InitializeSDK(const char* key, const char* appleAppID)
{
    NSString* devKey = [NSString stringWithUTF8String:key];
    NSString* appID = [NSString stringWithUTF8String:appleAppID];
    OnMainThread(^{
        g_sdkDelegate = [[DEFAFSDKDelegate alloc] init];
        [AppsFlyerLib shared].delegate = g_sdkDelegate;
        [[AppsFlyerLib shared] initWithDevKey:devKey appleAppId:appID];
        [[AppsFlyerLib shared] handleLaunchOptions:[AppsFlyerAttribution shared].launchOptions];
        [AppsFlyerAttribution shared].launchOptions = nil;
        [AppsFlyerAttribution shared].isBridgeReady = YES;
        [[NSNotificationCenter defaultCenter] postNotificationName:AF_BRIDGE_SET object:[AppsFlyerAttribution shared]];
        [[AppsFlyerLib shared] registerSessionReadyListener:^{
            g_sessionReady = true;
            StartIfReady();
        }];
    });
}

void StartSDK()
{
    OnMainThread(^{
        g_startRequested = true;
        StartIfReady();
    });
}

void SetDebugLog(bool is_debug)
{
    OnMainThread(^{ [AppsFlyerLib shared].isDebug = is_debug; });
}

void LogEvent(const char* eventName, dmArray<TrackData>* trackData)
{
    @autoreleasepool {
        NSMutableDictionary* values = [NSMutableDictionary dictionary];
        NSString* event = [NSString stringWithUTF8String:eventName];
        for (uint32_t i = 0; i != trackData->Size(); i++) {
            TrackData data = (*trackData)[i];
            values[[NSString stringWithUTF8String:data.key]] = [NSString stringWithUTF8String:data.value];
        }
        OnMainThread(^{
            [[AppsFlyerLib shared] logEventWithEventName:event eventValues:values completionHandler:^(NSDictionary* response, NSError* error) {
                ReportRequest(event, error);
            }];
        });
    }
}

void SetCustomerUserId(const char* userId)
{
    NSString* value = [NSString stringWithUTF8String:userId];
    OnMainThread(^{ [AppsFlyerLib shared].customerUserID = value; });
}

static NSNumber* ConsentValue(int value)
{
    return value < 0 ? nil : [NSNumber numberWithBool:value != 0];
}

void SetConsentData(int gdpr, int dataUsage, int adsPersonalization, int adStorage)
{
    OnMainThread(^{
        AppsFlyerConsent* consent = [[AppsFlyerConsent alloc]
            initWithIsUserSubjectToGDPR:ConsentValue(gdpr)
            hasConsentForDataUsage:ConsentValue(dataUsage)
            hasConsentForAdsPersonalization:ConsentValue(adsPersonalization)
            hasConsentForAdStorage:ConsentValue(adStorage)];
        [[AppsFlyerLib shared] setConsentData:consent];
        [consent release];
    });
}

void EnableTCFDataCollection(bool enabled)
{
    OnMainThread(^{ [[AppsFlyerLib shared] enableTCFDataCollection:enabled]; });
}

void AnonymizeUser(bool enabled)
{
    OnMainThread(^{ [AppsFlyerLib shared].anonymizeUser = enabled; });
}

void StopSDK(bool stopped)
{
    OnMainThread(^{
        if (stopped) g_startRequested = false;
        [AppsFlyerLib shared].isStopped = stopped;
        if (!stopped) g_sessionReady = [[AppsFlyerLib shared] isSessionReady];
    });
}

bool IsStopped()
{
    return [AppsFlyerLib shared].isStopped;
}

void SetCurrencyCode(const char* currency)
{
    NSString* value = [NSString stringWithUTF8String:currency];
    OnMainThread(^{ [AppsFlyerLib shared].currencyCode = value; });
}

void SetSharingFilterForPartners(const char** partners, uint32_t count)
{
    @autoreleasepool {
        NSMutableArray* values = [NSMutableArray arrayWithCapacity:count];
        for (uint32_t i = 0; i < count; ++i)
            [values addObject:[NSString stringWithUTF8String:partners[i]]];
        OnMainThread(^{ [[AppsFlyerLib shared] setSharingFilterForPartners:values]; });
    }
}

int GetAppsFlyerUID(lua_State* L)
{
    lua_pushstring(L, [[[AppsFlyerLib shared] getAppsFlyerUID] UTF8String]);
    return 1;
}

int GetSDKVersion(lua_State* L)
{
    lua_pushstring(L, [[[AppsFlyerLib shared] getSdkVersion] UTF8String]);
    return 1;
}

} // namespace

#endif // platform
