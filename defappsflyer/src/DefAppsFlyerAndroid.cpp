#if defined(DM_PLATFORM_ANDROID)
#include <jni.h>
#include <dmsdk/sdk.h>
#include "DefAppsFlyer.h"

const char* JAR_PATH = "com/agulev/defappsflyer/DefAppsFlyer";

static JNIEnv* Attach()
{
  JNIEnv* env;
  JavaVM* vm = dmGraphics::GetNativeAndroidJavaVM();
  vm->AttachCurrentThread(&env, NULL);
  return env;
}

static bool Detach(JNIEnv* env)
{
  bool exception = (bool) env->ExceptionCheck();
  env->ExceptionClear();
  JavaVM* vm = dmGraphics::GetNativeAndroidJavaVM();
  vm->DetachCurrentThread();
  return !exception;
}

namespace {
struct AttachScope
{
  JNIEnv* m_Env;
  AttachScope() : m_Env(Attach())
  {
  }
  ~AttachScope()
  {
    Detach(m_Env);
  }
};
}

static jclass GetClass(JNIEnv* env, const char* classname)
{
  jclass activity_class = env->FindClass("android/app/NativeActivity");
  jmethodID get_class_loader = env->GetMethodID(activity_class,"getClassLoader", "()Ljava/lang/ClassLoader;");
  jobject cls = env->CallObjectMethod(dmGraphics::GetNativeAndroidActivity(), get_class_loader);
  jclass class_loader = env->FindClass("java/lang/ClassLoader");
  jmethodID find_class = env->GetMethodID(class_loader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

  jstring str_class_name = env->NewStringUTF(classname);
  jclass outcls = (jclass)env->CallObjectMethod(cls, find_class, str_class_name);

  env->DeleteLocalRef(str_class_name);
  env->DeleteLocalRef(cls);
  env->DeleteLocalRef(activity_class);
  env->DeleteLocalRef(class_loader);
  return outcls;
}

void DefAppsFlyer_setAppsFlyerKey(const char*appsFlyerKey)
{
  AttachScope attachscope;
  JNIEnv* env = attachscope.m_Env;
  jclass cls = GetClass(env, JAR_PATH);
  jstring afkey = env->NewStringUTF(appsFlyerKey);
  jmethodID method = env->GetStaticMethodID(cls, "DefAppsFlyer_setAppsFlyerKey", "(Landroid/app/Activity;Ljava/lang/String;)V");
  env->CallStaticVoidMethod(cls, method, dmGraphics::GetNativeAndroidActivity(), afkey);

  env->DeleteLocalRef(cls);
}

void DefAppsFlyer_setIsDebug(bool is_debug)
{
  AttachScope attachscope;
  JNIEnv* env = attachscope.m_Env;
  jclass cls = GetClass(env, JAR_PATH);
  jmethodID method = env->GetStaticMethodID(cls, "DefAppsFlyer_setIsDebug", "(Z)V");
  env->CallStaticVoidMethod(cls, method, is_debug ? JNI_TRUE : JNI_FALSE);
  env->DeleteLocalRef(cls);
}

void DefAppsFlyer_trackEvent(const char*eventName, dmArray<TrackData>* trackData)
{
  // NSString* key;
  // NSString* value;
  // TrackData data;
  // for(uint32_t i = 0; i != trackData->Size(); i++)
  // {
  //   data = (*trackData)[i];
  // }
}

void DefAppsFlyer_trackAppLaunch()
{
  //no need on android
}

void DefAppsFlyer_setAppID(const char*appleAppID)
{
  //no need on android
}

#endif
