#pragma once

#include <resources/asset_handle.hpp>
#include <vector>
#include <string>

namespace bird::resources {

class IAssetPool { public: virtual ~IAssetPool() = default; };

template<typename T>
class AssetPool : public IAssetPool {
 public:
  AssetHandle<T> load(const std::string &filepath) {
    uint32_t index = data.size();
    data.emplace_back();
    generations.push_back(1);

    //data[index].data = OS::FileSystem::Read(filepath);

    return AssetHandle<T>(this, index, 1);
  }

  T *resolve(uint32_t index, uint32_t generation) {
    if (index >= data.size()) return nullptr;
    if (generations[index] != generation) return nullptr;
    return &data[index];
  }

 private:
  std::vector<T> data;
  std::vector<uint32_t> generations;
};

} // bird::resources