#pragma once
#include "pw_rpc/server.h"

namespace lock_test::rpc::system_server {

pw::rpc::Server& Server();

pw::Status Start();

}  // namespace lock_test::rpc::system_server
