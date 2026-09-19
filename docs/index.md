---
title: Defold AppsFlyer extension API documentation
brief: This manual covers AppsFlyer attribution, events, customer IDs and consent controls on Android and iOS in Defold.
---

# Defold AppsFlyer extension

[AppsFlyer](https://www.appsflyer.com/) native extension for [Defold](https://defold.com/), supporting install attribution, customer IDs, and in-app events on Android and iOS.

Uses **Android SDK 7.0.1** and **iOS SDK 7.0.2**. See the [editor API reference](../extension-appsflyer/api/appsflyer.script_api), [SDK 7 migration and Lua API audit](sdk7-upgrade.md) and [simulator testing instructions](../tests/README.md).

## Setup

Add a [Defold library dependency](https://defold.com/manuals/libraries/) using a ZIP URL from this repository's [releases](https://github.com/AGulev/defold-extension-appsflyer/releases), or the development branch:

```text
https://github.com/AGulev/defold-extension-appsflyer/archive/master.zip
```

Add these settings to `game.project`:

```ini
[appsflyer]
key = YOUR_APPSFLYER_DEV_KEY
apple_app_id = YOUR_NUMERIC_APP_STORE_ID
is_debug = 0
```

- `key`: the Dev Key for your registered AppsFlyer app. There is no shared public test key.
- `apple_app_id`: required on iOS; the numeric App Store ID, without the `id` prefix. It is different from your bundle identifier.
- `is_debug`: `1` enables SDK diagnostics; use `0` for release builds. Debug output can include the Dev Key and identifiers.
- `android_channel` and `android_af_store`: optional Android `CHANNEL` and `AF_STORE` manifest metadata.

The Android package must match the app registered in AppsFlyer. Android requires minimum SDK 21. AppsFlyer itself requires iOS 12+, while the effective minimum also depends on Defold (the tested Defold 1.13.1 bundles target iOS 15+).

Use a current Defold/Extender toolchain. This upgrade is tested with Defold 1.13.1; its Bob requires Java 25. Gradle and CocoaPods resolve the native dependencies. Google Play Install Referrer is included explicitly. The Android AAR supplies its own assets, backup rules and `AD_ID` permission; the iOS pod supplies its privacy manifest. See [AppsFlyer's installation guide](https://dev.appsflyer.com/hc/docs/install-android-sdk-7) when merging custom backup rules or configuring additional stores.

## Initialize and start

The extension initializes the native SDK during app startup if the required settings are present. **Call `start_sdk()` from Lua to enable session delivery**, after setting your customer ID and completing any required consent/ATT flow. SDK 7 readiness is handled by the extension. Subsequent foreground sessions are started automatically after this initial Lua call.

```lua
local function on_appsflyer(self, message_id, message)
    if message_id == appsflyer.START_SUCCESS then
        print("Session delivered")
    elseif message_id == appsflyer.EVENT_SUCCESS then
        print("Event delivered:", message.event_name)
    elseif message_id == appsflyer.CONVERSION_DATA_SUCCESS then
        pprint(message)
    elseif message.error then
        print("AppsFlyer error:", message.code, message.error)
    end
end

function init(self)
    if not appsflyer then return end -- Android and iOS only.
    appsflyer.set_callback(on_appsflyer)
    appsflyer.set_customer_user_id("your_customer_id")
    appsflyer.start_sdk()
end

function final(self)
    if appsflyer then appsflyer.set_callback(nil) end
end
```

Set the customer ID on **every cold start**, before `start_sdk()`, to associate it with the install/session. SDK 7 no longer persists it across process restarts.

## Lua API

| Function | Behavior |
| --- | --- |
| `start_sdk()` | Enables delivery once SDK session readiness is satisfied. Repeated calls in the same foreground cycle do not duplicate the session. Raises a Lua error if required configuration is missing. |
| `set_callback(function_or_nil)` | Sets one callback `(self, message_id, message)`. Passing `nil` removes it. Callbacks run during Defold's update on the Lua thread. |
| `set_customer_user_id(user_id)` | Sets your own string customer ID on both platforms. This API already existed before the SDK 7 upgrade. |
| `get_appsflyer_uid()` | Returns the AppsFlyer installation ID, or `nil` if unavailable. This is distinct from your customer ID. |
| `get_sdk_version()` | Returns the native SDK version string, which may include build information. |
| `set_debug_log(enabled)` | Enables/disables SDK debug output. |
| `log_event(name, parameters)` | Logs an in-app event. Parameters may be omitted or `nil`, or a flat table with string keys and string/number values. Numbers retain the extension's existing string serialization. Other value types raise a Lua error. |
| `set_consent_data(consent)` | Sends explicit optional consent booleans; omitted fields remain unspecified. Set before startup and when choices change. |
| `enable_tcf_data_collection(enabled)` | Enables/disables reading TCF consent stored by a compatible CMP. |
| `anonymize_user(enabled)` | Enables/disables SDK anonymization. Requests continue. |
| `stop_sdk(stopped)` | Sets the native stopped flag. `true` also revokes automatic foreground sessions. Resume with `stop_sdk(false)` followed by `start_sdk()`. |
| `is_stopped()` | Reads the native stopped flag. UI-thread setter calls are asynchronous, so an immediate read can return the previous state. |
| `set_currency_code(currency)` | Sets the default revenue currency, using an uppercase three-letter ISO 4217 code. |
| `set_sharing_filter_for_partners(partners)` | Excludes exact partner IDs from data sharing; `{ "all" }` excludes all partners and `{}` resets the filter. |

```lua
appsflyer.log_event("af_level_achieved", { af_level = 2, af_score = 100 })
appsflyer.log_event("af_purchase", {
    af_currency = "USD",
    af_content_id = "item_id",
    af_revenue = 1.99,
})
```

This purchase event only records revenue; it does not validate a receipt.

| Callback constant | Message |
| --- | --- |
| `CONVERSION_DATA_SUCCESS` | SDK conversion-data table, including fields such as `af_status` and `is_first_launch`. |
| `CONVERSION_DATA_FAIL` | `{ error = string, code = number }`. Android conversion errors use code `0`. |
| `START_SUCCESS` | Empty table; the SDK reports successful session delivery. |
| `START_FAIL` | `{ error = string, code = number }`. |
| `EVENT_SUCCESS` | `{ event_name = string }`. |
| `EVENT_FAIL` | `{ event_name = string, error = string, code = number }`. |

New Lua functions in this upgrade are `get_sdk_version`, `set_consent_data`, `enable_tcf_data_collection`, `anonymize_user`, `stop_sdk`, `is_stopped`, `set_currency_code`, and `set_sharing_filter_for_partners`. The four start/event callback constants are also new. Error codes come from each native SDK and need not match across platforms. Conversion data is a separate asynchronous response, not an event-delivery acknowledgment. Events with the same name have no separate request identifier in this API.

## Consent and collection controls

Provide the user's current consent choices on every cold start, before `start_sdk()`, and update them when they change:

```lua
local function apply_consent(choices)
    appsflyer.set_consent_data({
        is_user_subject_to_gdpr = choices.gdpr,
        has_consent_for_data_usage = choices.data_usage,
        has_consent_for_ads_personalization = choices.personalized_ads,
        has_consent_for_ad_storage = choices.ad_storage,
    })
    appsflyer.start_sdk()
end
```

Each field is an optional boolean. `false` is an explicit negative choice; omitted or `nil` remains unspecified in the native consent object. An empty table forwards all four fields as unspecified. Unknown fields and non-boolean values raise Lua errors. The extension does not display a consent dialog or persist the user's choices. Supplying consent signals does not itself stop session delivery.

If your consent management platform stores TCF data, enable collection before startup with `appsflyer.enable_tcf_data_collection(true)`. This does not create a TCF string. Explicit signals passed to `set_consent_data` take precedence. See AppsFlyer's [iOS consent guide](https://dev.appsflyer.com/hc/hc/docs/ios-send-consent-for-dma-compliance-7) and the [Android API reference](https://dev.appsflyer.com/hc/docs/android-sdk-reference-appsflyerlib#setconsentdata).

`appsflyer.anonymize_user(true)` enables the native SDK's anonymization mode. Apply it before the first session when required. Requests continue in this mode; it does not remove previously delivered data.

To stop SDK delivery, then later resume it:

```lua
appsflyer.stop_sdk(true)
-- Later, when the application intends to resume:
appsflyer.stop_sdk(false)
appsflyer.start_sdk()
```

Stopping also revokes the extension's automatic foreground-session gate. Clearing the stopped flag alone does not reopen that gate. `start_sdk()` alone does not clear the stopped flag. On resumption the extension waits for native session readiness; Android may wait until the next foreground cycle. `is_stopped()` reads the native flag, not request success or session readiness. Native setters are queued in order on the UI/main thread, so a getter immediately after a setter may still see the previous value. Reapply the intended collection settings on each cold start.

## Currency and partner sharing

```lua
appsflyer.set_currency_code("EUR")
appsflyer.set_sharing_filter_for_partners({ "examplePartner1_int" })
appsflyer.set_sharing_filter_for_partners({ "all" }) -- Exclude every partner.
appsflyer.set_sharing_filter_for_partners({})        -- Reset the filter.
```

Set these options before `start_sdk()` to affect the first session, and reapply on each cold start. An event's `af_currency` can specify its own revenue currency. Currency format is validated as three uppercase letters; the binding does not maintain an ISO currency registry.

Partner lists must be contiguous Lua arrays starting at index 1. Each entry must be an exact AppsFlyer partner ID containing 1–45 ASCII letters, digits or underscores. Invalid values, named keys and sparse arrays raise Lua errors. Excluding partners does not stop requests to AppsFlyer. See the native [Android](https://dev.appsflyer.com/hc/docs/android-sdk-reference-appsflyerlib#setsharingfilterforpartners) and [iOS](https://dev.appsflyer.com/hc/docs/ios-sdk-reference-appsflyerlib#setsharingfilterforpartners) documentation.

## API coverage and tests

The extension exposes 14 functions and six callback constants. Unified Deep Linking, typed/nested event values, dedicated ad revenue and receipt validation remain unbound. The [API audit](sdk7-upgrade.md#lua-api-coverage) lists the remaining gaps. See [test results](test-results.md) for the tested architectures, simulator runs and live-delivery status.

## Issues and suggestions

Please [open an issue](https://github.com/AGulev/defold-extension-appsflyer/issues) with the Defold version, platform, native SDK version, and sanitized reproduction details.
