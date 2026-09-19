//
//  AppsFlyerAttribution.h
//  UnityFramework
//
//  Created by Margot Guetta on 11/04/2021.
//

#ifndef AppsFlyerAttribution_h
#define AppsFlyerAttribution_h
#if __has_include(<AppsFlyerLib/AppsFlyerLib.h>)
#import <AppsFlyerLib/AppsFlyerLib.h>
#else
#import "AppsFlyerLib.h"
#endif


@interface AppsFlyerAttribution : NSObject
@property (nonatomic, retain) NSDictionary* _Nullable launchOptions;
@property (nonatomic, retain) NSUserActivity*_Nullable userActivity;
@property (nonatomic, copy) void (^ _Nullable restorationHandler)(NSArray *_Nullable );
@property (nonatomic, retain) NSURL * _Nullable url;
@property (nonatomic, retain) NSDictionary * _Nullable options;
@property (nonatomic, copy) NSString* _Nullable sourceApplication;
@property (nonatomic, retain) id _Nullable annotation;
@property BOOL isBridgeReady;

+ (AppsFlyerAttribution *_Nullable)shared;
- (void) continueUserActivity: (NSUserActivity*_Nullable) userActivity restorationHandler: (void (^_Nullable)(NSArray * _Nullable))restorationHandler;
- (void) handleOpenUrl:(NSURL*_Nullable)url options:(NSDictionary*_Nullable) options;
- (void) handleOpenUrl: (NSURL *_Nonnull)url sourceApplication:(NSString* _Nullable)sourceApplication annotation:(id _Nullable)annotation;

@end

static NSString * _Nullable const AF_BRIDGE_SET = @"bridge is set";

#endif /* AppsFlyerAttribution_h */
