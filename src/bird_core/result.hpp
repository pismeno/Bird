#pragma once

#include <string_view>
#include <source_location>

namespace bird {

struct Result {
  bool success;
  std::string_view message;
  std::source_location location;

  static Result ok() {
    return {true, "", std::source_location::current()};
  }

  static Result fail(std::string_view msg, std::source_location loc = std::source_location::current()) {
    return {false, msg, loc};
  }
};

} // bird