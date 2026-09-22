#pragma once

namespace bird {

struct OSContext;

class AssetManager;

struct ResourcesContext {
  OSContext* os_context;

  AssetManager* asset_manager;
};

} // bird