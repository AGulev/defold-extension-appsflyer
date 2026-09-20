#define EXTENSION_NAME AppsflyerExt
#define LIB_NAME "Appsflyer"
#define MODULE_NAME "appsflyer"

#ifndef DLIB_LOG_DOMAIN
#define DLIB_LOG_DOMAIN LIB_NAME
#endif

#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS)

#include <stdlib.h>
#include "appsflyer_private.h"
#include "appsflyer_callback_private.h"
#include "utils/lua_util.h"

namespace dmAppsflyer {

static bool g_sdkInitialized = false;

static int Lua_StartSDK(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    if (!g_sdkInitialized)
        return luaL_error(L, "AppsFlyer is not initialized. Configure appsflyer.key and, on iOS, appsflyer.apple_app_id.");
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

static int Lua_GetSDKVersion(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    return GetSDKVersion(L);
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
    if (!g_sdkInitialized)
        return luaL_error(L, "AppsFlyer is not initialized.");

    // Validate before allocating: luaL_error does not run C++ destructors.
    if (lua_type(L, 2) == LUA_TTABLE)
    {
        lua_pushnil(L);
        while (lua_next(L, 2) != 0)
        {
            if (lua_type(L, -2) != LUA_TSTRING)
                return luaL_error(L, "AppsFlyer event parameter keys must be strings.");
            if (!lua_isstring(L, -1))
                return luaL_error(L, "AppsFlyer event parameter '%s' must be a string or number.", lua_tostring(L, -2));
            lua_pop(L, 1);
        }
    }
    else if (!lua_isnoneornil(L, 2))
    {
        return luaL_error(L, "AppsFlyer event parameters must be a table or nil.");
    }

    dmArray<TrackData> list;
    if (lua_istable(L, 2))
    {
        lua_pushnil(L);
        while (lua_next(L, 2) != 0)
        {
            TrackData data;
            data.key = strdup(lua_tostring(L, -2));
            data.value = strdup(lua_tostring(L, -1));
            if(list.Full())
            {
                list.OffsetCapacity(2);
            }
            list.Push(data);
            lua_pop(L, 1);
        }
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

static int OptionalConsentBool(lua_State* L, const char* name)
{
    lua_getfield(L, 1, name);
    if (!lua_isnil(L, -1) && !lua_isboolean(L, -1))
        return luaL_error(L, "AppsFlyer consent field '%s' must be a boolean or nil.", name);
    int value = lua_isnil(L, -1) ? -1 : lua_toboolean(L, -1);
    lua_pop(L, 1);
    return value;
}

static int Lua_SetConsentData(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    luaL_checktype(L, 1, LUA_TTABLE);
    // Reject misspelled fields instead of silently dropping a consent choice.
    lua_pushnil(L);
    while (lua_next(L, 1) != 0)
    {
        if (lua_type(L, -2) != LUA_TSTRING)
            return luaL_error(L, "AppsFlyer consent keys must be strings.");
        const char* key = lua_tostring(L, -2);
        if (strcmp(key, "is_user_subject_to_gdpr") && strcmp(key, "has_consent_for_data_usage") &&
            strcmp(key, "has_consent_for_ads_personalization") && strcmp(key, "has_consent_for_ad_storage"))
            return luaL_error(L, "Unknown AppsFlyer consent field '%s'.", key);
        lua_pop(L, 1);
    }
    int gdpr = OptionalConsentBool(L, "is_user_subject_to_gdpr");
    int dataUsage = OptionalConsentBool(L, "has_consent_for_data_usage");
    int adsPersonalization = OptionalConsentBool(L, "has_consent_for_ads_personalization");
    int adStorage = OptionalConsentBool(L, "has_consent_for_ad_storage");
    SetConsentData(gdpr, dataUsage, adsPersonalization, adStorage);
    return 0;
}

static int Lua_EnableTCFDataCollection(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    EnableTCFDataCollection(luaL_checkbool(L, 1));
    return 0;
}

static int Lua_AnonymizeUser(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    AnonymizeUser(luaL_checkbool(L, 1));
    return 0;
}

static int Lua_StopSDK(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    StopSDK(luaL_checkbool(L, 1));
    return 0;
}

static int Lua_IsStopped(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushboolean(L, IsStopped());
    return 1;
}

static int Lua_SetCurrencyCode(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    size_t length;
    const char* currency = luaL_checklstring(L, 1, &length);
    if (length != 3 || strspn(currency, "ABCDEFGHIJKLMNOPQRSTUVWXYZ") != 3)
        return luaL_error(L, "AppsFlyer currency must be a three-letter uppercase ISO 4217 code.");
    SetCurrencyCode(currency);
    return 0;
}

static int Lua_SetSharingFilterForPartners(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    luaL_checktype(L, 1, LUA_TTABLE);
    uint32_t count = (uint32_t)lua_objlen(L, 1);
    uint32_t entries = 0;
    // Validate the full sequence before allocating or forwarding any values.
    lua_pushnil(L);
    while (lua_next(L, 1) != 0)
    {
        lua_Number index = lua_type(L, -2) == LUA_TNUMBER ? lua_tonumber(L, -2) : 0;
        if (index < 1 || index > count || index != (uint32_t)index)
            return luaL_error(L, "AppsFlyer partners must be a contiguous array starting at index 1.");
        if (lua_type(L, -1) != LUA_TSTRING)
            return luaL_error(L, "AppsFlyer partner IDs must be strings.");
        size_t length;
        const char* partner = lua_tolstring(L, -1, &length);
        if (length == 0 || length > 45 || strspn(partner, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_") != length)
            return luaL_error(L, "AppsFlyer partner IDs must contain 1-45 letters, digits or underscores.");
        ++entries;
        lua_pop(L, 1);
    }
    if (entries != count)
        return luaL_error(L, "AppsFlyer partners must not contain array holes.");
    dmArray<const char*> partners;
    partners.SetCapacity(count);
    for (uint32_t i = 1; i <= count; ++i)
    {
        lua_rawgeti(L, 1, i);
        partners.Push(lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    SetSharingFilterForPartners(partners.Begin(), count);
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
    {"get_sdk_version", Lua_GetSDKVersion},
    {"set_consent_data", Lua_SetConsentData},
    {"enable_tcf_data_collection", Lua_EnableTCFDataCollection},
    {"anonymize_user", Lua_AnonymizeUser},
    {"stop_sdk", Lua_StopSDK},
    {"is_stopped", Lua_IsStopped},
    {"set_currency_code", Lua_SetCurrencyCode},
    {"set_sharing_filter_for_partners", Lua_SetSharingFilterForPartners},
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
    SETCONSTANT(START_SUCCESS)
    SETCONSTANT(START_FAIL)
    SETCONSTANT(EVENT_SUCCESS)
    SETCONSTANT(EVENT_FAIL)
    SETCONSTANT(DEEP_LINK_RESULT)
#undef SETCONSTANT

    lua_pop(L, 1);
}

static dmExtension::Result AppInitializeAppsflyer(dmExtension::AppParams* params)
{
    InitializeCallback();
    Initialize_Ext();

    int isDebug = dmConfigFile::GetInt(params->m_ConfigFile, "appsflyer.is_debug", 0);
    SetDebugLog(isDebug > 0);

    const char* key = dmConfigFile::GetString(params->m_ConfigFile, "appsflyer.key", "");
    if (!key[0])
    {
        dmLogWarning("AppsFlyer disabled: configure appsflyer.key in game.project.");
        return dmExtension::RESULT_OK;
    }

    const char* appleAppID = dmConfigFile::GetString(params->m_ConfigFile, "appsflyer.apple_app_id", "");
#if defined(DM_PLATFORM_IOS)
    if (!appleAppID[0] || strspn(appleAppID, "0123456789") != strlen(appleAppID))
    {
        dmLogWarning("AppsFlyer disabled: appsflyer.apple_app_id must be a numeric App Store ID without the 'id' prefix.");
        return dmExtension::RESULT_OK;
    }
#endif
    InitializeSDK(key, appleAppID);
    g_sdkInitialized = true;
    return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeAppsflyer(dmExtension::Params* params)
{
    LuaInit(params->m_L);
    return dmExtension::RESULT_OK;
}

static dmExtension::Result UpdateAppsflyer(dmExtension::Params* params)
{
    UpdateCallback();
    return dmExtension::RESULT_OK;
}

static dmExtension::Result AppFinalizeAppsflyer(dmExtension::AppParams* params)
{
    g_sdkInitialized = false;
    Finalize_Ext();
    FinalizeCallback();
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeAppsflyer(dmExtension::Params* params)
{
    ClearLuaCallback();
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
