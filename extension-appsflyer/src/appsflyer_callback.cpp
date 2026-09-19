#if defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS)

#include "appsflyer_callback_private.h"
#include "utils/lua_util.h"
#include <stdlib.h>

namespace dmAppsflyer {

static dmScript::LuaCallbackInfo* m_luaCallback = 0x0;
static dmScript::LuaCallbackInfo* m_activeCallback = 0x0;
static dmArray<CallbackData> m_callbacksQueue;
static dmMutex::HMutex m_mutex;
static bool m_acceptCallbacks = false;

void ClearLuaCallback()
{
    if (m_luaCallback != 0x0)
    {
        // Lua may replace or clear its callback while it is being invoked.
        // Keep that callback alive until TeardownCallback has restored Lua state.
        if (m_luaCallback != m_activeCallback)
            dmScript::DestroyCallback(m_luaCallback);
        m_luaCallback = 0x0;
    }
}

static void InvokeCallback(MessageId type, const char* json)
{
    if (!m_luaCallback || !dmScript::IsCallbackValid(m_luaCallback))
    {
        dmLogError("Appsflyer callback is invalid. Set new callback using `appsflyer.set_callback()` function.");
        return;
    }

    dmScript::LuaCallbackInfo* callback = m_luaCallback;
    lua_State* L = dmScript::GetCallbackLuaContext(callback);
    int top = lua_gettop(L);

    if (!dmScript::SetupCallback(callback))
    {
        return;
    }
    
    m_activeCallback = callback;
    lua_pushnumber(L, type);
    dmScript::JsonToLua(L, json, strlen(json)); // SDK messages contain valid JSON.

    int ret = dmScript::PCall(L, 3, 0);
    (void)ret;
    dmScript::TeardownCallback(callback);
    m_activeCallback = 0x0;
    if (callback != m_luaCallback)
        dmScript::DestroyCallback(callback);

    assert(top == lua_gettop(L));
}

void InitializeCallback()
{
    if (!m_mutex)
        m_mutex = dmMutex::New();
    DM_MUTEX_SCOPED_LOCK(m_mutex);
    m_acceptCallbacks = true;
}

void FinalizeCallback()
{
    // SDK requests can finish after extension shutdown. Keep the mutex alive
    // for the process lifetime so those callbacks can safely be discarded.
    DM_MUTEX_SCOPED_LOCK(m_mutex);
    m_acceptCallbacks = false;
    for (uint32_t i = 0; i < m_callbacksQueue.Size(); ++i)
        free(m_callbacksQueue[i].json);
    m_callbacksQueue.SetSize(0);
}

void SetLuaCallback(lua_State* L, int pos)
{
    int type = lua_type(L, pos);
    if (type != LUA_TNONE && type != LUA_TNIL)
        luaL_checktype(L, pos, LUA_TFUNCTION);
    ClearLuaCallback();
    if (type == LUA_TNONE || type == LUA_TNIL)
    {
        return;
    }
    else
    {
        m_luaCallback = dmScript::CreateCallback(L, pos);
    }
}

void AddToQueueCallback(MessageId type, const char* json)
{
    DM_MUTEX_SCOPED_LOCK(m_mutex);
    if (!m_acceptCallbacks)
        return;

    CallbackData data;
    data.msg = type;
    data.json = strdup(json ? json : "{}");

    if(m_callbacksQueue.Full())
    {
        m_callbacksQueue.OffsetCapacity(2);
    }
    m_callbacksQueue.Push(data);
}

void UpdateCallback()
{
    if (!m_luaCallback)
    {
        return;
    }

    dmArray<CallbackData> tmp;
    {
        DM_MUTEX_SCOPED_LOCK(m_mutex);
        if (m_callbacksQueue.Empty())
            return;
        tmp.Swap(m_callbacksQueue);
    }

    for(uint32_t i = 0; i != tmp.Size(); ++i)
    {
        CallbackData* data = &tmp[i];
        InvokeCallback(data->msg, data->json);
        if(data->json)
        {
            free(data->json);
            data->json = 0;
        }
    }
}

} // namespace

#endif // platform
