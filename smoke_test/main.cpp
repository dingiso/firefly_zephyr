#include <array>
#include <functional>
#include <memory>
#include <vector>

#include <vector>
#include <kernel.h>
#include <ztest.h>

#include "cc1101.h"

void test_two_plus_two_is_four(void) {
  zassert_equal(2 + 2, 4, "2 + 2 is not 4");
}

void Cc1101Test() {
	Cc1101 cc1101;
	cc1101.Init();
	cc1101.SetPacketSize(12);
	zassert_equal(cc1101.GetPacketSize(), 12, "");
}

void test_main(void) {
  ztest_test_suite(smoke_test,
                   ztest_unit_test(test_two_plus_two_is_four),
									 ztest_unit_test(Cc1101Test)
  );
  ztest_run_test_suite(smoke_test);
}
