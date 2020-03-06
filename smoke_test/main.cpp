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

class Cc1101Test {
 private:
	static Cc1101 cc1101;

 public:
	static void CanInit() {
		Cc1101 cc1101;
		cc1101.Init();
	}

	static void CanSetPacketSize() {
		cc1101.SetPacketSize(12);
		zassert_equal(cc1101.GetPacketSize(), 12, "");
	}

	static void CanTransmitSomething() {
		struct Data {
			uint8_t a, b;
		};
		Data d = { .a = 5, .b = 10 };

		cc1101.Transmit(d);
	}
};

Cc1101 Cc1101Test::cc1101;

void test_main(void) {
  ztest_test_suite(smoke_test,
                   ztest_unit_test(test_two_plus_two_is_four),
									 ztest_unit_test(Cc1101Test::CanInit),
									 ztest_unit_test(Cc1101Test::CanSetPacketSize),
									 ztest_unit_test(Cc1101Test::CanTransmitSomething)
  );
  ztest_run_test_suite(smoke_test);
	// Flash the console. For some reason it doesn't happen automatically.
	printk("\n\n\n\n\n");
	k_sleep(10);
}
