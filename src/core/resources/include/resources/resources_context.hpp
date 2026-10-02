#pragma once

namespace bird {

struct OSContext;

class AssetManager;

/**
 * @brief Context for the resources layer.
 */
struct ResourcesContext {
  OSContext* os_context;

  AssetManager* asset_manager;
};

} // bird