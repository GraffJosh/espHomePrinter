#include "esphome/core/log.h"
#include "printer_component.h"

namespace esphome {
namespace printer_component {

static const char *TAG = "printer_component.component";

void PrinterComponent::setup() {

}

void PrinterComponent::loop() {
    ESP_LOGD("ERROR", "JPG Industries Custom Debug");
    Sleep(5);
}

void PrinterComponent::dump_config(){
    ESP_LOGCONFIG(TAG, "JPGIndustries Printer component");
}

}  // namespace printer_component
}  // namespace esphome