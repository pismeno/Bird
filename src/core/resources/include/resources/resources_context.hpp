#pragma once

namespace bird {

struct OSContext;

class AssetManager;
class MaterialPool;

/**
 * @brief Context for the resources layer.
 */
struct ResourcesContext {
  OSContext* os_context;

  AssetManager* asset_manager;
  MaterialPool* material_pool;
};

} // bird