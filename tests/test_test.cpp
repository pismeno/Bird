#include <doctest/doctest.h>

#include "utils/result.hpp"

TEST_CASE("Result utility successfully handles values") {
  bird::Result<void> res = bird::ok();

  CHECK(res.is_ok());
}