package com.defold.appsflyer;

import android.util.Log;
import android.app.Activity;

import com.appsflyer.AppsFlyerConversionListener;
import com.appsflyer.AppsFlyerLib;

import java.util.Map;

import org.json.JSONObject;
import org.json.JSONException;

public class AppsflyerJNI {
    private static final String TAG = "AppsflyerJNI";

    public static native void appsflyerAddToQueue(int msg, String json);

    private static final int CONVERSION_DATA_SUCCESS = 1;
    private static final int CONVERSION_DATA_FAIL = 2;

    private Activity activity;

    public AppsflyerJNI(Activity activity) {
        this.activity = activity;
    }

    public void initializeSDK(final String key) {
        Log.d(TAG, "Initialize SDK");
        AppsFlyerConversionListener conversionDataListener =
                new AppsFlyerConversionListener() {
                    @Override
                    public void onConversionDataSuccess(Map<String, Object> conversionData) {
                        Log.d(TAG, "onConversionDataSuccess");
                        try {
                            JSONObject obj = new JSONObject();
                            for (Map.Entry<String, Object> entry : conversionData.entrySet()) {
                                Object value = entry.getValue();
                                if (value != null) {
                                    obj.put(entry.getKey(), value);
                                }
                            }
                            appsflyerAddToQueue(CONVERSION_DATA_SUCCESS, obj.toString());
                        } catch (JSONException e) {
                            Log.e(TAG, "Unable to encode message data: " + e.getLocalizedMessage());
                        }
                    }

                    @Override
                    public void onConversionDataFail(String errorMessage) {
                        Log.d(TAG, "onConversionDataFail: " + errorMessage);
                        try {
                            JSONObject obj = new JSONObject();
                            obj.put("error", errorMessage);
                            appsflyerAddToQueue(CONVERSION_DATA_FAIL, obj.toString());
                        } catch (JSONException e) {
                            Log.e(TAG, "Unable to encode CONVERSION_DATA_FAIL message data: " + e.getLocalizedMessage());
                        }
                    }

                    @Override
                    public void onAppOpenAttribution(Map<String, String> conversionData) {
                        Log.d(TAG, "onConversionDataSuccess");
                    }

                    @Override
                    public void onAttributionFailure(String errorMessage) {
                        Log.d(TAG, "onConversionDataFail: " + errorMessage);
                    }
                };
        AppsFlyerLib.getInstance().init(key, conversionDataListener, activity.getApplicationContext());
    }

    public void startSDK() {
        Log.d(TAG, "Start SDK");
        AppsFlyerLib.getInstance().start(activity);
    }

    public void setDebugLog(boolean is_enable) {
        Log.d(TAG, "Set debug log: " + String.valueOf(is_enable));
        AppsFlyerLib.getInstance().setDebugLog(is_enable);
    }

    public void logEvent(String eventName, Map<String, Object> eventValue) {
        Log.d(TAG, "Log event: " + eventName);
        AppsFlyerLib.getInstance().logEvent(activity, eventName, eventValue);
    }
}
