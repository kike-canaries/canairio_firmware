#include <Sensors.hpp>
#include <time.h>

#if (CSL_NOISE_SENSOR_SUPPORTED == 1)

namespace {
static bool isTimeValid(time_t t) {
  // 2021-01-01T00:00:00Z to avoid sending the default epoch.
  return t > 1609459200;
}
}  // namespace

bool Sensors::sendNoiseSensorTime(uint32_t unixTime) {
  if (!noiseTimeSyncEnabled) return false;

  if (!noiseSensorEnabled) return false;

  if (!noiseWireReady) {
    noiseSensorInitWire();
  }
  if (noiseWire == nullptr || noiseSensorAddress == 0) return false;

  if (!noiseSensorDevicePresent(*noiseWire, noiseSensorAddress)) {
    noiseSensorEnabled = false;
    noiseSensorAddress = 0;
    return false;
  }

  uint8_t payload[1 + sizeof(uint32_t)];
  payload[0] = CMD_SET_TIME;
  memcpy(&payload[1], &unixTime, sizeof(uint32_t));

  noiseWire->beginTransmission(noiseSensorAddress);
  size_t wrote = noiseWire->write(payload, sizeof(payload));
  uint8_t err = noiseWire->endTransmission();
  if (err == 0 && wrote == sizeof(payload)) {
    noiseLastTimeSyncMs = millis();
    return true;
  }
  return false;
}

bool Sensors::syncNoiseSensorTime() {
  if (!noiseTimeSyncEnabled || !noiseSensorEnabled) return false;

  uint32_t now_ms = millis();
  if (noiseLastTimeSyncMs != 0) {
    if ((now_ms - noiseLastTimeSyncMs) < noiseTimeSyncIntervalMs) return false;
  } else {
    if ((now_ms - noiseLastSyncAttemptMs) < NOISE_SYNC_RETRY_MS) return false;
  }
  noiseLastSyncAttemptMs = now_ms;

  time_t now = time(nullptr);
  if (!isTimeValid(now)) return false;

  return sendNoiseSensorTime(static_cast<uint32_t>(now));
}

void Sensors::setNoiseSensorTimeSyncInterval(uint32_t intervalMs) {
  noiseTimeSyncIntervalMs = intervalMs;
}

#endif
