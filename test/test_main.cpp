#include <Arduino.h>
#include <unity.h>

void test_serviceuuid_check (void){
  TEST_ASSERT_EQUAL_STRING_MESSAGE("c8d1d262-861f-4082-947e-f383a259aaf3", SERVICE_UUID, "Warnning, SERVICE_UUID changed!");
}

void test_characteristic_check (void){
  TEST_ASSERT_EQUAL_STRING_MESSAGE("c8d1d262-861f-4082-947e-f383a259aaf3", SERVICE_UUID, "Warnning, SERVICE_UUID changed!");
}

void setup (){
  delay(100);

  UNITY_BEGIN();
  RUN_TEST(test_serviceuuid_check);
}
