#if defined(DM_PLATFORM_IOS)
#import "AppsFlyerAttribution.h"

@interface AppsFlyerAttribution ()
@property (nonatomic, retain) NSMutableArray* pendingHandlers;
@end

@implementation AppsFlyerAttribution
@synthesize isBridgeReady = _isBridgeReady;

+ (AppsFlyerAttribution*)shared {
    static AppsFlyerAttribution* shared = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{ shared = [[self alloc] init]; });
    return shared;
}

- (id)init {
    if ((self = [super init]))
        self.pendingHandlers = [NSMutableArray array];
    return self;
}

- (void)dealloc {
    [_launchOptions release];
    [_pendingHandlers release];
    [super dealloc];
}

- (void)setIsBridgeReady:(BOOL)ready {
    _isBridgeReady = ready;
    // All scene/SDK bridge operations run on the main thread. Snapshot before
    // invoking the SDK so a callback cannot change the array being enumerated.
    NSArray* pending = [self.pendingHandlers copy];
    [self.pendingHandlers removeAllObjects];
    if (ready) {
        for (dispatch_block_t handler in pending)
            handler();
    }
    [pending release];
}

- (void)performWhenReady:(dispatch_block_t)handler {
    if (self.isBridgeReady) {
        handler();
    } else {
        // Copy the block to retain the URL/activity and any restoration handler.
        dispatch_block_t pending = [handler copy];
        [self.pendingHandlers addObject:pending];
        [pending release];
    }
}

- (void)continueUserActivity:(NSUserActivity*)userActivity restorationHandler:(void (^)(NSArray*))restorationHandler {
    if (!userActivity) return;
    [self performWhenReady:^{
        [[AppsFlyerLib shared] continueUserActivity:userActivity restorationHandler:restorationHandler];
    }];
}

- (void)handleOpenUrl:(NSURL*)url options:(NSDictionary*)options {
    if (!url) return;
    [self performWhenReady:^{
        [[AppsFlyerLib shared] handleOpenUrl:url options:options ?: @{}];
    }];
}

@end
#endif
