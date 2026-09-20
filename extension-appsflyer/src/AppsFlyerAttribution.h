#ifndef AppsFlyerAttribution_h
#define AppsFlyerAttribution_h
#import <AppsFlyerLib/AppsFlyerLib.h>

@interface AppsFlyerAttribution : NSObject
@property (nonatomic, retain) NSDictionary* launchOptions;
@property (nonatomic) BOOL isBridgeReady;

+ (AppsFlyerAttribution*)shared;
- (void)continueUserActivity:(NSUserActivity*)userActivity restorationHandler:(void (^)(NSArray*))restorationHandler;
- (void)handleOpenUrl:(NSURL*)url options:(NSDictionary*)options;
@end

#endif
