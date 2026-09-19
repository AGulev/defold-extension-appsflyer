package com.defold.appsflyer;

import android.app.Activity;
import android.util.Log;

import com.appsflyer.AppsFlyerLib;
import com.appsflyer.share.AppsFlyerConversionListener;
import com.appsflyer.share.AppsFlyerConsent;
import com.appsflyer.share.SessionReadyListener;
import com.appsflyer.share.attribution.AppsFlyerRequestListener;
import com.appsflyer.share.deeplink.DeepLink;
import com.appsflyer.share.deeplink.DeepLinkListener;
import com.appsflyer.share.deeplink.DeepLinkResult;

import java.util.Map;
import java.util.concurrent.atomic.AtomicBoolean;
import org.json.JSONObject;
import org.json.JSONException;

public class AppsflyerJNI {
    private static final String TAG = "AppsflyerJNI";
    private static final int CONVERSION_DATA_SUCCESS = 1;
    private static final int CONVERSION_DATA_FAIL = 2;
    private static final int START_SUCCESS = 3;
    private static final int START_FAIL = 4;
    private static final int EVENT_SUCCESS = 5;
    private static final int EVENT_FAIL = 6;
    private static final int DEEP_LINK_RESULT = 7;

    public static native void appsflyerAddToQueue(int msg, String json);

    private final Activity activity;
    // Access these only on the Android UI thread.
    private boolean initialized;
    private boolean startRequested;
    private boolean sessionReady;

    public AppsflyerJNI(Activity activity) {
        this.activity = activity;
    }

    private static void reportError(int type, int code, String error, String eventName) {
        try {
            JSONObject data = new JSONObject();
            data.put("error", error);
            data.put("code", code);
            if (eventName != null) data.put("event_name", eventName);
            appsflyerAddToQueue(type, data.toString());
        } catch (JSONException e) {
            Log.e(TAG, "Unable to encode SDK error", e);
        }
    }

    private static AppsFlyerRequestListener requestListener(final String eventName) {
        return new AppsFlyerRequestListener() {
            // SDK 7 can report the stopped-event failure through two paths.
            private final AtomicBoolean completed = new AtomicBoolean();

            @Override
            public void onSuccess() {
                if (completed.getAndSet(true)) return;
                JSONObject data = new JSONObject();
                try {
                    if (eventName != null) data.put("event_name", eventName);
                    appsflyerAddToQueue(eventName == null ? START_SUCCESS : EVENT_SUCCESS, data.toString());
                } catch (JSONException e) {
                    Log.e(TAG, "Unable to encode SDK response", e);
                }
            }

            @Override
            public void onError(int code, String error) {
                if (completed.getAndSet(true)) return;
                reportError(eventName == null ? START_FAIL : EVENT_FAIL, code, error, eventName);
            }
        };
    }

    public void initializeSDK(final String key) {
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AppsFlyerLib sdk = AppsFlyerLib.getInstance();
                sdk.subscribeForDeepLink(new DeepLinkListener() {
                    @Override
                    public void onDeepLinking(DeepLinkResult result) {
                        try {
                            JSONObject data = new JSONObject();
                            data.put("status", result.getStatus().name());
                            DeepLink link = result.getDeepLink();
                            if (result.getStatus() == DeepLinkResult.Status.FOUND && link != null) {
                                data.put("deep_link", link.getClickEvent());
                                data.put("is_deferred", Boolean.TRUE.equals(link.isDeferred()));
                            } else if (result.getStatus() == DeepLinkResult.Status.ERROR) {
                                data.put("error", String.valueOf(result.getError()));
                            }
                            appsflyerAddToQueue(DEEP_LINK_RESULT, data.toString());
                        } catch (JSONException e) {
                            Log.e(TAG, "Unable to encode deep link result", e);
                        }
                    }
                });
                // Defold initializes extensions after Activity.onResume. Passing the
                // Activity lets AppsFlyer recognize that first foreground session.
                sdk.init(key, new AppsFlyerConversionListener() {
                    @Override
                    public void onConversionDataSuccess(Map<String, Object> conversionData) {
                        appsflyerAddToQueue(CONVERSION_DATA_SUCCESS, new JSONObject(conversionData).toString());
                    }

                    @Override
                    public void onConversionDataFail(String errorMessage) {
                        reportError(CONVERSION_DATA_FAIL, 0, errorMessage, null);
                    }
                }, activity);
                sdk.collectDataFromLauncherActivity(activity);
                initialized = true;
                sdk.registerSessionReadyListener(new SessionReadyListener() {
                    @Override
                    public void onSessionReady() {
                        activity.runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                sessionReady = true;
                                startIfReady();
                            }
                        });
                    }
                });
            }
        });
    }

    private void startIfReady() {
        AppsFlyerLib sdk = AppsFlyerLib.getInstance();
        if (initialized && startRequested && sessionReady && !sdk.isStopped() && sdk.isSessionReady()) {
            sessionReady = false;
            sdk.start(requestListener(null));
        }
    }

    public void startSDK() {
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                startRequested = true;
                startIfReady();
            }
        });
    }

    public void finalizeSDK() {
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                initialized = false;
                startRequested = false;
                sessionReady = false;
                AppsFlyerLib.getInstance().unregisterSessionReadyListener();
                AppsFlyerLib.getInstance().unregisterConversionListener();
            }
        });
    }

    public void setDebugLog(final boolean enabled) {
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AppsFlyerLib.getInstance().setDebugLog(enabled);
            }
        });
    }

    public void logEvent(final String eventName, final Map<String, Object> eventValue) {
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AppsFlyerLib.getInstance().logEvent(activity, eventName, eventValue, requestListener(eventName));
            }
        });
    }

    public void setCustomerUserId(final String userId) {
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AppsFlyerLib.getInstance().setCustomerUserId(userId);
            }
        });
    }

    public String getAppsFlyerUID() {
        return AppsFlyerLib.getInstance().getAppsFlyerUID(activity);
    }

    public String getSDKVersion() {
        return AppsFlyerLib.getInstance().getSdkVersion();
    }

    private static Boolean consentValue(int value) {
        return value < 0 ? null : Boolean.valueOf(value != 0);
    }

    public void setConsentData(final int gdpr, final int dataUsage, final int adsPersonalization, final int adStorage) {
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AppsFlyerLib.getInstance().setConsentData(new AppsFlyerConsent(
                    consentValue(gdpr), consentValue(dataUsage), consentValue(adsPersonalization), consentValue(adStorage)));
            }
        });
    }

    public void enableTCFDataCollection(final boolean enabled) {
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AppsFlyerLib.getInstance().enableTCFDataCollection(enabled);
            }
        });
    }

    public void anonymizeUser(final boolean enabled) {
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AppsFlyerLib.getInstance().anonymizeUser(enabled);
            }
        });
    }

    public void stopSDK(final boolean stopped) {
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AppsFlyerLib sdk = AppsFlyerLib.getInstance();
                if (stopped) startRequested = false;
                sdk.stop(stopped, activity);
                if (!stopped) sessionReady = sdk.isSessionReady();
            }
        });
    }

    public boolean isStopped() {
        return AppsFlyerLib.getInstance().isStopped();
    }

    public void setCurrencyCode(final String currency) {
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AppsFlyerLib.getInstance().setCurrencyCode(currency);
            }
        });
    }

    public void setSharingFilterForPartners(final String[] partners) {
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AppsFlyerLib.getInstance().setSharingFilterForPartners(partners);
            }
        });
    }
}
