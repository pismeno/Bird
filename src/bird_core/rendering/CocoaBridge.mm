/**
#include "GLFWBridge.hpp"

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <AppKit/AppKit.h>
#include <QuartzCore/CAMetalLayer.h>

namespace GLFWBridge {

void AddLayerToWindow(GLFWwindow* window, CA::MetalLayer* layer) {
    NSWindow* cocoa_window = glfwGetCocoaWindow(window);
    CAMetalLayer* native_layer = (__bridge CAMetalLayer*)layer;
    [[cocoa_window contentView] setLayer:native_layer];
    [native_layer setMaximumDrawableCount:2];
    [[cocoa_window contentView] setWantsLayer:YES];
    [[cocoa_window contentView] setNeedsLayout:YES];
}

} // namespace GLFWBridge
*/

#include "CocoaBridge.hpp"
#include <AppKit/AppKit.h>

namespace CocoaBridge {

void AddLayerToWindow(void* native_window_handle, CA::MetalLayer* layer) {
    // Safely cast the generic void* back to an Objective-C NSWindow*
    NSWindow* cocoa_window = (__bridge NSWindow*)native_window_handle;
    CAMetalLayer* native_layer = (__bridge CAMetalLayer*)layer;

    [[cocoa_window contentView] setLayer:native_layer];
    [native_layer setMaximumDrawableCount:2];
    [[cocoa_window contentView] setWantsLayer:YES];
    [[cocoa_window contentView] setNeedsLayout:YES];
}

} // namespace CocoaBridge