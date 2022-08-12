#include "pw_unit_test/googletest_style_event_handler.h"

class PrintkEventHandler final : public pw::unit_test::GoogleTestStyleEventHandler {
 public:
  constexpr PrintkEventHandler(bool verbose = false) : GoogleTestStyleEventHandler(verbose) {};

 private:
  void Write(const char* content) override;
  void WriteLine(const char* format, ...) override;
};
