#pragma once
#include <dmsdk/sdk.h>

struct TrackData
{
  char* key;
  char* value;
};

extern void DefAppsFlyer_setAppsFlyerKey(const char*appsFlyerKey);
extern void DefAppsFlyer_setAppID(const char*appleAppID);
extern void DefAppsFlyer_setIsDebug(bool is_debug);
extern void DefAppsFlyer_trackAppLaunch();
extern void DefAppsFlyer_trackEvent(const char*eventName, dmArray<TrackData>* trackData);
