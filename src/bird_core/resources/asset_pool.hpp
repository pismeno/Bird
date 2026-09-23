#pragma once

#include <resources/asset_handle.hpp>
#include <vector>
#include <string>
#include <unordered_map>

#include <resources/resources_context.hpp>
#include <osal/ifile_system.hpp>
#include <osal/os_context.hpp>
#include <utils/result.hpp>

namespace bird {

class IAssetPool { public: virtual ~IAssetPool() = default; };

template<typename T>
class AssetPool : public IAssetPool { // Implemented using Gemini AI
 public:
  AssetPool(ResourcesContext& context) : file_system(context.os_context->file_system) {};

  Result<AssetHandle<T>> load(const std::string &filepath) {
    // Check if the asset is already loaded
    auto it = cache.find(filepath);
    if (it != cache.end()) {
      uint32_t index = it->second;
      ref_counts[index]++;
      return AssetHandle<T>(this, index, generations[index]);
    }

    // If the asset is loaded, use it, else allocate for a new asset
    uint32_t index;
    if (!free_indices.empty()) {
      index = free_indices.back();
      free_indices.pop_back();
    } else {
      index = data.size();
      data.emplace_back();
      generations.push_back(1);
      ref_counts.push_back(0);
      filepaths.push_back("");
    }

    data[index].data = file_system->read_bytes(filepath);

    filepaths[index] = filepath;
    cache[filepath] = index;
    ref_counts[index] = 1;

    return AssetHandle<T>(this, index, generations[index]);
  }

  T *resolve(uint32_t index, uint32_t generation) {
    if (index >= data.size()) return nullptr;
    if (generations[index] != generation) return nullptr;
    return &data[index];
  }

  void add_ref(uint32_t index, uint32_t generation) {
    if (index < data.size() && generations[index] == generation) {
      ref_counts[index]++;
    }
  }

  void release(uint32_t index, uint32_t generation) {
    if (index >= data.size() || generations[index] != generation) return;

    ref_counts[index]--;

    if (ref_counts[index] == 0) {
      cache.erase(filepaths[index]);
      data[index] = T{};
      generations[index]++;
      free_indices.push_back(index);
    }
  }

 private:
  std::vector<T> data;
  std::vector<uint32_t> generations;
  std::vector<uint32_t> ref_counts;
  std::vector<std::string> filepaths;
  std::vector<uint32_t> free_indices;
  std::unordered_map<std::string, uint32_t> cache;

  IFileSystem* file_system;
};

// Asset Handle Implementation
template<typename T>
inline AssetHandle<T>::AssetHandle(AssetPool<T>* pool, uint32_t index, uint32_t generation) noexcept
    : pool(pool), index(index), generation(generation) {}

template<typename T>
inline AssetHandle<T>::~AssetHandle() {
  release_ref();
}

template<typename T>
inline AssetHandle<T>::AssetHandle(const AssetHandle& other)
    : pool(other.pool), index(other.index), generation(other.generation) {
  add_ref();
}

template<typename T>
inline AssetHandle<T>& AssetHandle<T>::operator=(const AssetHandle& other) {
  if (this != &other) {
    release_ref();
    pool = other.pool;
    index = other.index;
    generation = other.generation;
    add_ref();
  }
  return *this;
}

template<typename T>
inline AssetHandle<T>::AssetHandle(AssetHandle&& other) noexcept
    : pool(other.pool), index(other.index), generation(other.generation) {
  other.pool = nullptr;
  other.index = std::numeric_limits<uint32_t>::max();
}

template<typename T>
inline AssetHandle<T>& AssetHandle<T>::operator=(AssetHandle&& other) noexcept {
  if (this != &other) {
    release_ref();
    pool = other.pool;
    index = other.index;
    generation = other.generation;

    other.pool = nullptr;
    other.index = std::numeric_limits<uint32_t>::max();
  }
  return *this;
}

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

template<typename T>
inline void AssetHandle<T>::add_ref() {
  if (pool && index != std::numeric_limits<uint32_t>::max()) {
    pool->add_ref(index, generation);
  }
}

template<typename T>
inline void AssetHandle<T>::release_ref() {
  if (pool && index != std::numeric_limits<uint32_t>::max()) {
    pool->release(index, generation);
    pool = nullptr;
    index = std::numeric_limits<uint32_t>::max();
  }
}

} // bird