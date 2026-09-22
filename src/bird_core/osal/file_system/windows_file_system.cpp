#include "windows_file_system.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>

namespace bird {

namespace {
// Internal helper for RAII handle management, hidden in anonymous namespace
class scoped_win32_handle {
 public:
  explicit scoped_win32_handle(HANDLE raw_handle) : handle(raw_handle) {}

  ~scoped_win32_handle() {
    if (handle != INVALID_HANDLE_VALUE && handle != nullptr) {
      CloseHandle(handle);
    }
  }

  HANDLE get() const { return handle; }
  bool is_valid() const { return handle != INVALID_HANDLE_VALUE; }

 private:
  HANDLE handle;
};

// Internal helper to convert engine UTF-8 strings to Windows UTF-16 strings
std::wstring utf8_to_utf16(std::string_view utf8_str) {
  if (utf8_str.empty()) return {};

  int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8_str.data(),
                                        static_cast<int>(utf8_str.size()), nullptr, 0);
  std::wstring result(size_needed, 0);
  MultiByteToWideChar(CP_UTF8, 0, utf8_str.data(),
                      static_cast<int>(utf8_str.size()), result.data(), size_needed);

  return result;
}
} // anonymous namespace

Result<std::vector<uint8_t>> WindowsFileSystem::read_bytes(std::string_view path) {
  std::wstring wpath = utf8_to_utf16(path);

  scoped_win32_handle file(CreateFileW(
      wpath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr
  ));

  if (!file.is_valid()) {
    return fail("Failed to open file for reading: " + std::string(path));
  }

  LARGE_INTEGER file_size;
  if (!GetFileSizeEx(file.get(), &file_size)) {
    return fail("Failed to get file size: " + std::string(path));
  }

  if (file_size.QuadPart > UINT32_MAX) {
    return fail("File too large for single-pass read: " + std::string(path));
  }

  std::vector<uint8_t> buffer;
  buffer.resize(static_cast<size_t>(file_size.QuadPart));

  DWORD bytes_read = 0;
  if (!ReadFile(file.get(), buffer.data(), static_cast<DWORD>(buffer.size()), &bytes_read, nullptr)) {
    return fail("Failed to read file contents: " + std::string(path));
  }

  if (bytes_read != buffer.size()) {
    return fail("Read fewer bytes than expected from: " + std::string(path));
  }

  return buffer;
}

Result<void> WindowsFileSystem::write_bytes(std::string_view path, const std::vector<uint8_t>& data) {
  std::wstring wpath = utf8_to_utf16(path);

  scoped_win32_handle file(CreateFileW(
      wpath.c_str(), GENERIC_WRITE, 0, nullptr,
      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr
  ));

  if (!file.is_valid()) {
    return fail("Failed to open file for writing: " + std::string(path));
  }

  if (data.size() > UINT32_MAX) {
    return fail("Data too large for single-pass write: " + std::string(path));
  }

  DWORD bytes_written = 0;
  if (!WriteFile(file.get(), data.data(), static_cast<DWORD>(data.size()), &bytes_written, nullptr)) {
    return fail("Failed to write to file: " + std::string(path));
  }

  if (bytes_written != data.size()) {
    return fail("Wrote fewer bytes than expected to: " + std::string(path));
  }

  return ok();
}

Result<bool> WindowsFileSystem::exists(std::string_view path) const {
  std::wstring wpath = utf8_to_utf16(path);
  DWORD attributes = GetFileAttributesW(wpath.c_str());

  return (attributes != INVALID_FILE_ATTRIBUTES);
}

Result<uint64_t> WindowsFileSystem::get_file_size(std::string_view path) const {
  std::wstring wpath = utf8_to_utf16(path);
  WIN32_FILE_ATTRIBUTE_DATA file_info;

  if (!GetFileAttributesExW(wpath.c_str(), GetFileExInfoStandard, &file_info)) {
    return fail("Failed to get file attributes: " + std::string(path));
  }

  LARGE_INTEGER size;
  size.HighPart = file_info.nFileSizeHigh;
  size.LowPart = file_info.nFileSizeLow;

  return static_cast<uint64_t>(size.QuadPart);
}

} // namespace bird