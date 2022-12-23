#if defined(DM_PLATFORM_IOS)


#include <dmsdk/sdk.h>
// AppDelegate.h
#import <AppsFlyerLib/AppsFlyerLib.h>
#include "appsflyer_private.h"
#import "appsflyer_callback_private.h"
#import "DEFAFSDKDelegate.h"


namespace dmAppsflyer {

void ForwardIOSEvent(int type, NSString * json)
{
    const char *JsonCString = [json UTF8String];
    AddToQueueCallback((MessageId)type, JsonCString);
}

void Initialize_Ext(){

}


void onConversionDataSuccess(NSDictionary *installData) {
  
    NSData *data = [NSJSONSerialization dataWithJSONObject: installData options: 0 error: nil];
    NSString *serializedParameters = [[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding];
    ForwardIOSEvent(1, serializedParameters);
}

void onConversionDataFail(NSString *error) {
    NSLog(@"%@", error);
}
void onAppOpenAttribution(NSDictionary *attributionData) {

}
void onAppOpenAttributionFailure(NSString *error) {
    NSLog(@"%@", error);
}

void InitializeSDK(const char* key, const char* appleAppID){
  
  DEFAFSDKDelegate *delegate = [[DEFAFSDKDelegate alloc] init];
  delegate.onConversionDataSuccess = onConversionDataSuccess;
  delegate.onConversionDataFail = onConversionDataFail;
  delegate.onAppOpenAttribution = onAppOpenAttribution;
  delegate.onAppOpenAttributionFailure = onAppOpenAttributionFailure;
  [[AppsFlyerLib shared] setAppsFlyerDevKey:[NSString stringWithUTF8String: key]];
  [[AppsFlyerLib shared] setAppleAppID:[NSString stringWithUTF8String: appleAppID]];
  [AppsFlyerLib shared].delegate = (id<AppsFlyerLibDelegate>)delegate;
}

void StartSDK(){
  [[AppsFlyerLib shared] start];
}

void SetDebugLog(bool is_debug){
  [AppsFlyerLib shared].isDebug = is_debug;
}

void LogEvent(const char* eventName, dmArray<TrackData>* trackData){
  @autoreleasepool {
    NSMutableDictionary* newDict = [NSMutableDictionary dictionary];
    NSString* key;
    NSString* value;
    NSString* event = [NSString stringWithUTF8String: eventName];
    TrackData data;
    for(uint32_t i = 0; i != trackData->Size(); i++)
    {
      data = (*trackData)[i];
      key = [NSString stringWithUTF8String: data.key];
      value = [NSString stringWithUTF8String: data.value];
      newDict[key] = value;
    }
    [[AppsFlyerLib shared] logEvent: event withValues: newDict];
  }
}

void SetCustomerUserId(const char* userId){
  [AppsFlyerLib shared].customerUserID = [NSString stringWithUTF8String: userId];
}

int GetAppsFlyerUID(lua_State* L){
    lua_pushstring(L, [[[AppsFlyerLib shared] getAppsFlyerUID] UTF8String]);
    return 1;
}

} // namespace

#endif // platform