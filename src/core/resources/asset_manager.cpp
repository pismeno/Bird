#include <resources/asset_manager.hpp>

#include <memory>

#include <utils/result.hpp>
#include <resources/resources_context.hpp>

namespace mole {

Result<std::unique_ptr<AssetManager>> AssetManager::create(ResourcesContext& context) {
  auto manager = std::unique_ptr<AssetManager>(new AssetManager());

  auto r_init = manager->init(context);
  if (!r_init) return mole::fail(r_init.error());

  return std::move(manager);
}

Result<void> AssetManager::init(ResourcesContext& context) {
  file_system = context.os_context->file_system;
  return mole::ok();
}

Result<AssetHeader> AssetManager::read_asset_header(const std::string_view virtual_path) {
  auto r_chunk = file_system->read_chunk(virtual_path, 0, sizeof(AssetHeader));
  if (!r_chunk) {
    return mole::fail("Failed to read header from virtual path: " + r_chunk.error());
  }

  const auto& bytes = r_chunk.value();
  if (bytes.size() < sizeof(AssetHeader)) {
    return mole::fail("Failed to read header, file might be too small");
  }

  AssetHeader header{};
  std::memcpy(&header, bytes.data(), sizeof(AssetHeader));

  if (std::string_view(header.magic, 4) != ASSET_MAGIC) {
    return mole::fail("Invalid magic number");
  }

  if (header.version != ASSET_CURRENT_VERSION) {
    return mole::fail("Unsupported asset version");
  }

  return header;
}

} // mole