#include <logmem.hpp>

uint32_t heap_size = 0;

void logMemory(const char* msg) {
  if (!cfg.devmode) return;
  if (heap_size == 0) heap_size = ESP.getFreeHeap();
  heap_size = heap_size - ESP.getFreeHeap();
  Serial.printf("-->[HEAP] %s bytes used\t: %05db/%03dKb\r\n", msg, heap_size, ESP.getFreeHeap() / 1024);
  heap_size = ESP.getFreeHeap();
}

void logMemoryObjects(){
    Serial.printf("-->[HEAP] sizeof sensors\t: %05ub\r\n", sizeof(sensors));
    Serial.printf("-->[HEAP] sizeof config \t: %05ub\r\n", sizeof(cfg));
    Serial.printf("-->[HEAP] sizeof GUI    \t: %05ub\r\n", sizeof(gui));
    Serial.printf("-->[HEAP] free memory   \t: %05ub\r\n", ESP.getFreeHeap());
}