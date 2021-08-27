# DefAppsFlyer

[AppsFlyer](https://appsflyer.com) [Native Extension](https://www.defold.com/manuals/extensions/) for the [Defold Game Engine](https://www.defold.com) (Android only now).

At the moment it is a very basic implementation of install tracking and custom event tracking. If you need some more functions feel free to contribute or [open an issue](https://github.com/AGulev/DefAppsFlyer/issues).

SDK initialized automatically when the application starts assuming you complete the following setup steps.

## Setup

You can use the DefAppsFlyer extension in your own project by adding this project as a [Defold library dependency](https://www.defold.com/manuals/libraries/). Open your game.project file and in the dependencies field under project add:

https://github.com/AGulev/DefAppsFlyer/archive/master.zip

Or point to the ZIP file of a [specific release](https://github.com/AGulev/DefAppsFlyer/releases).

Open your game.project in a text editor and paste next section:

```lua
[appsflyer]
key = your_appsflyer_key
apple_app_id = your_app_apple_id
android_channel = amazon
is_debug = 1
```
`your_appsflyer_key` - your application Dev Key in AppsFlyer dashboard (Your Application -> App Settings -> Dev Key)  
`apple_app_id` - id of your application in App Store (not used now)  
`android_channel` - custom channel (if needed)  
`is_debug` - 1 if you wanna use debug mode of the SDK and 0 for release mode  

## API

#### `appsflyer.start_sdk()`

Method for starting SDK.

#### `appsflyer.set_debug_log(is_enabled)`

`is_enabled` boolean value

Method for manually switching SDK to debug mode. The same flag available in game.project settings appsflyer.is_debug = 1 (true) or 0 (false) value.

```lua
appsflyer.set_debug_log(true) -- turn-on debug mode
appsflyer.set_debug_log(false) -- turn-off debug mode
```

#### `appsflyer.log_event(event, event_data)`

`event` is an event name that maybe your custom or one of predefined by Appsflyer  
`event_data` is a table with data for the `event`  

More information about predefined AppsFlyer methods [here](https://support.appsflyer.com/hc/en-us/articles/115005544169-Rich-In-App-Events-Android-and-iOS#Event-Types).

Track event to Appsflyer analytics.

```lua
appsflyer.log_event("af_level_achieved", {
  af_level = 2,
  af_score = 100
})

appsflyer.log_event("af_purchase",{
  af_currency = "USD",
  af_content_id = "item_id",
  af_revenue = 100
})
```

## Issues and suggestions

If you have any issues, questions or suggestions please [create an issue](https://github.com/AGulev/DefAppsFlyer/issues) or contact me: me@agulev.com
