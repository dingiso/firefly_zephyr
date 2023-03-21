#include "nfc_definitions.h"

#include "pw_log/log.h"

namespace nfc {

void Log(ComIrqRegBits bits) {
  PW_LOG_INFO("ComIrqReg: %d (Set1 = %d, TxIRq = %d, RxIRq = %d, IdleIRq = %d, HiAlertIRq = %d, LoAlertIRq = %d, ErrIRq = %d, TimerIRq = %d)",
              underlying(bits),
              any(bits & ComIrqRegBits::Set1),
              any(bits & ComIrqRegBits::TxIRq),
              any(bits & ComIrqRegBits::RxIRq),
              any(bits & ComIrqRegBits::IdleIRq),
              any(bits & ComIrqRegBits::HiAlertIRq),
              any(bits & ComIrqRegBits::LoAlertIRq),
              any(bits & ComIrqRegBits::ErrIRq),
              any(bits & ComIrqRegBits::TimerIRq));
}

void Log(Status1RegBits bits) {
  PW_LOG_INFO("Status1Reg: %d (CRCOk = %d, CRCReady = %d, IRq = %d, TRunning = %d, HiAlert = %d, LoAlert = %d)",
              underlying(bits),
              any(bits & Status1RegBits::CRCOk),
              any(bits & Status1RegBits::CRCReady),
              any(bits & Status1RegBits::IRq),
              any(bits & Status1RegBits::TRunning),
              any(bits & Status1RegBits::HiAlert),
              any(bits & Status1RegBits::LoAlert));
}

void Log(ErrorRegBits bits) {
  PW_LOG_INFO("ErrorReg: %d (WrErr = %d, TempErr = %d, BufferOvfl = %d, CollErr = %d, CRCErr = %d, ParityErr = %d, ProtocolErr = %d)",
              underlying(bits),
              any(bits & ErrorRegBits::WrErr),
              any(bits & ErrorRegBits::TempErr),
              any(bits & ErrorRegBits::BufferOvfl),
              any(bits & ErrorRegBits::CollErr),
              any(bits & ErrorRegBits::CRCErr),
              any(bits & ErrorRegBits::ParityErr),
              any(bits & ErrorRegBits::ProtocolErr));
}

} // namespace nfc