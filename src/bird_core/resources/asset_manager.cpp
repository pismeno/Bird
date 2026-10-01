#include <resources/asset_manager.hpp>

#include <memory>

#include <utils/result.hpp>
#include <resources/resources_context.hpp>

namespace bird {

Result<std::unique_ptr<AssetManager>> AssetManager::create(ResourcesContext& context) {
  auto manager = std::unique_ptr<AssetManager>(new AssetManager());

  auto r_init = manager->init(context);
  if (!r_init) return bird::fail(r_init.error());

  return std::move(manager);
}

Result<void> AssetManager::init(ResourcesContext& context) {
  file_system = context.os_context->file_system;
  return bird::ok();
}

} // bird