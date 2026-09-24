#pragma once

#include <cstdint>
#include <string_view>
#include <vector>
#include <memory>
#include <filesystem>

#include <utils/result.hpp>

namespace bird {

class IFileSystem {
 public:
  explicit IFileSystem() = default;
  virtual ~IFileSystem() = default;

  virtual void mount(std::string_view prefix, const std::filesystem::path& physical_path) = 0;
  virtual Result<std::filesystem::path> resolve(std::string_view virtual_path) const = 0;

  virtual Result<std::vector<uint8_t>> read_bytes(std::string_view path) = 0;
  virtual Result<void> write_bytes(std::string_view path, const std::vector<uint8_t>& data) = 0;
  virtual Result<bool> exists(std::string_view path) const = 0;
  virtual Result<uint64_t> get_file_size(std::string_view path) const = 0;

  static Result<std::unique_ptr<IFileSystem>> create();
};

} // bird