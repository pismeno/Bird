#pragma once

namespace bird {

class IFileSystem;

/**
 * @brief Context for the operating system layer.
 */
struct OSContext {
  IFileSystem* file_system;
};

} // bird