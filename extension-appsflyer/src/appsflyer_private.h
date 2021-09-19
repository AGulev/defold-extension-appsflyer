#if defined(DM_PLATFORM_ANDROID)

#pragma once

#include <dmsdk/sdk.h>

namespace dmAppsflyer {

struct TrackData
{
  char* key;
  char* value;
};

void Initialize_Ext();

void InitializeSDK(const char* key, const char* appleAppID);
void StartSDK();
void SetDebugLog(bool is_debug);
void LogEvent(const char* eventName, dmArray<TrackData>* trackData);
void SetCustomerUserId(const char* userId);

} // namespace

#endif // platform