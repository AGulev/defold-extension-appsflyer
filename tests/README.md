# Simulator smoke tests

The smoke collection exercises the actual Lua/C++/Java or Objective-C bridge and the native SDK. Live tests need a registered AppsFlyer test app and its Dev Key. AppsFlyer does not publish a shared test key. Follow the official [integration testing](https://dev.appsflyer.com/hc/docs/integration-testing), [Android testing](https://dev.appsflyer.com/hc/docs/testing-android), and [iOS testing](https://dev.appsflyer.com/hc/docs/testing-ios) guides.

## Preparation

Install Java 25, Defold 1.13.1 Bob, and the Android SDK and/or Xcode. Native extension builds use Extender and upload source code. Keep `appsflyer.key` blank in the project and build inputs; `inject_credentials.py` inserts it into the completed bundle locally.

1. Register an Android pending/unpublished app whose ID matches your test package.
2. Register an iOS debug app using AppsFlyer's [debug-app instructions](https://dev.appsflyer.com/hc/docs/testing-ios#creating-an-ios-debug-app). The fake numeric ID must have nine digits and begin with `1111`. The dashboard displays an `id` prefix; the SDK setting omits it.
3. Copy `credentials.settings.example` outside the repository, set the platform's Dev Key and numeric iOS ID, and restrict the file to your user (`chmod 600`). Do not pass this file to Extender.
4. Put only nonsecret identifiers in a separate `app.settings` file:

```ini
[android]
package = YOUR_REGISTERED_TEST_PACKAGE
[ios]
bundle_identifier = YOUR_TEST_BUNDLE_ID
[appsflyer]
apple_app_id = YOUR_NUMERIC_IOS_TEST_ID
```

Run the following from the project directory. Replace `/tmp/af-test`, the Bob path, package IDs and device IDs with your own. Use separate project copies/output directories for concurrent platform builds. Paths in `--settings` are resolved against Bob's working directory.

## Android

```sh
java -jar /path/to/bob.jar \
  --platform arm64-android --architectures arm64-android \
  --variant debug --archive --bundle-format apk \
  --settings tests/smoke.settings --settings /tmp/af-test/app.settings \
  --build-server https://build.defold.com \
  --bundle-output /tmp/af-test/android resolve build bundle
```

Install this unmodified APK first to exercise missing-key handling. It should print `AF_SMOKE MISSING_KEY_PASS` and should not send a session.

For the live test, insert the Dev Key locally:

```sh
python3 tests/inject_credentials.py \
  --credentials /tmp/af-test/android-credentials.settings \
  /tmp/af-test/android/Appsflyer/Appsflyer.apk \
  /tmp/af-test/android-unsigned.apk

/path/to/android/build-tools/zipalign -f -p 4 \
  /tmp/af-test/android-unsigned.apk /tmp/af-test/android-test.apk
/path/to/android/build-tools/apksigner sign \
  --ks debug.keystore --ks-pass file:debug.keystore.pass.txt \
  --ks-key-alias androiddebugkey /tmp/af-test/android-test.apk

adb -s YOUR_EMULATOR install -r /tmp/af-test/android-test.apk
adb -s YOUR_EMULATOR shell am force-stop YOUR_REGISTERED_TEST_PACKAGE
adb -s YOUR_EMULATOR shell am start \
  -n YOUR_REGISTERED_TEST_PACKAGE/com.dynamo.android.DefoldActivity
adb -s YOUR_EMULATOR logcat -s defold
```

Bob creates the example debug keystore when bundling. For a different project, use its development signing key. This signing flow is for test APKs only.

## iOS simulator

Defold 1.13.1 supplies an x86_64 simulator engine. Apple Silicon hosts need Rosetta and a runtime supporting Intel simulator apps; this was tested with iOS 18.6. The production device architecture is `arm64-ios`.

```sh
java -jar /path/to/bob.jar \
  --platform arm64-ios --architectures x86_64-ios \
  --variant debug --archive \
  --settings tests/smoke.settings --settings /tmp/af-test/app.settings \
  --build-server https://build.defold.com \
  --bundle-output /tmp/af-test/ios resolve build bundle

codesign --force --deep --sign - /tmp/af-test/ios/Appsflyer.app
xcrun simctl install YOUR_SIMULATOR /tmp/af-test/ios/Appsflyer.app
xcrun simctl launch --arch=x86_64 --console YOUR_SIMULATOR YOUR_TEST_BUNDLE_ID
```

The unmodified bundle should print `AF_SMOKE MISSING_KEY_PASS`. Insert credentials into a fresh copy for the live test:

```sh
python3 tests/inject_credentials.py \
  --credentials /tmp/af-test/ios-credentials.settings \
  /tmp/af-test/ios/Appsflyer.app /tmp/af-test/AppsflyerTest.app
codesign --force --deep --sign - /tmp/af-test/AppsflyerTest.app
xcrun simctl install YOUR_SIMULATOR /tmp/af-test/AppsflyerTest.app
xcrun simctl launch --arch=x86_64 --console YOUR_SIMULATOR YOUR_TEST_BUNDLE_ID
```

For a device compilation check, build with `--architectures arm64-ios` or `arm64-ios,x86_64-ios`. Installing on a physical device requires your own signing/provisioning.

## Expected results

With valid credentials, keep the app in the foreground for 40 seconds:

- SDK version contains Android `7.0.1` or iOS `7.0.2`.
- `INPUT_VALIDATION_PASS` verifies invalid Lua event tables are rejected.
- `NEW_API_VALIDATION_PASS` verifies consent types/field names, booleans, currency format and partner-array validation.
- `NEW_API_CONFIG_PASS` exercises consent objects (including omitted fields), TCF/anonymization toggles, partner-filter set/reset, currency and native stopped-state queries. A stopped SDK must not start; clearing its flag alone must not reopen the Lua start gate.
- A three-second delay verifies there is no session delivery before Lua starts the SDK.
- Two consecutive `start_sdk()` calls produce one session response.
- `START_SUCCESS`, `EVENT_SUCCESS`, `UID_OK`, `CALLBACK_REPLACEMENT_PASS`, then `DELIVERY_PASS` and `RESULT PASS` confirm the live flow.
- `CONVERSION_DATA_SUCCESS` arrives independently and may contain cached attribution on subsequent launches.
- At 12 seconds the test stops the SDK; an event attempted while stopped must produce one failure (`STOPPED_EVENT_PASS`). At 16 seconds it resumes (`STOP_RESUME_PASS`). A resumed session waits for native readiness; Android may need another foreground cycle.

Keep the simulator app active and dismiss any system dialogs before evaluating timer-based assertions. Background the app for more than five seconds and reopen it to check that another session succeeds. On iOS, temporarily opening the simulator's built-in Settings app and then reopening the test app exercises this without changing any settings.

The outgoing session and in-app event should contain customer ID `defold_sdk7_smoke`, currency `EUR`, sharing filter `["all"]`, and manual consent with GDPR/data usage true and ads personalization false. Ad-storage consent is deliberately omitted and must remain absent. Confirm received sessions/events and their customer ID in AppsFlyer's Live Event Viewer; the aggregate dashboard can lag behind. The viewer also displays manual consent, but does not expose every native configuration field. These are synthetic choices for this test app. TCF extraction itself requires a CMP and is not covered by toggling its flag. Keep raw debug logs private: the native SDK may print the Dev Key. Do not commit test credentials, signed test bundles or raw logs.

For repeated install-attribution testing, [register the emulator/simulator as a test device](https://support.appsflyer.com/hc/en-us/articles/207031996-Registering-test-devices) using a supported identifier before reinstalling. Ad-campaign attribution, ATT/IDFA, SKAN and purchases need their own physical-device/store tests.

Deep-link testing is separate from this smoke test and was deferred for this upgrade.
