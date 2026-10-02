#pragma once

namespace CA {
class MetalLayer;
}

namespace CocoaBridge {
void AddLayerToWindow(void* native_window_handle, CA::MetalLayer* layer);
}
