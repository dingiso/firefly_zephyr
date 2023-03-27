#include <zephyr/kernel.h>
#include <zephyr/random/rand32.h>
#include <zephyr/drivers/gpio.h>

#include <array>
#include <atomic>
#include <functional>
#include <memory>

#include "gtest/gtest.h"
#include "printk_event_handler.h"
#include "rpc/system_server.h"
#include "test.pwpb.h"
#include "test.rpc.pwpb.h"
#include "thread.h"
#include "pw_log/log.h"
#include "pw_assert/check.h"
#include "pw_log/proto/log.raw_rpc.pb.h"
#include "buzzer.h"
#include "rgb_led.h"
#include "eeprom.h"

using namespace common::rpc;

Buzzer buzzer;

TEST(BasicTest, Sum) {
  ASSERT_EQ(2 + 2, 4);
}

TEST(ProtoTest, Equality) {
  static_assert(pw::protobuf::IsTriviallyComparable<pwpb::Customer::Message>());
  pwpb::Customer::Message a, b;
  a.age = 5;
  a.status = pwpb::Customer::Status::ACTIVE;

  b.age = 5;
  b.status = pwpb::Customer::Status::ACTIVE;

  ASSERT_EQ(a, b);
}

RgbLed led;

TEST(RgbLedTest, InstantColorTransition) {
  const Color c = {255, 0, 0};
  led.SetColor(c);
  ASSERT_EQ(led.GetColor(), c);
}

TEST(RgbLedTest, SmoothColorTransition) {
  led.SetColor({0, 0, 0});
  led.SetColorSmooth({254, 0, 0}, 1000);
  ASSERT_EQ(led.GetColor(), Color(0, 0, 0));
  k_sleep(K_MSEC(550));
  auto r = led.GetColor().r;
  ASSERT_GE(r, 127 - 10);
  ASSERT_LE(r, 127 + 10);
  k_sleep(K_MSEC(550));
  ASSERT_EQ(led.GetColor().r, 254);
}

TEST(TimerTest, RunsDelayed) {
  std::atomic<uint8_t> counter = 0;
  const auto t = RunDelayed([&](){ ++counter; }, 30);
  k_sleep(K_MSEC(10));
  ASSERT_EQ(counter, 0);
  k_sleep(K_MSEC(10));
  ASSERT_EQ(counter, 0);
  k_sleep(K_MSEC(10));
  ASSERT_EQ(counter, 1);
  k_sleep(K_MSEC(10));
  ASSERT_EQ(counter, 1);
}

TEST(TimerTest, RunsEvery) {
  uint8_t counter = 0;
  const auto t = RunEvery([&](){ ++counter; }, 10);
  k_sleep(K_MSEC(10));
  ASSERT_EQ(counter, 1);
  k_sleep(K_MSEC(10));
  ASSERT_EQ(counter, 2);
  k_sleep(K_MSEC(10));
  ASSERT_EQ(counter, 3);
  k_sleep(K_MSEC(10));
  ASSERT_EQ(counter, 4);
}

TEST(EepromTest, CanReadWritten) {
  eeprom::EnablePower();

  for (int i = 0; i < 200; ++i) {
    uint16_t in = sys_rand32_get() % 32768 + 23;
    uint32_t address = (2 * sys_rand32_get()) % 1024;
    eeprom::Write(in, address);
    k_sleep(K_MSEC(5));
    uint16_t out = eeprom::Read<uint16_t>(address);
    ASSERT_EQ(in, out);
  }
}

TEST(Header1Test, NoShortCircuit) {
  gpio_dt_spec spec[7] = {
    GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(header_1), gpios, 0),
    GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(header_1), gpios, 1),
    GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(header_1), gpios, 2),
    GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(header_1), gpios, 3),
    GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(header_1), gpios, 4),
    GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(header_1), gpios, 5),
    GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(header_1), gpios, 6),
  };
  for (auto& s : spec) {
    gpio_pin_configure_dt(&s, GPIO_INPUT);
  }

  for (int i = 0; i < 7; ++i) {
    gpio_pin_configure_dt(&spec[i], GPIO_OUTPUT_ACTIVE);
    for (int j = 0; j < 7; ++j) {
      if (i != j) {
        ASSERT_EQ(gpio_pin_get_dt(&spec[j]), 0);
      }
    }
    gpio_pin_configure_dt(&spec[i], GPIO_INPUT);
  }
}

TEST(Header2Test, NoShortCircuit) {
  gpio_dt_spec spec[8] = {
    GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(header_2), gpios, 0),
    GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(header_2), gpios, 1),
    GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(header_2), gpios, 2),
    GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(header_2), gpios, 3),
    GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(header_2), gpios, 4),
    GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(header_2), gpios, 5),
    GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(header_2), gpios, 6),
    GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(header_2), gpios, 7),
  };
  for (auto& s : spec) {
    gpio_pin_configure_dt(&s, GPIO_INPUT);
  }

  for (int i = 0; i < 8; ++i) {
    gpio_pin_configure_dt(&spec[i], GPIO_OUTPUT_ACTIVE);
    for (int j = 0; j < 8; ++j) {
      if (i != j) {
        ASSERT_EQ(gpio_pin_get_dt(&spec[j]), 0);
      }
    }
    gpio_pin_configure_dt(&spec[i], GPIO_INPUT);
  }
}

class EchoService final
    : public common::rpc::pw_rpc::pwpb::EchoService::Service<EchoService> {
 public:
  pw::Status Echo(const pwpb::Customer::Message& request, pwpb::Customer::Message& response) {
    response = request;
    response.age += 66;
    return pw::OkStatus();
  }
};

class LogService final : public pw::log::pw_rpc::raw::Logs::Service<LogService> {
 public:
  void Listen(pw::ConstByteSpan, pw::rpc::RawServerWriter&) {}
};

static EchoService echo_service;
static LogService log_service;

int main() {
  PrintkEventHandler handler;
  pw::unit_test::RegisterEventHandler(&handler);

  int num_failures = RUN_ALL_TESTS();
  if (!num_failures) {
    buzzer.Beep(100, 600, 100);
    printk("All tests passed!\n");
  } else {
    buzzer.Beep(100, 600, 1000);
  }

  system_server::Server().RegisterService(echo_service);
  system_server::Server().RegisterService(log_service);
  Thread t{[](){
    PW_CHECK_OK(system_server::Start());
  }};

  while (true) {
    k_sleep(K_MSEC(400));
  };
}
