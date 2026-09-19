# SDK 7 upgrade and Lua API audit

Verified on 2026-09-19 against AppsFlyer's documentation and published native SDK artifacts.

## Versions and migration

| Platform | Previous | Updated | Authoritative release source |
| --- | --- | --- | --- |
| Android | 6.12.2 | **7.0.1** | [AppsFlyer Maven metadata](https://repo.maven.apache.org/maven2/com/appsflyer/af-android-sdk/maven-metadata.xml) |
| iOS | 6.12.1 | **7.0.2** | [Official GitHub release](https://github.com/AppsFlyerSDK/AppsFlyerFramework/releases/tag/7.0.2), [CocoaPods specification](https://trunk.cocoapods.org/api/v1/pods/AppsFlyerFramework/specs/latest) |

The release artifacts are newer than some developer-guide release-note entries. The dependency versions are pinned, not floating.

SDK 7 changes the session lifecycle: initialization and session delivery are separate, and a session-ready listener must coordinate startup. The extension waits for both native readiness and Lua's `start_sdk()`, then starts each later foreground session. Customer ID must be set again on each cold start. See the official [Android migration](https://dev.appsflyer.com/hc/docs/migrate-android-sdk-to-v7) and [iOS migration](https://dev.appsflyer.com/hc/docs/migrate-ios-sdk-to-v7) guides.

### Changes made

- Android imports the SDK 7 `share` callback interfaces and calls the new `start(listener)` overload. The conversion listener now implements only its supported success/failure methods.
- Android passes the current Activity to `init`. Defold initializes extensions after the Activity has resumed; passing only the application context missed first-launch readiness in the emulator. The Activity overload handles that existing foreground state. Launcher intent data is also collected before start.
- The removed legacy app-open-attribution callbacks are not replaced with a Lua UDL binding in this upgrade. The old iOS implementation emitted an undocumented numeric message `3`; that number now belongs to the documented `START_SUCCESS` callback. Use named constants, and see the deep-linking gap below.
- iOS uses `initWithDevKey:appleAppId:` and retains its weak SDK delegate. The removed iAd framework dependency is gone. Launch options and buffered URL/universal-link arguments survive until SDK initialization.
- SDK mutations and session coordination run on the Android UI thread or iOS main thread. Native asynchronous responses are queued for Lua's update thread.
- Lua receives session/event success and failure callbacks, plus `get_sdk_version()`. Conversion failures on iOS are now forwarded.
- New Lua controls cover explicit consent (including unspecified fields), TCF collection, anonymization, stop/resume, stopped-state queries, default currency and partner-sharing filters. Stopping revokes the foreground-session gate until Lua explicitly resumes it. The Android bridge suppresses duplicate completion calls observed when SDK 7 rejects a stopped event.
- Callback replacement, removal during a callback, startup/shutdown ordering, and JNI string ownership are corrected. Invalid event parameters are rejected before native allocations.
- Empty credentials disable initialization with a clear warning. `start_sdk()`/`log_event()` reject use without initialization. iOS App Store IDs must be numeric.
- Obsolete Android SDK 6 assets were removed. The SDK 7 AAR supplies its current assets and backup resources. CocoaPods supplies the iOS privacy resource bundle.

The [Android installation guide](https://dev.appsflyer.com/hc/docs/install-android-sdk-7) requires API 21+, Kotlin 2+ when using Kotlin, and an explicit Play Install Referrer dependency. This extension includes Install Referrer 2.2. Extra store referrer modules and optional Google advertising-ID/AppSet libraries remain app-specific dependencies. The standalone smoke app does not add the optional Google Play services libraries; AppsFlyer still obtained an advertising ID through its fallback path in the emulator.

AppsFlyer 7.0.2 supports iOS 12+, per its podspec. Defold may impose a higher minimum; the tested 1.13.1 bundles specify iOS 15. Builds and tests use current Extender and Java 25 for Bob.

## Lua API coverage

**Customer ID was already available** as `appsflyer.set_customer_user_id(string)` on both platforms. The sample and API documentation now explain its required placement before the first session. `get_appsflyer_uid()` was also already bound; it returns the AppsFlyer installation identifier, not your customer ID.

The complete exposed surface is 14 functions, documented in the [manual](index.md) and [editor API reference](../extension-appsflyer/api/appsflyer.script_api). The eight added functions are `get_sdk_version`, `set_consent_data`, `enable_tcf_data_collection`, `anonymize_user`, `stop_sdk`, `is_stopped`, `set_currency_code`, and `set_sharing_filter_for_partners`. Existing customer-ID support is retained. Four new constants report session/event success and failure.

The following gaps were checked against the shipped Android 7.0.1 `AppsFlyerLib` public methods and iOS 7.0.2 public headers, alongside the [Android API reference](https://dev.appsflyer.com/hc/docs/android-sdk-reference-appsflyerlib) and [iOS API reference](https://dev.appsflyer.com/hc/docs/ios-sdk-reference-appsflyerlib). Some reference examples still show SDK 6 initialization; the SDK 7 migration guides and shipped headers take precedence for changed signatures.

| Missing Lua capability | Native APIs / examples | Consequence |
| --- | --- | --- |
| Identifier/network controls | `setDisableAdvertisingIdentifiers`, `setDisableNetworkData`, platform-specific ID controls | Per-app privacy configuration needs native integration or supported SDK configuration files. |
| Unified Deep Linking / OneLink | Android `subscribeForDeepLink`, iOS `deepLinkDelegate`, OneLink domains, link generation and push deep links | There is no Lua UDL result callback. Existing iOS URL forwarding alone is insufficient for gameplay routing. |
| Dedicated ad revenue | `logAdRevenue`, mediation/revenue data objects | Generic events cannot express the full dedicated ad-revenue API. |
| Purchase validation | `validateAndLogInAppPurchase` and purchase connectors | `af_purchase` records an event but does not validate a transaction. |
| Rich event values | Native maps/dictionaries support richer values | Lua currently supports only flat string/number values, and serializes numbers as strings; nested items, booleans and arrays are absent. |
| Extra user identifiers | SDK 7 email, phone, name, Facebook login ID, `clearUserPii` | Only customer ID is currently exposed. |
| Additional/partner data | `setAdditionalData`, `setPartnerData` | Arbitrary custom metadata and partner payloads remain unbound. Currency and partner-sharing filters are now exposed. |
| Uninstall attribution | Android uninstall-token updates; iOS device-token registration | Push registration must be connected separately. |
| Advanced platform configuration | Session interval, host, store/preinstall data, iOS ATT/SKAN controls | These features are outside the current binding. ATT prompting itself belongs to the app's ATT integration. |

Suggested order for future bindings: UDL callbacks; typed/nested event values; ad revenue and purchase validation. Add other APIs when required by a concrete app integration. This upgrade does not claim full SDK parity.

## Validation

See [tests/README.md](../tests/README.md) for reproducible builds and [test results](test-results.md) for the actual devices and outcomes. Simulator testing verifies the extension, initialization, request callbacks and server delivery. It does not establish production ad attribution, IDFA/ATT behavior, SKAdNetwork postbacks, Play Install Referrer campaigns or purchase validation on physical devices.
