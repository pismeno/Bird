#pragma once

namespace mole {

class IFileSystem;

/**
 * @brief Context for the operating system layer.
 */
struct OSContext {
  IFileSystem* file_system;
};

} // mole