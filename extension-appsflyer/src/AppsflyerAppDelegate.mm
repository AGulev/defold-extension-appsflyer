#if defined(DM_PLATFORM_IOS)
#import "AppsflyerAppDelegate.h"
#import "AppsFlyerAttribution.h"

@implementation AppsflyerAppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    if ([AppsFlyerAttribution shared].isBridgeReady) {
        [[AppsFlyerLib shared] handleLaunchOptions:launchOptions];
    } else {
        [AppsFlyerAttribution shared].launchOptions = launchOptions;
    }
    return YES;
}

- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions {
    // Cold-start links arrive before the SDK/Lua bridge exists. The attribution
    // bridge retains every pending URL/activity until SDK initialization.
    [self scene:scene openURLContexts:connectionOptions.URLContexts];
    for (NSUserActivity* activity in connectionOptions.userActivities)
        [self scene:scene continueUserActivity:activity];
}

- (void)scene:(UIScene *)scene openURLContexts:(NSSet<UIOpenURLContext *> *)URLContexts {
    for (UIOpenURLContext* context in URLContexts) {
        NSMutableDictionary* options = [NSMutableDictionary dictionary];
        if (context.options.sourceApplication)
            options[UIApplicationOpenURLOptionsSourceApplicationKey] = context.options.sourceApplication;
        if (context.options.annotation)
            options[UIApplicationOpenURLOptionsAnnotationKey] = context.options.annotation;
        options[UIApplicationOpenURLOptionsOpenInPlaceKey] = @(context.options.openInPlace);
        [[AppsFlyerAttribution shared] handleOpenUrl:context.URL options:options];
    }
}

- (void)scene:(UIScene *)scene continueUserActivity:(NSUserActivity *)userActivity {
    [[AppsFlyerAttribution shared] continueUserActivity:userActivity restorationHandler:nil];
}

@end
#endif
