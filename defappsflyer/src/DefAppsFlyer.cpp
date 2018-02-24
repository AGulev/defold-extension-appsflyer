#define EXTENSION_NAME DefAppsFlyer
#define LIB_NAME "DefAppsFlyer"
#define MODULE_NAME "appsflyer"

#define DLIB_LOG_DOMAIN LIB_NAME
#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_IOS) || defined(DM_PLATFORM_ANDROID)
#include "utils/LuaUtils.h"
#include "DefAppsFlyer.h"

dmArray<TrackData> list;

static int setAppsFlyerKey(lua_State* L)
{
  DM_LUA_STACK_CHECK(L, 0);
  const char *appsFlyerKey = luaL_checkstring(L, 1);
  DefAppsFlyer_setAppsFlyerKey(appsFlyerKey);
  return 0;
}

static int setIsDebug(lua_State* L)
{
  DM_LUA_STACK_CHECK(L, 0);
  bool enableDebugMode_lua = luaL_checkbool(L, 1);
  DefAppsFlyer_setIsDebug(enableDebugMode_lua);
  return 0;
}

static int setAppID(lua_State* L)
{
  DM_LUA_STACK_CHECK(L, 0);
  const char *appId = luaL_checkstring(L, 1);
  DefAppsFlyer_setAppID(appId);
  return 0;
}

static int trackAppLaunch(lua_State* L)
{
  DefAppsFlyer_trackAppLaunch();
  return 0;
}

static int trackEvent(lua_State* L)
{
  DM_LUA_STACK_CHECK(L, 0);
  const char *eventName = luaL_checkstring(L, 1);
  if(lua_type(L, 2) == LUA_TTABLE)
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
  DefAppsFlyer_trackEvent(eventName, &list);
  for(uint32_t i = list.Size() - 1; i > 0; --i)
  {
    free(list[i].key);
    free(list[i].value);
    list.EraseSwap(i);
  }
  return 0;
}

static const luaL_reg Module_methods[] =
{
  {"setAppsFlyerKey", setAppsFlyerKey},
  {"setIsDebug", setIsDebug},
  {"setAppID", setAppID},
  {"trackAppLaunch", trackAppLaunch},
  {"trackEvent", trackEvent},
  {0, 0}
};

static void LuaInit(lua_State* L)
{
  int top = lua_gettop(L);
  luaL_register(L, MODULE_NAME, Module_methods);
  lua_pop(L, 1);
  assert(top == lua_gettop(L));
}

dmExtension::Result Initilize(dmExtension::Params* params)
{
  LuaInit(params->m_L);
  int isDebug = dmConfigFile::GetInt(params->m_ConfigFile, "AppsFlyer.is_debug", 0);
  if (isDebug > 0)
  {
    DefAppsFlyer_setIsDebug(true);
  }
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
#elif defined(DM_PLATFORM_IOS)
  const char* appleAppID = dmConfigFile::GetString(params->m_ConfigFile, "AppsFlyer.AppleAppID", 0);
  if (appleAppID)
  {
    DefAppsFlyer_setAppID(appleAppID);
  }
#endif
  trackAppLaunch(params->m_L);
  return dmExtension::RESULT_OK;
}

static void OnEvent(dmExtension::Params* params, const dmExtension::Event* event)
{
  if (event->m_Event == dmExtension::EVENT_ID_ACTIVATEAPP)
  {
    trackAppLaunch(params->m_L);
  }
}

dmExtension::Result Finalize(dmExtension::Params* params)
{
  return dmExtension::RESULT_OK;
}

#else // unsupported platforms

static dmExtension::Result Initilize(dmExtension::Params* params)
{
  dmLogWarning("Registered %s (null) Extension\n", MODULE_NAME);
  return dmExtension::RESULT_OK;
}

static void OnEvent(dmExtension::Params* params, const dmExtension::Event* event)
{
}

static dmExtension::Result Finalize(dmExtension::Params* params)
{
  return dmExtension::RESULT_OK;
}

#endif // platforms


DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, 0, 0, Initilize, 0, OnEvent, Finalize)
