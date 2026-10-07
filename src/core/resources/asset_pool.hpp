#pragma once

#include <resources/asset_handle.hpp>
#include <vector>
#include <string>
#include <unordered_map>

#include <resources/asset_types.hpp>
#include <osal/ifile_system.hpp>
#include <osal/os_context.hpp>
#include <utils/result.hpp>
#include <utils/string_hash.hpp>

namespace bird {

class IAssetPool { public: virtual ~IAssetPool() = default; };

template<typename T>
class AssetPool : public IAssetPool {
 public:
  AssetPool(IFileSystem* file_system) : file_system(file_system) {};

  Result<AssetHandle<T>> load(std::string_view filepath) {
    // Check if the asset is already loaded
    auto it = cache.find(filepath);
    if (it != cache.end()) {
      uint32_t index = it->second;
      ref_counts[index]++;
      return AssetHandle<T>(this, index, generations[index]);
    }

    // Perform I/O before touching any internal index state
    auto r_read_bytes = file_system->read_bytes(filepath);
    if (!r_read_bytes) {
      return bird::fail(r_read_bytes.error());
    }

    auto r_asset = parse_asset(filepath, std::move(r_read_bytes).value());
    if (!r_asset) {
      return bird::fail(r_asset.error());
    }

    // I/O succeeded: now acquire/allocate an index safely
    uint32_t index;
    if (!free_indices.empty()) {
      index = free_indices.back();
      free_indices.pop_back();
    } else {
      index = static_cast<uint32_t>(data.size());
      data.emplace_back();
      generations.push_back(1);
      ref_counts.push_back(0);
      filepaths.emplace_back();
    }

    data[index] = std::move(r_asset).value();
    filepaths[index] = std::string(filepath);
    cache.try_emplace(filepaths[index], index);
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

 protected:
  // 1. Declare the template method inside the class without the inline definition
  Result<T> parse_asset(std::string_view filepath, std::vector<uint8_t> raw_bytes);

 private:
  std::vector<T> data;
  std::vector<uint32_t> generations;
  std::vector<uint32_t> ref_counts;
  std::vector<std::string> filepaths;
  std::vector<uint32_t> free_indices;
  std::unordered_map<std::string, uint32_t, StringHash, std::equal_to<>> cache;

  IFileSystem* file_system;
};

template<typename T>
inline Result<T> AssetPool<T>::parse_asset(std::string_view filepath, std::vector<uint8_t> raw_bytes) {
  T asset;
  asset.data = std::move(raw_bytes);
  return asset;
}

template <>
Result<ImageAsset> AssetPool<ImageAsset>::parse_asset(std::string_view filepath, std::vector<uint8_t> raw_bytes);

} // bird