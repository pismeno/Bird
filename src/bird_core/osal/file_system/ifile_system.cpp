#include <osal/ifile_system.hpp>

#include <memory>

#include <utils/result.hpp>

#ifdef BIRD_PLATFORM_WINDOWS
#include "windows_file_system.hpp"
#endif

namespace bird {

Result<std::unique_ptr<IFileSystem>> IFileSystem::create() {
#ifdef BIRD_PLATFORM_WINDOWS
  return std::unique_ptr<IFileSystem>(new WindowsFileSystem());
#else
  return bird::fail("Unsupported platform");
#endif
}

} //
