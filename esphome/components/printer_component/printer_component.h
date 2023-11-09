#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "test.h"
#include "thermalprinter/thermalprinter.h"

namespace esphome {
namespace printer_component {

class PrinterComponent : public uart::UARTDevice, public Component {
  public:
    void setup() override;
    void loop() override;
    void dump_config() override;
};


}  // namespace printer_component
}  // namespace esphome