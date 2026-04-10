#import "Renderer.h"

#include <imgui_impl_osx.h>
#include <imgui_impl_metal.h>

@implementation Renderer {

}

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)mtkView {
    if (self = [super init]) {
        _device = mtkView.device;
        _commandQueue = [_device newCommandQueue];
    }
    return self;
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    @autoreleasepool {
        id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBufferWithUnretainedReferences];

        MTLRenderPassDescriptor *renderPassDescriptor = view.currentRenderPassDescriptor;
        renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionDontCare;
        if (renderPassDescriptor != nil) {
            id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];

            ImGui_ImplMetal_NewFrame(renderPassDescriptor);
            ImGui_ImplOSX_NewFrame(view);
            ImGui::NewFrame();

            ImGui::DockSpaceOverViewport();
            ImGui::ShowDemoWindow();

            ImGui::Render();
            ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commandBuffer, renderEncoder);

            [renderEncoder endEncoding];

            id<CAMetalDrawable> currentDrawable = view.currentDrawable;
            if (currentDrawable != nil) {
                [commandBuffer presentDrawable:currentDrawable];
            }
        }

        [commandBuffer commit];
        [commandBuffer waitUntilCompleted];
    }
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {

}

@end