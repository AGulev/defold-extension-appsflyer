#if defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS)

#pragma once

#include <dmsdk/sdk.h>

namespace dmAppsflyer {

struct TrackData
{
  char* key;
  char* value;
};

void Initialize_Ext();
void Finalize_Ext();

void InitializeSDK(const char* key, const char* appleAppID);
void StartSDK();
void SetDebugLog(bool is_debug);
void LogEvent(const char* eventName, dmArray<TrackData>* trackData);
void SetCustomerUserId(const char* userId);
// -1 preserves an unspecified consent signal; 0/1 are explicit false/true.
void SetConsentData(int gdpr, int dataUsage, int adsPersonalization, int adStorage);
void EnableTCFDataCollection(bool enabled);
void AnonymizeUser(bool enabled);
void StopSDK(bool stopped);
bool IsStopped();
void SetCurrencyCode(const char* currency);
void SetSharingFilterForPartners(const char** partners, uint32_t count);
int GetAppsFlyerUID(lua_State* L);
int GetSDKVersion(lua_State* L);

} // namespace

#endif // platform
