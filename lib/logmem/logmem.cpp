#include <logmem.hpp>
#include "ConfigApp.hpp"
#include "esp_core_dump.h"

uint32_t heap_size = 0;

void checkCoreDumpPartition() {
  Serial.println("-->[INFO] Check core dump partition..");
  esp_core_dump_init();
  esp_core_dump_summary_t *summary =
      static_cast<esp_core_dump_summary_t *>(malloc(sizeof(esp_core_dump_summary_t)));
  if (summary) {
    esp_err_t err = esp_core_dump_get_summary(summary);
    if (err == ESP_OK) {
      Serial.println("-->[INFO] Getting core dump summary ok.");
      log_i("Getting core dump summary ok.");

    } else {
      log_e("Getting core dump summary not ok. Error: %d", (int)err);
      log_e("Probably no coredump present yet.");
      log_e("esp_core_dump_image_check() = %d", esp_core_dump_image_check());
    }
    free(summary);
  }
  else {
    Serial.println("[E][INFO] Failed core dump summary malloc");
    log_e("Failed core dump summary malloc");
  }
}

void logMemory(const char* msg) {
  if (!devmode) return;
  if (heap_size == 0) heap_size = ESP.getFreeHeap();
  heap_size = heap_size - ESP.getFreeHeap();
  Serial.printf("-->[HEAP] %s bytes used\t: %05db/%03dKb\r\n", msg, heap_size, ESP.getFreeHeap() / 1024);
  heap_size = ESP.getFreeHeap();
}

void logMemoryObjects(){
    Serial.printf("-->[HEAP] sizeof sensors\t: %05ub\r\n", sizeof(sensors));
    Serial.printf("-->[HEAP] sizeof GUI    \t: %05ub\r\n", sizeof(gui));
    Serial.printf("-->[HEAP] free memory   \t: %05ub\r\n", ESP.getFreeHeap());
}