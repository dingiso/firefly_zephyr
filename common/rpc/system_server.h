#pragma once
#include "pw_rpc/server.h"

namespace common::rpc::system_server {

pw::rpc::Server& Server();

pw::Status Start();

}  // common::rpc::system_server
