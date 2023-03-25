pw_proto_library(rpc.test_proto
  SOURCES
    test.proto
  DEPS
    pw_protobuf.field_options_proto
)

pw_add_library(rpc.system_server STATIC
  HEADERS
    system_server.h
  PRIVATE_DEPS
    pw_hdlc.pw_rpc
    pw_hdlc.rpc_channel_output
    pw_stream.sys_io_stream
    pw_log
  SOURCES
    system_server.cc
)
