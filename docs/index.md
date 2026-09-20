---
title: Defold AppsFlyer extension API documentation
brief: This manual covers AppsFlyer attribution, events, customer IDs, deep links and consent controls on Android and iOS in Defold.
---

# Defold AppsFlyer extension

[AppsFlyer](https://www.appsflyer.com/) native extension for [Defold](https://defold.com/), supporting install attribution, deep links, customer IDs, and in-app events on Android and iOS.

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
- `uri_scheme`: optional custom URI scheme, without `://`, registered on Android and iOS. Leave empty if your own manifests already configure it.

The Android package must match the app registered in AppsFlyer. Android requires minimum SDK 21. AppsFlyer itself requires iOS 12+, while the effective minimum also depends on Defold (Defold 1.14.0 targets iOS 15+).

Requires **Defold 1.14.0 or later** with the iOS scene-delegate API introduced in [Defold PR #13256](https://github.com/defold/defold/pull/13256). While that change is on a development branch, use Bob from that branch and `https://build-stage.defold.com`. Bob requires Java 25. Gradle and CocoaPods resolve the native dependencies. Google Play Install Referrer is included explicitly. The Android AAR supplies its own assets, backup rules and `AD_ID` permission; the iOS pod supplies its privacy manifest. See [AppsFlyer's installation guide](https://dev.appsflyer.com/hc/docs/install-android-sdk-7) when merging custom backup rules or configuring additional stores.

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
| `DEEP_LINK_RESULT` | `{ status = "FOUND", deep_link = table, is_deferred = boolean }`, `{ status = "NOT_FOUND" }`, or `{ status = "ERROR", error = string }`. |

New Lua functions in this upgrade are `get_sdk_version`, `set_consent_data`, `enable_tcf_data_collection`, `anonymize_user`, `stop_sdk`, `is_stopped`, `set_currency_code`, and `set_sharing_filter_for_partners`. The four start/event callback constants are also new. Error codes come from each native SDK and need not match across platforms. Conversion data is a separate asynchronous response, not an event-delivery acknowledgment. Events with the same name have no separate request identifier in this API.

## Deep links

The extension subscribes to AppsFlyer's Unified Deep Linking (UDL) API on both platforms. Register your Lua callback in `init()` to receive `DEEP_LINK_RESULT`. Results are queued until a callback is registered and delivered on the Lua thread. Deep-link resolution can finish before `start_sdk()` or session delivery; the session start gate still applies.

```lua
local function on_appsflyer(self, message_id, message)
    if message_id == appsflyer.DEEP_LINK_RESULT then
        if message.status == "FOUND" then
            local link = message.deep_link
            print("Destination:", link.deep_link_value)
            print("Parameter:", link.deep_link_sub1)
            print("Deferred:", message.is_deferred)
            -- Validate the destination/parameters before routing your game.
        elseif message.status == "ERROR" then
            print("Deep link failed:", message.error)
        end
    end
end
```

`deep_link` preserves the native SDK's click-event fields. Fields other than `deep_link_value` and `deep_link_sub1` through `deep_link_sub10` depend on attribution and privacy rules. `is_deferred` distinguishes post-install resolution from direct links. `NOT_FOUND` and `ERROR` do not contain a `deep_link` table. Error descriptions differ by platform.

For a custom URI scheme, add this setting to your app's `game.project`:

```ini
[appsflyer]
uri_scheme = mygame
```

This registers URLs such as `mygame://open?deep_link_value=level&deep_link_sub1=42`. Choose a scheme for your own app. If your manifests already register it, leave this setting empty.

On iOS, a scene observer is registered before application launch. Cold-start connection URLs/user activities are buffered until SDK initialization; later scene URL and user-activity callbacks are forwarded directly. Keep Defold's default scene manifest and `DefoldSceneDelegate` when customizing `Info.plist`. The extension observes scenes without replacing Defold's window or scene delegate.

Android SDK 7.0.1 currently resolves links on cold launch and when the app returns from the background. A new intent delivered while the app remains in the foreground may not produce a UDL callback.

URI schemes alone do not configure Universal Links, Android App Links or deferred attribution. For those, configure your AppsFlyer OneLink template, iOS Associated Domains/signing and Android verified HTTPS intent filters for your own app/domain. Follow AppsFlyer's [iOS UDL](https://dev.appsflyer.com/hc/docs/dl_ios_unified_deep_linking) and [Android UDL](https://dev.appsflyer.com/hc/docs/dl_android_unified_deep_linking) guides. Custom OneLink domains, link generation and push deep-link configuration are not exposed by the Lua API.

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

The extension exposes 14 functions and seven callback constants. UDL results are supported; typed/nested event values, dedicated ad revenue and receipt validation remain unbound. The [API audit](sdk7-upgrade.md#lua-api-coverage) lists the remaining gaps. See [simulator test instructions](../tests/README.md) for reproducible builds and integration checks.

## Issues and suggestions

Please [open an issue](https://github.com/AGulev/defold-extension-appsflyer/issues) with the Defold version, platform, native SDK version, and sanitized reproduction details.
