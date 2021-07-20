package com.agulev.defappsflyer;

import android.app.Activity;

import com.appsflyer.AppsFlyerConversionListener;
import com.appsflyer.AppsFlyerLib;

import java.util.Map;

public class DefAppsFlyer {

    public static void DefAppsFlyer_setAppsFlyerKey(final Activity appActivity, final String appsFlyerKey) {
        AppsFlyerConversionListener conversionDataListener =
                new AppsFlyerConversionListener() {
                    @Override
                    public void onConversionDataSuccess(Map<String, Object> conversionData) {
                    }

                    @Override
                    public void onConversionDataFail(String errorMessage) {
                    }

                    @Override
                    public void onAppOpenAttribution(Map<String, String> conversionData) {
                    }

                    @Override
                    public void onAttributionFailure(String errorMessage) {
                    }
                };
        AppsFlyerLib.getInstance().init(appsFlyerKey, conversionDataListener, appActivity.getApplicationContext());
        AppsFlyerLib.getInstance().start(appActivity);
    }

    public static void DefAppsFlyer_setIsDebug(boolean debugMode) {
        AppsFlyerLib.getInstance().setDebugLog(debugMode);
    }

     public static void DefAppsFlyer_setCustomerId(String customerId) {
        AppsFlyerLib.getInstance().setCustomerUserId(customerId);
    }

    public static void DefAppsFlyer_trackEvent(Activity appActivity, String eventName, Map<String, Object> eventValue) {
        AppsFlyerLib.getInstance().logEvent(appActivity, eventName, eventValue);
    }
}
