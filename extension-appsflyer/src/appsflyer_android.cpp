#if defined(DM_PLATFORM_ANDROID)

#include <jni.h>

#include "appsflyer_jni.h"
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
};

static Appsflyer g_appsflyer;

static void CallVoidMethodChar(jobject instance, jmethodID method, const char* cstr)
{
    ThreadAttacher attacher;
    JNIEnv *env = attacher.env;

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
}

void Initialize_Ext()
{
    ThreadAttacher attacher;
    JNIEnv *env = attacher.env;
    ClassLoader class_loader = ClassLoader(env);
    jclass cls = class_loader.load("com.defold.appsflyer.AppsflyerJNI");
    InitJNIMethods(env, cls);
    jmethodID jni_constructor = env->GetMethodID(cls, "<init>", "(Landroid/app/Activity;)V");
    g_appsflyer.m_AppsflyerJNI = env->NewGlobalRef(env->NewObject(cls, jni_constructor, dmGraphics::GetNativeAndroidActivity()));
}

void InitializeSDK(const char* key, const char* appleAppID)
{
    ThreadAttacher attacher;
    JNIEnv *env = attacher.env;
    jstring jKey = env->NewStringUTF(key);
    env->CallVoidMethod(g_appsflyer.m_AppsflyerJNI, g_appsflyer.m_InitializeSDK, jKey);
    env->DeleteLocalRef(jKey);
}

void StartSDK()
{
    ThreadAttacher attacher;
    JNIEnv *env = attacher.env;
    env->CallVoidMethod(g_appsflyer.m_AppsflyerJNI, g_appsflyer.m_StartSDK);
}

void SetDebugLog(bool is_debug)
{
    ThreadAttacher attacher;
    JNIEnv *env = attacher.env;
    env->CallVoidMethod(g_appsflyer.m_AppsflyerJNI, g_appsflyer.m_SetDebugLog, is_debug);
}

void LogEvent(const char* eventName, dmArray<TrackData>* trackData)
{
    ThreadAttacher attacher;
    JNIEnv *env = attacher.env;

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