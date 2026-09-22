#pragma once

#include <cstdint>
#include <limits>

namespace bird::resources {

// 1. Forward declare the pool
template <typename T> class AssetPool;

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

  [[nodiscard]] bool is_valid() const noexcept;
  explicit operator bool() const noexcept { return is_valid(); }

  [[nodiscard]] T* get() const noexcept;
  T* operator->() const noexcept { return get(); }

  bool operator==(const AssetHandle& other) const {
    return index == other.index && generation == other.generation && pool == other.pool;
  }

 private:
  AssetHandle(AssetPool<T>* pool, uint32_t index, uint32_t generation) noexcept;

  void add_ref();
  void release_ref();

  AssetPool<T>* pool = nullptr;
  uint32_t index = std::numeric_limits<uint32_t>::max();
  uint32_t generation = 0;
};

} // bird::resources