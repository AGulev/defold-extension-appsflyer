#if defined(DM_PLATFORM_ANDROID)

#include <dmsdk/dlib/android.h>

#include "appsflyer_private.h"
#include "com_defold_appsflyer_AppsflyerJNI.h"
#include "appsflyer_callback_private.h"

JNIEXPORT void JNICALL Java_com_defold_appsflyer_AppsflyerJNI_appsflyerAddToQueue(JNIEnv * env, jclass cls, jint jmsg, jstring jjson)
{
    const char* json = env->GetStringUTFChars(jjson, 0);
    dmAppsflyer::AddToQueueCallback((dmAppsflyer::MessageId)jmsg, json);
    env->ReleaseStringUTFChars(jjson, json);
}

namespace dmAppsflyer {

struct Appsflyer
{
    jobject         m_AppsflyerJNI;
    jmethodID       m_InitializeSDK;
    jmethodID       m_StartSDK;
    jmethodID       m_SetDebugLog;
    jmethodID       m_LogEvent;
    jmethodID       m_SetCustomerUserId;
    jmethodID       m_GetAppsFlyerUID;
};

static Appsflyer g_appsflyer;

static void CallVoidMethodChar(jobject instance, jmethodID method, const char* cstr)
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();

    jstring jstr = env->NewStringUTF(cstr);
    env->CallVoidMethod(instance, method, jstr);
    env->DeleteLocalRef(jstr);
}

static void InitJNIMethods(JNIEnv* env, jclass cls)
{
    g_appsflyer.m_InitializeSDK = env->GetMethodID(cls, "initializeSDK", "(Ljava/lang/String;)V");
    g_appsflyer.m_StartSDK = env->GetMethodID(cls, "startSDK", "()V");
    g_appsflyer.m_SetDebugLog = env->GetMethodID(cls, "setDebugLog", "(Z)V");
    g_appsflyer.m_LogEvent = env->GetMethodID(cls, "logEvent", "(Ljava/lang/String;Ljava/util/Map;)V");
    g_appsflyer.m_SetCustomerUserId = env->GetMethodID(cls, "setCustomerUserId", "(Ljava/lang/String;)V");
    g_appsflyer.m_GetAppsFlyerUID = env->GetMethodID(cls, "getAppsFlyerUID", "()Ljava/lang/String;");
}

void Initialize_Ext()
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();
    jclass cls = dmAndroid::LoadClass(env, "com.defold.appsflyer.AppsflyerJNI");
    InitJNIMethods(env, cls);
    jmethodID jni_constructor = env->GetMethodID(cls, "<init>", "(Landroid/app/Activity;)V");
    g_appsflyer.m_AppsflyerJNI = env->NewGlobalRef(env->NewObject(cls, jni_constructor, threadAttacher.GetActivity()->clazz));
}

void InitializeSDK(const char* key, const char* appleAppID)
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();
    jstring jKey = env->NewStringUTF(key);
    env->CallVoidMethod(g_appsflyer.m_AppsflyerJNI, g_appsflyer.m_InitializeSDK, jKey);
    env->DeleteLocalRef(jKey);
}

void StartSDK()
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();
    env->CallVoidMethod(g_appsflyer.m_AppsflyerJNI, g_appsflyer.m_StartSDK);
}

int GetAppsFlyerUID(lua_State* L)
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();
    jstring jni_device_uid = (jstring)env->CallObjectMethod(g_appsflyer.m_AppsflyerJNI, g_appsflyer.m_GetAppsFlyerUID);
    lua_pushstring(L, env->GetStringUTFChars(jni_device_uid, 0));
    env->DeleteLocalRef(jni_device_uid);
    return 1;
}

void SetDebugLog(bool is_debug)
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();
    env->CallVoidMethod(g_appsflyer.m_AppsflyerJNI, g_appsflyer.m_SetDebugLog, is_debug);
}

void LogEvent(const char* eventName, dmArray<TrackData>* trackData)
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();

    jstring jEventName = env->NewStringUTF(eventName);

    jclass hashMapClass = env->FindClass("java/util/HashMap");
    jmethodID hashMapInit = env->GetMethodID(hashMapClass, "<init>", "(I)V");
    jobject hashMapObj = env->NewObject(hashMapClass, hashMapInit, trackData->Size());
    jmethodID hashMapId = env->GetMethodID(hashMapClass, "put","(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

    TrackData data;
    jstring key, value;
    for(uint32_t i = 0; i != trackData->Size(); i++)
    {
        data = (*trackData)[i];
        key = env->NewStringUTF(data.key);
        value = env->NewStringUTF(data.value);
        env->CallObjectMethod(hashMapObj, hashMapId, key, value);

        env->DeleteLocalRef(key);
        env->DeleteLocalRef(value);
    }

    env->CallVoidMethod(g_appsflyer.m_AppsflyerJNI, g_appsflyer.m_LogEvent, jEventName, hashMapObj);

    env->DeleteLocalRef(hashMapClass);
    env->DeleteLocalRef(hashMapObj);
    env->DeleteLocalRef(jEventName);
}

void SetCustomerUserId(const char* userId)
{
    CallVoidMethodChar(g_appsflyer.m_AppsflyerJNI, g_appsflyer.m_SetCustomerUserId, userId);
}

} // namespace

#endif // platform
