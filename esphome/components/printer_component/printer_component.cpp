#include "esphome/core/log.h"
#include "printer_component.h"

namespace esphome {
namespace printer_component {

static const char *TAG = "printer_component.component";

void PrinterComponent::setup() {

}

void PrinterComponent::loop() {
    Sleep(5000);
    ESP_LOGD("ERROR", "JPG Industries Custom Debug");
}

void PrinterComponent::dump_config(){
    ESP_LOGCONFIG(TAG, "JPGIndustries UART component");
}

}  // namespace printer_component
}  // namespace esphome