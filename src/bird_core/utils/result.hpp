#pragma once

#include <string>
#include <string_view>
#include <source_location>
#include <variant>
#include <iostream>
#include <utility>
#include <cassert>
#include <type_traits>
#include <concepts>

namespace bird {

struct Failure {
  std::string message;
};

/**
 * @brief A class that is to be used instead of throwing errors. If successfull it holds the passed down value,
 * otherwise it holds an error message.
 * @tparam T type of the value
 */
template <typename T>
class [[nodiscard]] Result {
 public:
  Result(T value) : data(std::move(value)) {} // Accept by value and move, works for both copyable and move-only types
  Result(bird::Failure failure) : data(std::move(failure.message)) {}

  template <typename U>
  requires std::convertible_to<U, T> && (!std::same_as<std::decay_t<U>, Result<T>>)
  Result(U&& value) : data(T(std::forward<U>(value))) {}

  Result(Result&& other) noexcept = default; // moving the Result itself
  Result& operator=(Result&& other) noexcept = default;

  Result(const Result&) = default; // Disable copying if T is move-only
  Result& operator=(const Result&) = default;

  [[nodiscard]] inline bool is_ok() const noexcept { return std::holds_alternative<T>(data); }
  explicit operator bool() const noexcept { return is_ok(); }

  /**
   * @brief If the result is successful, returns the value, otherwise terminates the program. Only call this if the result is successful.
   */
  [[nodiscard]] const T& value() const & noexcept {
    assert(is_ok() && "Error: tried to access value of a Result that is failed.");
    return std::get<T>(data);
  }

  /**
   * @brief If the result is successful, returns the value, otherwise terminates the program. Only call this if the result is successful.
   */
  [[nodiscard]] T&& value() && noexcept {
    assert(is_ok() && "Error: tried to access value of a Result that is failed.");
    return std::get<T>(std::move(data));
  }

  /**
 * @brief If the result is failed, returns the error message, otherwise terminates the program. Only call this if the result is failed.
 */
  [[nodiscard]] const std::string& error() const {
    assert(!is_ok() && "Error: tried to access error of a Result that is successful.");
    return std::get<std::string>(data);
  }

 private:
  std::variant<T, std::string> data;
};

/**
 * @brief A specialization of Result for void. It is to be used for functions that would return void and throw exceptions otherwise.'
 */
template <>
class [[nodiscard]] Result<void> {
 public:
  Result() : success(true) {}
  Result(Failure failure) : error_message(std::move(failure.message)), success(false) {}

  bool is_ok() const noexcept { return success; }
  explicit operator bool() const noexcept { return success; }

  const std::string& error() const noexcept { return error_message; }

 private:
  std::string error_message = "";
  bool success;
};

static inline Result<void> ok() { return Result<void>(); }
static inline Failure fail(std::string error_message) { return Failure{std::move(error_message)}; }
} // bird