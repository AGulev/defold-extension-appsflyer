#if defined(DM_PLATFORM_ANDROID)

#include <jni.h>
/* Header for class com_defold_appsflyer_AppsflyerJNI */

#ifndef COM_DEFOLD_APPSFLYER_APPSFLYERJNI_H
#define COM_DEFOLD_APPSFLYER_APPSFLYERJNI_H
#ifdef __cplusplus
extern "C" {
#endif
    /*
    * Class:     com_defold_appsflyer_AppsflyerJNI
    * Method:    appsflyerAddToQueue_first_arg
    * Signature: (ILjava/lang/String;I)V
    */
    JNIEXPORT void JNICALL Java_com_defold_appsflyer_AppsflyerJNI_appsflyerAddToQueue
        (JNIEnv *, jclass, jint, jstring);

#ifdef __cplusplus
}
#endif
#endif

#endif