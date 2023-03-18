#include <zephyr/kernel.h>
#include <zephyr/random/rand32.h>

#include <array>
#include <atomic>
#include <functional>
#include <memory>

#include "gtest/gtest.h"
#include "printk_event_handler.h"
#include "pw_rpc/server.h"
#include "test.pwpb.h"
#include "test.rpc.pwpb.h"
#include "system_server.h"
#include "pw_log/log.h"
#include "pw_assert/check.h"
#include "pw_log/proto/log.raw_rpc.pb.h"

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

  lock_test::rpc::system_server::Server().RegisterService(echo_service);
  lock_test::rpc::system_server::Server().RegisterService(log_service);

  PW_LOG_INFO("Starting pw_rpc server");
  PW_CHECK_OK(lock_test::rpc::system_server::Start());


  while (true) k_sleep(K_MSEC(1000));
}
