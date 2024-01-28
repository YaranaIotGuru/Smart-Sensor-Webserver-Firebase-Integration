#include <WiFi.h>
#include <IOXhop_FirebaseESP32.h>
#include <DHT.h>

#define FIREBASE_HOST "androidstatus-8ebf1.firebaseio.com"

//https://androidstatus-8ebf1.firebaseio.com/

#define FIREBASE_AUTH "AIzaSyAdD0OqUE8lO2JBjaHVE4OUdPZu9n2kO-s"
#define WIFI_SSID "Abhishek"
#define WIFI_PASSWORD "12345678"
#define DHT_PIN 16       // Define the pin to which the DATA pin of the DHT sensor is connected
#define DHT_TYPE DHT11   // Set the type of DHT sensor you are using (DHT11 or DHT22)
#define ULTRASONIC_PIN_TRIGGER 2  // Set the pin for the ultrasonic sensor trigger
#define ULTRASONIC_PIN_ECHO 4     // Set the pin for the ultrasonic sensor echo
#define BUZZER_PIN 5  // Set the pin for the buzzer

DHT dht(DHT_PIN, DHT_TYPE);

unsigned long previousMillisDHT = 0;
unsigned long previousMillisUltrasonic = 0;
const long intervalDHT = 2000;       // Interval for DHT sensor readings in milliseconds
const long intervalUltrasonic = 1000;  // Interval for ultrasonic sensor readings in milliseconds

void setup() {
  Serial.begin(9600);
  delay(1000);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  delay(1000);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  pinMode(ULTRASONIC_PIN_TRIGGER, OUTPUT);
  pinMode(ULTRASONIC_PIN_ECHO, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);

  Serial.print("IP Address is : ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

float getUltrasonicDistance() {
  // Trigger ultrasonic sensor
  digitalWrite(ULTRASONIC_PIN_TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_PIN_TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_PIN_TRIGGER, LOW);

  // Measure the duration of the pulse on the echo pin
  float duration = pulseIn(ULTRASONIC_PIN_ECHO, HIGH);

  // Convert the duration to distance (in cm)
  float distance = (duration * 0.0343) / 2;

  return distance;
}

void loop() {
  unsigned long currentMillis = millis();

  // Read DHT sensor data every 2 seconds
  if (currentMillis - previousMillisDHT >= intervalDHT) {
    previousMillisDHT = currentMillis;

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    int roundedTemperature = round(temperature);
    int roundedHumidity = round(humidity);

    Firebase.setInt("values/temperature", roundedTemperature);
    Firebase.setInt("values/humidity", roundedHumidity);

    Serial.print("Temperature: ");
    Serial.print(roundedTemperature);
    Serial.print(" °C, Humidity: ");
    Serial.print(roundedHumidity);
    Serial.println(" %");

    // Check if temperature is above 25°C
    if (roundedTemperature > 25) {
      activateBuzzer();
    }
  }

  // Read ultrasonic sensor data every 1 second
  if (currentMillis - previousMillisUltrasonic >= intervalUltrasonic) {
    previousMillisUltrasonic = currentMillis;

    float distance = getUltrasonicDistance();
    int roundedDistance = round(distance);

    Firebase.setInt("values/waterLevel", roundedDistance);

    Serial.print("Distance: ");
    Serial.print(roundedDistance);
    Serial.println(" cm");

    // Check if distance is less than or equal to 15 cm
    if (roundedDistance <= 15) {
      activateBuzzer();
    }
  }

  // Print rounded data to Serial Monitor

  delay(1000);  // Delay for 1 second between Firebase updates
}

void activateBuzzer() {
  // Activate the buzzer (you can customize this function based on your buzzer type and wiring)
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);  // Buzz for 1 second (adjust as needed)
  digitalWrite(BUZZER_PIN, LOW);
}
