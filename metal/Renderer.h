#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

@interface Renderer : NSObject <MTKViewDelegate>
@property (nonatomic, strong) _Nonnull id<MTLDevice> device;
@property (nonatomic, strong) _Nonnull id<MTLCommandQueue> commandQueue;

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)mtkView;
@end

