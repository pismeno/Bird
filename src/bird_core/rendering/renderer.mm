#include "renderer.hpp"

#include <AppKit/AppKit.h>
#include <Metal/Metal.hpp>
#include <QuartzCore/CAMetalLayer.hpp>

#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

namespace bird {

void Renderer::initDevice() {
    metalDevice = MTL::CreateSystemDefaultDevice();
}

void Renderer::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindow = glfwCreateWindow(1280, 720, "Bird Editor", nullptr, nullptr);

    if (!glfwWindow) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    metalWindow = glfwGetCocoaWindow(glfwWindow);

    NSWindow* window = (NSWindow*)metalWindow;

    metalLayer = CA::MetalLayer::layer();

    metalLayer->setDevice(metalDevice);
    metalLayer->setPixelFormat(MTL::PixelFormatBGRA8Unorm);

    [window.contentView setLayer:(CALayer*)metalLayer];
    [window.contentView setWantsLayer:YES];
}

void Renderer::init() {
    initDevice();
    initWindow();
}

void Renderer::run() {
    while (!glfwWindowShouldClose(glfwWindow)) {
        
        glfwPollEvents();
    }
}

void Renderer::shutdown() {
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
}

} // namespace bird