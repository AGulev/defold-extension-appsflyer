# DefAppsFlyer

[![Build Status](https://github.com/AGulev/defold-extension-appsflyer/workflows/Build%20with%20bob/badge.svg)](https://github.com/AGulev/defold-extension-appsflyer/actions)

[AppsFlyer](https://www.appsflyer.com/) native extension for [Defold](https://defold.com/), supporting attribution, events, customer IDs, consent controls and partner-sharing filters on Android and iOS.

Uses **Android SDK 7.0.1** and **iOS SDK 7.0.2**.

- [Installation, configuration and Lua API manual](docs/index.md)
- [Editor API reference](extension-appsflyer/api/appsflyer.script_api)
- [SDK 7 migration and missing-API audit](docs/sdk7-upgrade.md)
- [Simulator test instructions](tests/README.md) and [recorded results](docs/test-results.md)

Customer ID is available through `appsflyer.set_customer_user_id()`. Set it on each cold start before `appsflyer.start_sdk()`. The module exposes 14 functions; see the manual for consent, stop/resume, currency and sharing controls.

Please [open an issue](https://github.com/AGulev/defold-extension-appsflyer/issues) with the Defold version, platform, SDK version and sanitized reproduction details.
