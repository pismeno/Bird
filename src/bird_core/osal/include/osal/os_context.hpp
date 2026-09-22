#pragma once

namespace bird {

class IFileSystem;

struct OSContext {
  IFileSystem& file_system;
};

} // bird