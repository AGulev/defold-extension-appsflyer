#define EXTENSION_NAME AppsflyerExt
#define LIB_NAME "Appsflyer"
#define MODULE_NAME "appsflyer"

#define DLIB_LOG_DOMAIN LIB_NAME

#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_ANDROID)

#include <stdlib.h>
#include "appsflyer_private.h"
#include "appsflyer_callback_private.h"
#include "utils/lua_util.h"

namespace dmAppsflyer {

dmArray<TrackData> list;

static int Lua_StartSDK(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    StartSDK();
    return 0;
}

static int Lua_GetAppsFlyerUID(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);

    return GetAppsFlyerUID(L);
}

static int Lua_SetCallback(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    SetLuaCallback(L, 1);
    return 0;
}

static int Lua_SetDebugLog(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    bool is_enable = luaL_checkbool(L, 1);
    SetDebugLog(is_enable);
    return 0;
}

static int Lua_LogEvent(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    const char* eventName = luaL_checkstring(L, 1);
    if (lua_type(L, 2) == LUA_TTABLE)
    {
        lua_pushvalue(L, 2);
        lua_pushnil(L);

        while (lua_next(L, -2) != 0)
        {
            TrackData data;
            const char* k = lua_tostring(L, -2);
            const char* s = lua_tostring(L, -1);
            if (!s)
            {
                char msg[256];
                snprintf(msg, sizeof(msg), "Wrong type for table attribute '%s'. Expected string, got %s", lua_tostring(L, -2), luaL_typename(L, -1) );
                luaL_error(L, msg);
                return 0;
            }
            data.key = strdup(k);
            data.value = strdup(s);
            if(list.Full())
            {
                list.OffsetCapacity(2);
            }
            list.Push(data);
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    }

    LogEvent(eventName, &list);

    for(int i = list.Size() - 1; i >= 0; --i)
    {
        free(list[i].key);
        free(list[i].value);
        list.EraseSwap(i);
    }

    return 0;
}

static int Lua_SetCustomerUserId(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    const char *customerUserId = luaL_checkstring(L, 1);
    SetCustomerUserId(customerUserId);
    return 0;
}

static const luaL_reg Module_methods[] =
{
    {"start_sdk", Lua_StartSDK},
    {"set_callback", Lua_SetCallback},
    {"set_debug_log", Lua_SetDebugLog},
    {"log_event", Lua_LogEvent},
    {"set_customer_user_id", Lua_SetCustomerUserId},
    {"get_appsflyer_uid", Lua_GetAppsFlyerUID},
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    luaL_register(L, MODULE_NAME, Module_methods);

#define SETCONSTANT(name) \
    lua_pushnumber(L, (lua_Number) name); \
    lua_setfield(L, -2, #name); \

    SETCONSTANT(CONVERSION_DATA_SUCCESS)
    SETCONSTANT(CONVERSION_DATA_FAIL)
#undef SETCONSTANT

    lua_pop(L, 1);
}

static dmExtension::Result AppInitializeAppsflyer(dmExtension::AppParams* params)
{
    Initialize_Ext();

    int isDebug = dmConfigFile::GetInt(params->m_ConfigFile, "appsflyer.is_debug", 0);
    if (isDebug > 0)
    {
        SetDebugLog(true);
    }

    const char* key = dmConfigFile::GetString(params->m_ConfigFile, "appsflyer.key", 0);
    if (!key)
    {
        dmLogError("Unable to resolve appsflyer.key option");
    }

    const char* appleAppID = dmConfigFile::GetString(params->m_ConfigFile, "appsflyer.apple_app_id", "");
    InitializeSDK(key, appleAppID);
    return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeAppsflyer(dmExtension::Params* params)
{
    LuaInit(params->m_L);
    InitializeCallback();
    return dmExtension::RESULT_OK;
}

static dmExtension::Result UpdateAppsflyer(dmExtension::Params* params)
{
    UpdateCallback();
    return dmExtension::RESULT_OK;
}

static dmExtension::Result AppFinalizeAppsflyer(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeAppsflyer(dmExtension::Params* params)
{
    FinalizeCallback();
    return dmExtension::RESULT_OK;
}

} // namespace

DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, dmAppsflyer::AppInitializeAppsflyer, dmAppsflyer::AppFinalizeAppsflyer, dmAppsflyer::InitializeAppsflyer, dmAppsflyer::UpdateAppsflyer,  0, dmAppsflyer::FinalizeAppsflyer)

#else // platform

static dmExtension::Result InitializeAppsflyer(dmExtension::Params *params) {
    dmLogInfo("Registered extension Appsflyer (null)");
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeAppsflyer(dmExtension::Params *params) {
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(EXTENSION_NAME,
LIB_NAME, 0, 0, InitializeAppsflyer, 0, 0, FinalizeAppsflyer)

#endif // platform