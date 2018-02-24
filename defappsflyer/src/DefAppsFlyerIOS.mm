#if defined(DM_PLATFORM_IOS)
#include "DefAppsFlyer.h"
#include <dmsdk/sdk.h>

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

void DefAppsFlyer_trackAppLaunch()
{
  [[AppsFlyerTracker sharedTracker] trackAppLaunch];
}


void DefAppsFlyer_trackEvent(const char*eventName, dmArray<TrackData>* trackData)
{
  NSMutableDictionary* newDict = [NSMutableDictionary dictionary];
  NSString* key;
  NSString* value;
  TrackData data;
  for(uint32_t i = 0; i != trackData->Size(); i++)
  {
    data = (*trackData)[i];
    key = [NSString stringWithUTF8String: data.key];
    value = [NSString stringWithUTF8String: data.value];
    newDict[key] = value;
  }
  [[AppsFlyerTracker sharedTracker] trackEvent: [NSString stringWithUTF8String: eventName] withValues: newDict];
}

#endif
