#define EXTENSION_NAME DefAppsFlyer
#define LIB_NAME "DefAppsFlyer"
#define MODULE_NAME "appsflyer"

#define DLIB_LOG_DOMAIN LIB_NAME
#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_IOS) || defined(DM_PLATFORM_ANDROID)
#include "utils/LuaUtils.h"
#include "DefAppsFlyer.h"

static int setAppsFlyerKey(lua_State* L) {
    const char *appsFlyerKey = luaL_checkstring(L, 1);
    DefAppsFlyer_setAppsFlyerKey(appsFlyerKey);
    return 0;
}

static int setIsDebug(lua_State* L) {
    bool enableDebugMode_lua = luaL_checkbool(L, 1);
    DefAppsFlyer_setIsDebug(enableDebugMode_lua);
    return 0;
}

static int setAppID(lua_State* L) {
    const char *appId = luaL_checkstring(L, 1);
    DefAppsFlyer_setAppID(appId);
    return 0;
}

static const luaL_reg Module_methods[] =
{
    {"setAppsFlyerKey", setAppsFlyerKey},
    {"setIsDebug ", setIsDebug},
    {"setAppID ", setAppID},
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);
    luaL_register(L, MODULE_NAME, Module_methods);
    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

dmExtension::Result AppInitialize(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result Initialize(dmExtension::Params* params)
{
    LuaInit(params->m_L);
    const char* appsFlyerKey = dmConfigFile::GetString(params->m_ConfigFile, "AppsFlyer.key", 0);
    if (appsFlyerKey)
    {
        DefAppsFlyer_setAppsFlyerKey(appsFlyerKey);
    }
    #if defined(DM_PLATFORM_ANDROID)
    const char* package = dmConfigFile::GetString(params->m_ConfigFile, "android.package", 0);
    if (package)
    {
        DefAppsFlyer_setAppID(package);
    }
    #elseif defined(DM_PLATFORM_IOS)
    const char* appleAppID = dmConfigFile::GetString(params->m_ConfigFile, "AppsFlyer.AppleAppID", 0);
    if (appleAppID)
    {
        DefAppsFlyer_setAppID(appleAppID);
    }
    #endif
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalize(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result Finalize(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

#else // unsupported platforms

static dmExtension::Result AppInitialize(dmExtension::AppParams* params)
{
    dmLogWarning("Registered %s (null) Extension\n", MODULE_NAME);
    return dmExtension::RESULT_OK;
}

static dmExtension::Result Initialize(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result AppFinalize(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result Finalize(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

#endif // platforms


DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, AppInitialize, AppFinalize, Initialize, 0, 0, Finalize)
