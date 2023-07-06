#include <Geohash.hpp>

inline uint8_t Geohash::singleCoordBitsPrecision(uint8_t precision, CoordType type) {
  // Single coordinate occupies only half of the total bits.
  const uint8_t bits = (precision * BITS_PER_SYMBOL) / 2;
  if (precision & 0x1 && type == LONGITUDE) {
    return bits + 1;
  }
  return bits;
}

inline Encoded Geohash::base32Decode(const char* encoded_string, size_t encoded_length) {
  extern const uint8_t geohash_base32_decode_lookup_table[256];
  Encoded result;
  for (size_t i = 0; i < encoded_length; ++i) {
    const uint8_t c = static_cast<uint8_t>(encoded_string[i]);
    const uint8_t decoded = geohash_base32_decode_lookup_table[c] & 0x1F;
    result[i * 5 + 4] = (decoded >> 0) & 0x01;
    result[i * 5 + 3] = (decoded >> 1) & 0x01;
    result[i * 5 + 2] = (decoded >> 2) & 0x01;
    result[i * 5 + 1] = (decoded >> 3) & 0x01;
    result[i * 5 + 0] = (decoded >> 4) & 0x01;
  }
  return result;
}

inline float Geohash::decodeCoordinate(const Encoded& coord, float min, float max, uint8_t bits) {
  float mid = (max + min) / 2;
  for (size_t i = 0; i < bits; ++i) {
    const auto c = coord[i];
    if (c == 1) {
      min = mid;
    } else {
      max = mid;
    }
    mid = (max + min) / 2;
  }
  return mid;
}

inline std::tuple<Encoded, Encoded> Geohash::split(const Encoded& combined, uint8_t precision) {
  Encoded lat, lon;
  lat.fill(0);
  lon.fill(0);
  size_t i = 0;
  for (; i < precision * BITS_PER_SYMBOL - 1; i += 2) {
    // longitude is even bits
    lon[i / 2] = combined[i];
    lat[i / 2] = combined[i + 1];
  }
  // precision is even, read the last bit as lat.
  if (precision & 0x1) {
    lon[i / 2] = combined[precision * BITS_PER_SYMBOL - 1];
  }
  return std::tie(lon, lat);
}

void Geohash::decode(const char* encoded_string, size_t encoded_len, float* longitude, float* latitude) {
  const uint8_t precision = std::min(encoded_len, static_cast<size_t>(MAX_PRECISION));
  if (precision == 0) {
    // Empty string is converted to (0, 0)
    *longitude = 0;
    *latitude = 0;
    return;
  }
  Encoded lat_encoded, lon_encoded;
  std::tie(lon_encoded, lat_encoded) = split(base32Decode(encoded_string, precision), precision);
  *longitude = decodeCoordinate(lon_encoded, LON_MIN, LON_MAX, singleCoordBitsPrecision(precision, LONGITUDE));
  *latitude = decodeCoordinate(lat_encoded, LAT_MIN, LAT_MAX, singleCoordBitsPrecision(precision, LATITUDE));
}