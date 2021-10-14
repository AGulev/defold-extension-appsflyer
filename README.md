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

`key` is AppsFlyer dev key  
`apple_app_id` is id of your application in App Store (not used now)  
`android_channel` is custom channel (if needed)  
`is_debug` 1 if you wanna use debug logs of the SDK and 0 for release mode  

`Minimum SDK version` in `game.project->Android` should be **21** or above.

`android.permission.READ_PHONE_STATE` is optional for AppsFlyer. Please add it into your `AndroidManifest.xml` if it's important for you.
 ```
 <!-- Optional : -->
 <uses-permission android:name="android.permission.READ_PHONE_STATE" />
 ```

## API

#### `appsflyer.start_sdk()`

Starts the SDK.

Typical usage of deferred SDK start is when an app would like to request consent from the user to collect data.

#### `appsflyer.set_debug_log(is_enabled)`

`is_enabled` boolean value

Enables Debug logs for the AppsFlyer SDK. Should only be set to true in development environments.

```lua
appsflyer.set_debug_log(true)
appsflyer.set_debug_log(false)
```

#### `appsflyer.set_callback(callback)`

Sets the callback function to receive conversion data events.

```lua
local function appsflyer_callback(self, message_id, message)
    if message_id == appsflyer.CONVERSION_DATA_SUCCESS then
        print("Conversion data loaded:");
        pprint(message);
    elseif message_id == appsflyer.CONVERSION_DATA_FAIL then
        print("Conversion data loading failed:", message.error);
    end
end

appsflyer.set_callback(appsflyer_callback)
```

#### `appsflyer.log_event(event, event_data)`

`event` is an event name that maybe your custom or one of predefined by Appsflyer  
`event_data` is a table with data for the `event`  

Log an in-app event.

More information about predefined AppsFlyer methods [here](https://support.appsflyer.com/hc/en-us/articles/115005544169-Rich-In-App-Events-Android-and-iOS#Event-Types).

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

#### `appsflyer.set_customer_user_id(user_id)`

Setting your own Customer User ID in the AppsFlyer SDK enables you to cross-reference your own unique ID with the AppsFlyer ID and other identifiers.

More information about this method is available [here](https://support.appsflyer.com/hc/en-us/articles/207032126-Android-SDK-integration-guide-for-marketers#additional-apis-set-customer-user-id)

```lua
appsflyer.set_customer_user_id("your_customer_user_id")
```

## Issues and suggestions

If you have any issues, questions or suggestions please [create an issue](https://github.com/AGulev/DefAppsFlyer/issues) or contact me: me@agulev.com
