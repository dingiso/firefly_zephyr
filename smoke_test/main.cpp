#include <array>
#include <functional>
#include <memory>
#include <vector>

#include <vector>
#include <kernel.h>
#include <ztest.h>

static void test_two_plus_two_is_four(void) {
  zassert_equal(2 + 2, 4, "2 + 2 is not 4");
}

void test_main(void) {
  ztest_test_suite(smoke_test,
                   ztest_unit_test(test_two_plus_two_is_four)
  );
  ztest_run_test_suite(smoke_test);
}
