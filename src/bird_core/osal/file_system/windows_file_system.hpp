#pragma once

#include <osal/ifile_system.hpp>

#include <string_view>
#include <vector>
#include <cstdint>
#include <unordered_map>

namespace bird {

class WindowsFileSystem : public IFileSystem {

  friend class IFileSystem;

 public:
  ~WindowsFileSystem() override = default;

  void mount(std::string_view prefix, const std::filesystem::path& physical_path) override;
  Result<std::filesystem::path> resolve(std::string_view virtual_path) const override;

  Result<std::vector<uint8_t>> read_bytes(std::string_view path) override;
  Result<void> write_bytes(std::string_view path, const std::vector<uint8_t>& data) override;
  [[nodiscard]] Result<bool> exists(std::string_view path) const override;
  [[nodiscard]] Result<uint64_t> get_file_size(std::string_view path) const override;
 private:
  WindowsFileSystem() = default;

  std::unordered_map<std::string, std::filesystem::path> mount_points;
};

} // bird