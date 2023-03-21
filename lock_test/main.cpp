#include <zephyr/kernel.h>
#include <zephyr/random/rand32.h>

#include <array>
#include <atomic>
#include <functional>
#include <memory>

#include "keyboard.h"
#include "nfc.h"
#include "gtest/gtest.h"
#include "printk_event_handler.h"
#include "pw_rpc/server.h"
#include "test.pwpb.h"
#include "test.rpc.pwpb.h"
#include "system_server.h"
#include "pw_log/log.h"
#include "pw_assert/check.h"
#include "pw_log/proto/log.raw_rpc.pb.h"

const gpio_dt_spec beeper_gpio_device_spec = GPIO_DT_SPEC_GET(DT_NODELABEL(rx_beeper), gpios);

TEST(BasicTest, Sum) {
  ASSERT_EQ(2 + 2, 4);
}

TEST(ProtoTest, Equality) {
  static_assert(pw::protobuf::IsTriviallyComparable<lock_test::pwpb::Customer::Message>());
  lock_test::pwpb::Customer::Message a, b;
  a.age = 5;
  a.status = lock_test::pwpb::Customer::Status::ACTIVE;
  a.name = "Alexey";

  b.age = 5;
  b.status = lock_test::pwpb::Customer::Status::ACTIVE;
  b.name = "Alexey";

  ASSERT_EQ(a, b);
}

class EchoService final
    : public lock_test::pw_rpc::pwpb::EchoService::Service<EchoService> {
 public:
  pw::Status Echo(const lock_test::pwpb::Customer::Message& request, lock_test::pwpb::Customer::Message& response) {
    response = request;
    response.age += 7;
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
    printk("All tests passed!\n");
  }

  Keyboard keyboard([](char c) {
    PW_LOG_INFO("Pressed %c", c);
  });

  lock_test::rpc::system_server::Server().RegisterService(echo_service);
  lock_test::rpc::system_server::Server().RegisterService(log_service);

  PW_CHECK_INT_EQ(gpio_pin_configure_dt(&beeper_gpio_device_spec, GPIO_OUTPUT), 0);
  Nfc nfc;
  nfc.Init();
  nfc.ReadUIDContinuously(400, [](const pw::Vector<uint8_t>& uid) {
    if (uid.size() == 4) {
      PW_LOG_INFO("4 byte UID: %02x %02x %02x %02x", uid[0], uid[1], uid[2], uid[3]);
    } else if (uid.size() == 7) {
      PW_LOG_INFO("7 byte UID: %02x %02x %02x %02x %02x %02x %02x", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);
    } else {
      PW_LOG_WARN("Unsupported UID length: %d", uid.size());
    }

    if (uid.size() > 0) {
      gpio_pin_set_dt(&beeper_gpio_device_spec, 1);
      k_sleep(K_MSEC(100));
      gpio_pin_set_dt(&beeper_gpio_device_spec, 0);
    }
  });

  PW_LOG_INFO("Starting pw_rpc server");
  PW_CHECK_OK(lock_test::rpc::system_server::Start());


  while (true) k_sleep(K_MSEC(1000));
}
