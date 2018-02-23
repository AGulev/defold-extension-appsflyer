#if defined(DM_PLATFORM_IOS)
#include "DefAppsFlyer.h"

#include <AppsFlyerLib/AppsFlyerTracker.h>

void DefAppsFlyer_setAppsFlyerKey(const char*appsFlyerKey)
{
    [AppsFlyerTracker sharedTracker].appsFlyerDevKey = [NSString stringWithUTF8String:appsFlyerKey];
    //[AppsFlyerTracker sharedTracker].delegate = self;
}

void DefAppsFlyer_setAppID(const char*appleAppID)
{
    [AppsFlyerTracker sharedTracker].appleAppID = [NSString stringWithUTF8String:appleAppID];
}

void DefAppsFlyer_setIsDebug(bool is_debug)
{
    [AppsFlyerTracker sharedTracker].isDebug = is_debug;
}

#endif
