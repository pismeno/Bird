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
  [[nodiscard]] const T* get() const noexcept;

  /**
   * @return Pointer to the underlying asset, or nullptr if invalid.
   */
  const T* operator->() const noexcept { return get(); }

  /**
   * @return True if both handles point to the exact same asset generation in the same pool.
   */
  bool operator==(const AssetHandle& other) const {
    return index == other.index && generation == other.generation && pool == other.pool;
  }

  /**
   * @brief Returns a packed 64-bit ID (32-bit generation | 32-bit index)
   */
  [[nodiscard]] inline uint64_t get_packed_id() const noexcept { return (static_cast<uint64_t>(generation) << 32) | static_cast<uint64_t>(index); }
  [[nodiscard]] inline uint32_t get_index() const noexcept { return index; }
  [[nodiscard]] inline uint32_t get_generation() const noexcept {return generation; }

 private:
  /**
   * @brief Internal constructor used by AssetPool.
   * @param pool The pool that manages this asset.
   * @param index The index of the asset in the pool.
   * @param generation The generation of the asset.
   */
  AssetHandle(AssetPool<T>* pool, uint32_t index, uint32_t generation) noexcept;

  void add_ref();
  void release_ref();

  AssetPool<T>* pool = nullptr;
  uint32_t index = std::numeric_limits<uint32_t>::max();
  uint32_t generation = 0;
};

} // bird