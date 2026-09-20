#if defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS)
#pragma once

#include "appsflyer_private.h"
#include <dmsdk/sdk.h>

namespace dmAppsflyer {

enum MessageId
{
    CONVERSION_DATA_SUCCESS = 1,
    CONVERSION_DATA_FAIL = 2,
    START_SUCCESS = 3,
    START_FAIL = 4,
    EVENT_SUCCESS = 5,
    EVENT_FAIL = 6,
    DEEP_LINK_RESULT = 7
};

struct CallbackData
{
    MessageId msg;
    char* json;
};

void SetLuaCallback(lua_State* L, int pos);
void ClearLuaCallback();
void UpdateCallback();
void InitializeCallback();
void FinalizeCallback();

void AddToQueueCallback(MessageId type, const char* json);

} // namespace

#endif // platform
