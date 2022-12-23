#if defined(DM_PLATFORM_IOS)

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface DEFAFSDKDelegate : NSObject

@property (nonatomic) void (*onConversionDataSuccess)(NSDictionary *conversionInfo);
@property (nonatomic) void (*onAppOpenAttribution)(NSDictionary *attributionData);
@property (nonatomic) void (*onConversionDataFail)(NSString *error);
@property (nonatomic) void (*onAppOpenAttributionFailure)(NSString *error);

@end

NS_ASSUME_NONNULL_END

#endif
