#include <resources/asset_handle.hpp>

namespace bird::resources {

template<typename T>
[[nodiscard]] inline bool AssetHandle<T>::is_valid() const noexcept {
  if (!pool || index == std::numeric_limits<uint32_t>::max()) return false;
  return pool->resolve(index, generation) != nullptr;
}

template<typename T>
[[nodiscard]] inline T* AssetHandle<T>::get() const noexcept {
  if (!pool) return nullptr;
  return pool->resolve(index, generation);
}

} // bird::resources