
#include <Arduino.h>
#include <DHT.h>

#define DHTPIN 4         // DHT22 Data Pin
#define DHTTYPE DHT22
#define RED_LED 14       
#define GREEN_LED 27     
#define BLUE_LED 26      
#define BUZZER_PIN 25    
#define SWITCH_LEFT 35   
#define SWITCH_RIGHT 34  
#define DEBOUNCE_DELAY 50 /

DHT dht(DHTPIN, DHTTYPE);

// Variaveis
float humidity, temperature;
volatile bool switchLeftFlag = false;
volatile bool switchRightFlag = false;
volatile unsigned long lastInterruptTime = 0;
unsigned long switchTime = 0;
const unsigned long SWITCH_DURATION = 2000; 
bool systemInitialized = false;


struct Color {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

const Color PINK = {255, 20, 147};
const Color CYAN = {27, 255, 245};
const Color WHITE = {255, 255, 255};
const Color OFF = {0, 0, 0};

void IRAM_ATTR handleSwitchLeft() {
  if (millis() - lastInterruptTime > DEBOUNCE_DELAY) {
    switchLeftFlag = true;
    lastInterruptTime = millis();
  }
}

void IRAM_ATTR handleSwitchRight() {
  if (millis() - lastInterruptTime > DEBOUNCE_DELAY) {
    switchRightFlag = true;
    lastInterruptTime = millis();
  }
}

void setLEDColor(Color color) {
  analogWrite(RED_LED, color.red);
  analogWrite(GREEN_LED, color.green);
  analogWrite(BLUE_LED, color.blue);
}

void playBuzzer(uint16_t frequency, uint16_t duration) {
  if (frequency > 0) {
    tone(BUZZER_PIN, frequency, duration);
  } else {
    noTone(BUZZER_PIN);
  }
}

void readDHT22() {
  static unsigned long lastReadTime = 0;
  const unsigned long READ_INTERVAL = 2000; // Read every 2 seconds
  
  if (millis() - lastReadTime >= READ_INTERVAL) {
    float newHumidity = dht.readHumidity();
    float newTemperature = dht.readTemperature();
    
    if (!isnan(newHumidity) && !isnan(newTemperature)) {
      humidity = newHumidity;
      temperature = newTemperature;
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.print("°C, Humidity: ");
      Serial.print(humidity);
      Serial.println("%");
    } else {
      Serial.println("Failed to read from DHT sensor!");
      setLEDColor({255, 0, 0}); // Red for error
      playBuzzer(1000, 200);
    }
    lastReadTime = millis();
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000); // Wait for serial or timeout
  
  // Inicializar dht 
  dht.begin();
  
  ledcSetup(0, 5000, 8); // Channel 0
  ledcSetup(1, 5000, 8); // Channel 1
  ledcSetup(2, 5000, 8); // Channel 2
  ledcAttachPin(RED_LED, 0);
  ledcAttachPin(GREEN_LED, 1);
  ledcAttachPin(BLUE_LED, 2);
  
  //  Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  
  pinMode(SWITCH_RIGHT, INPUT_PULLDOWN);
  pinMode(SWITCH_LEFT, INPUT_PULLUP);

  
  attachInterrupt(digitalPinToInterrupt(SWITCH_LEFT), handleSwitchLeft, RISING);
  attachInterrupt(digitalPinToInterrupt(SWITCH_RIGHT), handleSwitchRight, RISING);

  // System startup 
  setLEDColor(WHITE);
  playBuzzer(2000, 100);
  delay(100);
  playBuzzer(2500, 100);
  setLEDColor(OFF);
  
  Serial.println("System Initialized Successfully");
  systemInitialized = true;
}

void loop() {
  if (!systemInitialized) return;

  // Read sensor data
  readDHT22();

  // Handle switch interrupts
  if (switchLeftFlag || switchRightFlag) {
    switchTime = millis();
    
    if (switchLeftFlag) {
      setLEDColor(PINK);
      Serial.println("Switch LEFT - Pink for 2 seconds");
      playBuzzer(1500, 100);
      switchLeftFlag = false;
    } else {
      setLEDColor(CYAN);
      Serial.println("Switch RIGHT - Cyan for 2 seconds");
      playBuzzer(2000, 100);
      switchRightFlag = false;
    }
  }

      // apaga dps de um tempo  o led
  if (millis() - switchTime > SWITCH_DURATION && switchTime != 0) {
    setLEDColor(OFF);
    switchTime = 0;
  }

  // cor baseada na temperatura
  if (switchTime == 0) {
    if (temperature > 30.0) {
      setLEDColor({255, 0, 0}); // vermelho pra quente
   } else if (temperature < 15.0) {
      setLEDColor({0, 0, 255}); //  azul pra frio
    } else {
      setLEDColor({0, 255, 0}); // verde pra normal
    }
  }
}