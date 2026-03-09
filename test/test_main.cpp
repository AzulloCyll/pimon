#include <Arduino.h>
#include <unity.h>

// Deklaracje funkcji testowych
void setUp(void) {
  // Tutaj można przygotować środowisko przed każdym testem
}

void tearDown(void) {
  // Tutaj można posprzątać po każdym teście
}

void test_basic_math_works(void) {
  TEST_ASSERT_EQUAL(32, 32);
}

void test_sensor_data_struct(void) {
  // Możesz też załączyć nagłówki swoich modułów i testować funkcje logiczne.
  // Uwaga: sprzętowe testy na M5 będą wymagały fizycznego czujnika.
  int val = 5;
  TEST_ASSERT_GREATER_THAN(0, val);
}

void setup() {
  // Inicjalizacja portu szeregowego
  Serial.begin(115200);
  
  // Oczekiwanie na zainicjowanie sprzętowego USB CDC przez system:
  while (!Serial) {
    delay(10);
  }
  
  // Konieczne opóźnienie dla platformy ESP32 (szczególnie AtomS3 używającego sprzętowego USB CDC), 
  // by monitor portu zdążył się połączyć po restarcie urządzenia.
  delay(3000);
  
  UNITY_BEGIN();
  RUN_TEST(test_basic_math_works);
  RUN_TEST(test_sensor_data_struct);
  UNITY_END();
}

void loop() {
  delay(500);
}
