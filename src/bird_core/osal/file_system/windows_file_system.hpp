#pragma once

#include <osal/ifile_system.hpp>

#include <string_view>
#include <vector>
#include <cstdint>

namespace bird {

class WindowsFileSystem : public IFileSystem {
 public:
  WindowsFileSystem() = default;
  ~WindowsFileSystem() override = default;

  Result<std::vector<uint8_t>> read_bytes(std::string_view path) override;
  Result<void> write_bytes(std::string_view path, const std::vector<uint8_t>& data) override;
  [[nodiscard]] Result<bool> exists(std::string_view path) const override;
  [[nodiscard]] Result<uint64_t> get_file_size(std::string_view path) const override;
};

} // namespace bird