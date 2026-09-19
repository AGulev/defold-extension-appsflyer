# Simulator smoke tests

The smoke collection exercises the actual Lua/C++/Java or Objective-C bridge and the native SDK. Live tests need a registered AppsFlyer test app and its Dev Key. AppsFlyer does not publish a shared test key. Follow the official [integration testing](https://dev.appsflyer.com/hc/docs/integration-testing), [Android testing](https://dev.appsflyer.com/hc/docs/testing-android), and [iOS testing](https://dev.appsflyer.com/hc/docs/testing-ios) guides.

## Preparation

Install Java 25, Defold 1.14.0 Bob with the scene-delegate API from [Defold PR #13256](https://github.com/defold/defold/pull/13256), and the Android SDK and/or Xcode. Native extension builds use Extender and upload source code. Keep `appsflyer.key` blank in the project and build inputs; `inject_credentials.py` inserts it into the completed bundle locally.

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

The shared GitHub workflow pins the PR's Bob at commit `97c0cda31740591125d4347d916f68a87030d367`. Download that Bob for the commands below and check `java -jar /path/to/bob.jar --version`:

```sh
curl -fL -o /tmp/bob-scene.jar \
  https://d.defold.com/archive/dev/97c0cda31740591125d4347d916f68a87030d367/bob/bob.jar
```

Development archives expire; once the scene change reaches alpha/release, switch the workflow and local builds to that published version.

Run the following from the project directory. Replace `/tmp/af-test`, the Bob path, package IDs and device IDs with your own. Use separate project copies/output directories for concurrent platform builds. Paths in `--settings` are resolved against Bob's working directory.

## Android

```sh
java -jar /path/to/bob.jar \
  --platform arm64-android --architectures arm64-android \
  --variant debug --archive --bundle-format apk \
  --settings tests/smoke.settings --settings /tmp/af-test/app.settings \
  --build-server https://build-stage.defold.com \
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

The PR supplies a native Apple Silicon simulator platform, `arm64_sim-ios`. The production device platform is `arm64-ios`; build it separately.

```sh
java -jar /path/to/bob.jar \
  --platform arm64_sim-ios --architectures arm64_sim-ios \
  --variant debug --archive \
  --settings tests/smoke.settings --settings /tmp/af-test/app.settings \
  --build-server https://build-stage.defold.com \
  --bundle-output /tmp/af-test/ios resolve build bundle

codesign --force --deep --sign - /tmp/af-test/ios/Appsflyer.app
xcrun simctl install YOUR_SIMULATOR /tmp/af-test/ios/Appsflyer.app
xcrun simctl launch --console YOUR_SIMULATOR YOUR_TEST_BUNDLE_ID
```

The unmodified bundle should print `AF_SMOKE MISSING_KEY_PASS`. Insert credentials into a fresh copy for the live test:

```sh
python3 tests/inject_credentials.py \
  --credentials /tmp/af-test/ios-credentials.settings \
  /tmp/af-test/ios/Appsflyer.app /tmp/af-test/AppsflyerTest.app
codesign --force --deep --sign - /tmp/af-test/AppsflyerTest.app
xcrun simctl install YOUR_SIMULATOR /tmp/af-test/AppsflyerTest.app
xcrun simctl launch --console YOUR_SIMULATOR YOUR_TEST_BUNDLE_ID
```

For a device compilation check, build with `--platform arm64-ios --architectures arm64-ios`. Installing on a physical device requires your own signing/provisioning.

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

## Deep links

Use `--settings tests/deeplink.settings` in place of `tests/smoke.settings` in the build commands above. This selects a dedicated test collection and registers the `defold-appsflyer-test` URI scheme on both platforms. Inject credentials locally and sign/install as above. Keep logs private; no test-results file is needed in the repository.

Run each scenario with a different `deep_link_sub1` token:

1. **Cold start:** terminate/force-stop the test app, then open the URL.
2. **Background:** launch the app normally, put it in the background for at least five seconds, then open the URL.
3. **Foreground:** with the app active, open another URL.
4. Open two distinct URLs in succession to check that each delivers once and parameters are not replaced by an earlier link.

On Android, keep the URL quoted for the device shell as well as the host shell:

```sh
adb -s YOUR_EMULATOR shell am force-stop YOUR_REGISTERED_TEST_PACKAGE
adb -s YOUR_EMULATOR shell 'am start -W -a android.intent.action.VIEW -c android.intent.category.BROWSABLE -d "defold-appsflyer-test://open?deep_link_value=cold&deep_link_sub1=android-cold-1" YOUR_REGISTERED_TEST_PACKAGE'
```

On iOS, accept the system’s “Open in AppsFlyer?” prompt if shown (manually or with XCUITest):

```sh
xcrun simctl terminate YOUR_SIMULATOR YOUR_TEST_BUNDLE_ID
xcrun simctl openurl YOUR_SIMULATOR \
  'defold-appsflyer-test://open?deep_link_value=cold&deep_link_sub1=ios-cold-1'
```

The iOS scene paths should cover all four cases. Android SDK 7.0.1 resolves cold and background launches, but currently misses new intents delivered while the app remains in the foreground; include that case to detect when native support changes.

A successful direct link prints `AF_DEEPLINK FOUND_PASS`, the exact destination and test token, and `false` for `is_deferred`. The collection rejects duplicate tokens and malformed callback payloads. It also logs a `defold_deeplink` event; expect `AF_DEEPLINK EVENT_SUCCESS` once session delivery is available. A normal launch may report `NOT_FOUND_PASS`; it must not fabricate a found link. Check logs for Lua assertion errors as well as pass markers. Compare every sent token with the received tokens to detect missing results.

These custom-scheme checks exercise actual operating-system URL delivery and native AppsFlyer resolution. For Universal Links/App Links, separately configure a OneLink template and platform domain associations and repeat using its HTTPS URL. Deferred attribution additionally requires a fresh installation and an eligible test-device/store flow; a direct simulator link does not verify it.
