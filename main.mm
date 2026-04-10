#include <imgui.h>
#include <imgui_impl_osx.h>
#include <imgui_impl_metal.h>

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

#import "Metal/Renderer.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, strong) NSWindow *window;
@property (nonatomic, strong) MTKView *mtkView;
@property (nonatomic, strong) Renderer *renderer;
@end

@implementation AppDelegate

- (void)openDocument:(id)sender {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setCanChooseFiles:YES];
    [panel setCanChooseDirectories:NO];
    [panel setAllowsMultipleSelection:NO];

    NSArray<UTType *> *allowedTypes = @[
        [UTType typeWithFilenameExtension:@"gltf"],
        [UTType typeWithFilenameExtension:@"glb"],
    ];
    [panel setAllowedContentTypes:allowedTypes];

    if ([panel runModal] == NSModalResponseOK) {
        if (NSURL *url = [panel URL]) {
            // TODO
        }
    }
}

- (void)setupMenu {
    // App menu
    NSMenu *appMenu = [[NSMenu alloc] initWithTitle:@"App"];
    [appMenu addItemWithTitle:@"About Model Viewer" action:nil /* TODO */ keyEquivalent:@""];
    [appMenu addItem:[NSMenuItem separatorItem]];
    [appMenu addItemWithTitle:@"Settings..." action:nil /* TODO */ keyEquivalent:@","];
    [appMenu addItem:[NSMenuItem separatorItem]];
    [appMenu addItemWithTitle:@"Hide Model Viewer" action:@selector(hide:) keyEquivalent:@"h"];
    [appMenu addItem:[NSMenuItem separatorItem]];
    [appMenu addItemWithTitle:@"Quit Model Viewer" action:@selector(terminate:) keyEquivalent:@"q"];

    NSMenuItem *appMenuItem = [[NSMenuItem alloc] init];
    [appMenuItem setSubmenu:appMenu];

    // File menu
    NSMenu *fileMenu = [[NSMenu alloc] initWithTitle:@"File"];
    [fileMenu addItemWithTitle:@"Open..." action:@selector(openDocument:) keyEquivalent:@"o"];

    NSMenuItem *fileMenuItem = [[NSMenuItem alloc] init];
    [fileMenuItem setSubmenu:fileMenu];

    // Window menu
    NSMenu *windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
    [windowMenu addItemWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@"m"];
    [windowMenu addItemWithTitle:@"Close" action:@selector(performClose:) keyEquivalent:@"w"];

    NSMenuItem *fullscreenItem = [windowMenu addItemWithTitle:@"Toggle Full Screen"
                                                       action:@selector(toggleFullScreen:)
                                                keyEquivalent:@"f"];
    [fullscreenItem setKeyEquivalentModifierMask:NSEventModifierFlagControl | NSEventModifierFlagCommand];

    NSMenuItem *windowMenuItem = [[NSMenuItem alloc] init];
    [windowMenuItem setSubmenu:windowMenu];

    // Main menu
    NSMenu *mainMenu = [[NSMenu alloc] init];
    [mainMenu addItem:appMenuItem];
    [mainMenu addItem:fileMenuItem];
    [mainMenu addItem:windowMenuItem];

    [NSApp setMainMenu:mainMenu];
    [NSApp setWindowsMenu:windowMenu];
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    [self setupMenu];

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#if __APPLE__
    // TODO: save in Application Support folder
    io.IniFilename = "../../../imgui.ini";
#endif

    NSRect frame = NSMakeRect(0, 0, 1280, 720);
    self.window = [[NSWindow alloc] initWithContentRect:frame
                                              styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
    [self.window setTitle:@"Model Viewer"];
    [self.window center];

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();

    self.mtkView = [[MTKView alloc] initWithFrame:frame device:device];
    self.mtkView.colorPixelFormat = MTLPixelFormatRGBA8Unorm;
    self.mtkView.framebufferOnly = YES;

    self.renderer = [[Renderer alloc] initWithMetalKitView:self.mtkView];
    self.mtkView.delegate = self.renderer;

    self.window.contentView = self.mtkView;
    [self.window makeKeyAndOrderFront:nil];

    ImGui_ImplOSX_Init(self.mtkView);
    ImGui_ImplMetal_Init(device);

    [self.window activate];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

- (void)applicationWillTerminate:(NSNotification *)notification {
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplOSX_Shutdown();
    ImGui::DestroyContext();
}

@end

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        AppDelegate *delegate = [[AppDelegate alloc] init];
        app.delegate = delegate;
        [app run];
    }
}
