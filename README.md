# DefAppsFlyer

[AppsFlyer](https://appsflyer.com) [Native Extension](https://www.defold.com/manuals/extensions/) for the [Defold Game Engine](https://www.defold.com) (iOS and Android).

At the moment it is a very basic implementation of install tracking and custom event tracking. If you need some more functions feel free to contribute or [open an issue](https://github.com/AGulev/DefAppsFlyer/issues).

SDK initialized automatically when the application starts assuming you complete the following setup steps.

## Setup

You can use the DefAppsFlyer extension in your own project by adding this project as a [Defold library dependency](https://www.defold.com/manuals/libraries/). Open your game.project file and in the dependencies field under project add:

https://github.com/AGulev/DefAppsFlyer/archive/master.zip

Or point to the ZIP file of a [specific release](https://github.com/AGulev/DefAppsFlyer/releases).

Open your game.project in a text editor and paste next section:

```lua
[apps_flyer]
key = your_appsflyer_key
apple_app_id = your_app_apple_id
is_debug = 1
```
`your_appsflyer_key` - your application Dev Key in AppsFlyer dashboard (Your Application -> App Settings -> Dev Key)
`apple_app_id` - id of your application in App Store (part of the link to your app, for example for https://itunes.apple.com/app/id1167655230 it will be 1167655230)

`is_debug` - 1 if you wanna use debug mode of the SDK and 0 for release mode.

## API

#### `appsflyer.setIsDebug(is_debug)`

`is_debug` boolean value.

Method for manually switching SDK to debug mode. The same flag available in game.project settings apps_flyer.is_debug = 1 (true) or 0 (false) value.

###### example
```lua
appsflyer.setIsDebug(true) -- turn-on debug mode
appsflyer.setIsDebug(false) -- turn-off debug mode
```

#### `appsflyer.trackEvent(event, data_table)`

`event` is an event name that maybe your custom or one of predefined by Appsflyer.
`data_table` is a table with data for the `event`.

More information about predefined AppsFlyer methods [here](https://support.appsflyer.com/hc/en-us/articles/115005544169-Rich-In-App-Events-Android-and-iOS#Event-Types).

Track event to Appsflyer analytics.

###### example
```lua
appsflyer.trackEvent("af_level_achieved", {
  af_level = 2,
  af_score = 9990}) -- tracking of predefined by AppsFlyer af_level_achieved event

appsflyer.trackEvent("user_did_it",{
  times = 5,
  lvl="Level7",
  session = 8}) -- custom user event

appsflyer.trackEvent("af_purchase",{
  af_currency = shop_item.currency_code,
  af_content_id = "item_id",
  af_revenue = shop_item.price
}) -- tracking of predefined by AppsFlyer purchase event
```

## Issues and suggestions

If you have any issues, questions or suggestions please [create an issue](https://github.com/AGulev/DefAppsFlyer/issues) or contact me: me@agulev.com
