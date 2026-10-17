#include <doctest/doctest.h>

#include "utils/result.hpp"

TEST_CASE("Result utility successfully handles values") {
  mole::Result<void> res = mole::ok();

  CHECK(res.is_ok());
}