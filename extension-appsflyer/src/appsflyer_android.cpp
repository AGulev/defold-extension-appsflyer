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
    jmethodID       m_GetSDKVersion;
    jmethodID       m_FinalizeSDK;
    jmethodID       m_SetConsentData;
    jmethodID       m_EnableTCFDataCollection;
    jmethodID       m_AnonymizeUser;
    jmethodID       m_StopSDK;
    jmethodID       m_IsStopped;
    jmethodID       m_SetCurrencyCode;
    jmethodID       m_SetSharingFilterForPartners;
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
    g_appsflyer.m_GetSDKVersion = env->GetMethodID(cls, "getSDKVersion", "()Ljava/lang/String;");
    g_appsflyer.m_FinalizeSDK = env->GetMethodID(cls, "finalizeSDK", "()V");
    g_appsflyer.m_SetConsentData = env->GetMethodID(cls, "setConsentData", "(IIII)V");
    g_appsflyer.m_EnableTCFDataCollection = env->GetMethodID(cls, "enableTCFDataCollection", "(Z)V");
    g_appsflyer.m_AnonymizeUser = env->GetMethodID(cls, "anonymizeUser", "(Z)V");
    g_appsflyer.m_StopSDK = env->GetMethodID(cls, "stopSDK", "(Z)V");
    g_appsflyer.m_IsStopped = env->GetMethodID(cls, "isStopped", "()Z");
    g_appsflyer.m_SetCurrencyCode = env->GetMethodID(cls, "setCurrencyCode", "(Ljava/lang/String;)V");
    g_appsflyer.m_SetSharingFilterForPartners = env->GetMethodID(cls, "setSharingFilterForPartners", "([Ljava/lang/String;)V");
}

void Initialize_Ext()
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();
    jclass cls = dmAndroid::LoadClass(env, "com.defold.appsflyer.AppsflyerJNI");
    InitJNIMethods(env, cls);
    jmethodID jni_constructor = env->GetMethodID(cls, "<init>", "(Landroid/app/Activity;)V");
    jobject instance = env->NewObject(cls, jni_constructor, threadAttacher.GetActivity()->clazz);
    g_appsflyer.m_AppsflyerJNI = env->NewGlobalRef(instance);
    env->DeleteLocalRef(instance);
    env->DeleteLocalRef(cls);
}

void Finalize_Ext()
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();
    env->CallVoidMethod(g_appsflyer.m_AppsflyerJNI, g_appsflyer.m_FinalizeSDK);
    env->DeleteGlobalRef(g_appsflyer.m_AppsflyerJNI);
    g_appsflyer.m_AppsflyerJNI = 0;
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

static int PushJavaString(lua_State* L, jmethodID method)
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();
    jstring value = (jstring)env->CallObjectMethod(g_appsflyer.m_AppsflyerJNI, method);
    if (value)
    {
        const char* utf8 = env->GetStringUTFChars(value, 0);
        lua_pushstring(L, utf8);
        env->ReleaseStringUTFChars(value, utf8);
        env->DeleteLocalRef(value);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

int GetAppsFlyerUID(lua_State* L)
{
    return PushJavaString(L, g_appsflyer.m_GetAppsFlyerUID);
}

int GetSDKVersion(lua_State* L)
{
    return PushJavaString(L, g_appsflyer.m_GetSDKVersion);
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

void SetConsentData(int gdpr, int dataUsage, int adsPersonalization, int adStorage)
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();
    env->CallVoidMethod(g_appsflyer.m_AppsflyerJNI, g_appsflyer.m_SetConsentData, gdpr, dataUsage, adsPersonalization, adStorage);
}

static void CallBooleanSetter(jmethodID method, bool value)
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();
    env->CallVoidMethod(g_appsflyer.m_AppsflyerJNI, method, (jboolean)value);
}

void EnableTCFDataCollection(bool enabled)
{
    CallBooleanSetter(g_appsflyer.m_EnableTCFDataCollection, enabled);
}

void AnonymizeUser(bool enabled)
{
    CallBooleanSetter(g_appsflyer.m_AnonymizeUser, enabled);
}

void StopSDK(bool stopped)
{
    CallBooleanSetter(g_appsflyer.m_StopSDK, stopped);
}

bool IsStopped()
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();
    return env->CallBooleanMethod(g_appsflyer.m_AppsflyerJNI, g_appsflyer.m_IsStopped);
}

void SetCurrencyCode(const char* currency)
{
    CallVoidMethodChar(g_appsflyer.m_AppsflyerJNI, g_appsflyer.m_SetCurrencyCode, currency);
}

void SetSharingFilterForPartners(const char** partners, uint32_t count)
{
    dmAndroid::ThreadAttacher threadAttacher;
    JNIEnv* env = threadAttacher.GetEnv();
    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray values = env->NewObjectArray(count, stringClass, 0);
    for (uint32_t i = 0; i < count; ++i)
    {
        jstring value = env->NewStringUTF(partners[i]);
        env->SetObjectArrayElement(values, i, value);
        env->DeleteLocalRef(value);
    }
    env->CallVoidMethod(g_appsflyer.m_AppsflyerJNI, g_appsflyer.m_SetSharingFilterForPartners, values);
    env->DeleteLocalRef(values);
    env->DeleteLocalRef(stringClass);
}

} // namespace

#endif // platform
