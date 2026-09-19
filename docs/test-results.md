# SDK 7 validation results

Run on 2026-09-19 with Defold 1.13.1 (`574678c7d44be490d874fbed2d0ae6211feec4d9`), Java 25, and the official Extender service. Native source uploads were authorized. Builds used blank Dev Keys; live credentials were inserted and bundles signed locally.

| Check | Android | iOS |
| --- | --- | --- |
| Native compilation | ARM32 and ARM64 passed | ARM64 device and x86_64 simulator passed |
| Runtime SDK version | 7.0.1, build 386 | 7.0.2, build 1 |
| Test runtime | Pixel 9a AVD, Android 17/API 37.1, ARM64, 16 KB pages | iPhone 16 Pro simulator, iOS 18.6, x86_64 via Rosetta |
| Missing credentials | Passed: warning and Lua start error | Passed: warning and Lua start error |
| Invalid Lua event values and new API arguments | Passed | Passed |
| Consent/filter/currency setters and stop/resume | Passed; request payload checked | Passed through native bridge on invalid-configuration path |
| Event rejection while stopped | Passed; one Lua error result | Passed; one Lua error result |
| Deferred startup and duplicate start calls | Passed; one session callback before backgrounding | Passed with invalid configuration; one session error callback |
| Callback replacement during delivery | Passed | Passed |
| Stable AppsFlyer UID returned to Lua | Passed | Passed |
| Successful server delivery | **Passed**, including dashboard confirmation | **Pending test-app registration** |
| Customer ID in accepted events | **Confirmed in AppsFlyer Live Event Viewer** | Pending successful delivery |
| Subsequent foreground session | Passed | Passed on the invalid-configuration path; live success pending |
| Invalid-configuration error callbacks | Missing-key path tested | Session, event and conversion errors returned to Lua |

## Android live evidence

AppsFlyer's Live Event Viewer recorded a Launch at **11:31:53** and the in-app event `defold_sdk_smoke` at **11:31:55** (Europe/Stockholm), for the registered Android test app `io.refold.appsflyer.sdk7test`. Its event details showed SDK version **7.0.1** and customer user ID **`defold_sdk7_smoke`**. The emulator was registered as a test device using the advertising ID reported by the SDK.

The initial live-delivery smoke script produced:

```text
AF_SMOKE SDK_VERSION version: 7.0.1 (build 386)
AF_SMOKE INPUT_VALIDATION_PASS
AF_SMOKE START_REQUESTED
AF_SMOKE START_SUCCESS {}
AF_SMOKE CALLBACK_REPLACEMENT_PASS
AF_SMOKE UID_OK
AF_SMOKE CONVERSION_DATA_SUCCESS ... af_status: Organic ...
AF_SMOKE EVENT_SUCCESS {"event_name":"defold_sdk_smoke"}
AF_SMOKE DELIVERY_PASS
AF_SMOKE RESULT PASS session_callbacks 1
```

Backgrounding and reopening the app produced a second `START_SUCCESS`. The outgoing native session/event payloads also contained `appUserId: defold_sdk7_smoke`.

The SDK Information aggregate dashboard initially showed no data; it was not used as evidence of failure. The Live Event Viewer provided direct confirmation of received events.

## iOS status

The simulator runs the actual Defold extension and smoke collection. Rosetta was installed to run Defold's Intel simulator engine on this Apple Silicon host. Missing-key handling passed. With deliberately invalid configuration, the native SDK reached AppsFlyer and returned HTTP 404, surfaced as `START_FAIL` and `EVENT_FAIL` (native error code 40), plus `CONVERSION_DATA_FAIL` with `App ID is incorrect`. Callback replacement, UID checks, and a subsequent foreground session callback passed on that path. The smoke script correctly reported `RESULT FAIL` because successful delivery was not expected with invalid configuration.

The planned iOS debug app uses dashboard ID `id111126919`, numeric SDK ID `111126919`. It was not registered during this run. Successful iOS event delivery is therefore **not yet verified**; the invalid-configuration test must not be treated as a live-delivery pass.

## Added API validation

The final build includes all 14 Lua functions. ARM32/ARM64 Android and ARM64-device/x86_64-simulator iOS compilation passed after the additions. Both simulators passed `NEW_API_VALIDATION_PASS`, `NEW_API_CONFIG_PASS`, `STOPPED_EVENT_PASS` and `STOP_RESUME_PASS`. These checks cover malformed consent/partner tables, non-boolean controls, currency format, optional consent fields, filter reset, native stopped-state queries and rejection of an event while stopped.

The final Android live run at **12:10** returned `START_SUCCESS`, `EVENT_SUCCESS`, `DELIVERY_PASS` and `RESULT PASS`. Exactly one stopped-event error reached Lua. Backgrounding and reopening the final build after stop/resume produced another `START_SUCCESS`. SDK 7.0.1 originally invoked that request's error callback twice; the bridge now forwards only the first completion.

The accepted event request contained these inspected fields (other fields are omitted):

```json
{
  "appUserId": "defold_sdk7_smoke",
  "currency": "EUR",
  "sharing_filter": ["all"],
  "consent_data": {
    "manual": {
      "gdpr_applies": true,
      "ad_user_data_enabled": true,
      "ad_personalization_enabled": false
    }
  }
}
```

Ad-storage consent was intentionally omitted in Lua and remained absent from the native payload. These are synthetic choices for the test app. The TCF flag was toggled, but actual CMP/TCF-string extraction was not tested. Anonymization was toggled before delivery; a separate anonymized request was not tested. Partner-filter forwarding was checked; downstream partner behavior was not tested.

On iOS the same new API assertions passed, including one stopped-event failure (native code 11) and a new session attempt after resumption. Its session/event requests still returned the expected HTTP 404 with invalid test configuration, so `RESULT FAIL` is the correct delivery result. Successful iOS delivery and payload contents remain unverified.

## Other checks and limits

- The Android bundle contains the four current SDK assets, byte-for-byte matching the official 7.0.1 AAR, and the SDK's backup XML resources.
- iOS device/simulator bundles include AppsFlyer's privacy resource bundle.
- YAML manifests, API documentation and CI workflow parse successfully. The credential-injection helper compiles, and was exercised for both APK and .app packaging.
- No actual Dev Key is present in the repository changes. Credentials and raw logs are kept outside the repository under `/private/tmp/appsflyer-upgrade`.
- These tests do not validate campaign attribution from ad clicks, ATT/IDFA, SKAdNetwork, store purchase validation, or physical-device behavior.

See [testing instructions](../tests/README.md) to reproduce or complete the live iOS test.
