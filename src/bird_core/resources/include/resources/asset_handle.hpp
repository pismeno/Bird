#pragma once

#include <cstdint>
#include <limits>

namespace bird {

template <typename T> class AssetPool;

/**
 * @brief A reference-counted handle to an asset managed by an AssetPool.
 * @tparam T The asset type.
 */
template <typename T>
class AssetHandle {
  friend class AssetPool<T>;

 public:
  AssetHandle() = default;
  ~AssetHandle();
  AssetHandle(const AssetHandle& other);
  AssetHandle& operator=(const AssetHandle& other);
  AssetHandle(AssetHandle&& other) noexcept;
  AssetHandle& operator=(AssetHandle&& other) noexcept;

  /**
   * @return True if the handle points to a currently loaded and valid asset.
   */
  [[nodiscard]] bool is_valid() const noexcept;

  /**
   * @return True if the handle points to a currently loaded and valid asset.
   */
  explicit operator bool() const noexcept { return is_valid(); }

  /**
   * @return Pointer to the underlying asset, or nullptr if invalid.
   */
  [[nodiscard]] T* get() const noexcept;

  /**
   * @return Pointer to the underlying asset, or nullptr if invalid.
   */
  T* operator->() const noexcept { return get(); }

  /**
   * @return True if both handles point to the exact same asset generation in the same pool.
   */
  bool operator==(const AssetHandle& other) const {
    return index == other.index && generation == other.generation && pool == other.pool;
  }

 private:
  /**
   * @brief Internal constructor used by AssetPool.
   */
  AssetHandle(AssetPool<T>* pool, uint32_t index, uint32_t generation) noexcept;

  void add_ref();
  void release_ref();

  AssetPool<T>* pool = nullptr;
  uint32_t index = std::numeric_limits<uint32_t>::max();
  uint32_t generation = 0;
};

} // bird