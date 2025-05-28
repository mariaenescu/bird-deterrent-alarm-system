// Enescu Maria 331CA

#include <LiquidCrystal.h>

#include <pcf8574.h>

PCF8574 ex(0x20);

#define DEBUG;

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// Definirea pinilor pentru LED-ul RGB 
int red = 0;
int green = 1;
int blue = 2;


// Pinii pentru senzorul ultrasonic
int trigger = 13;
int echoPins[] = {9, 10, 11, 12};

// Pinul pentru difuzor
int buzzer = 8;

// Pinul analogic pentru senzorul de lumina
int lum = A0;

float prev[] = {1000, 1000, 1000, 1000};
float distances[4];

// Vector cu frecventele sunetului care va fi redat
int melody[] = {
  30000, 40000, 20000, 10000, 5000, 25000, 15000, 50000, 1000, 12500
};

// Vector cu jocul luminilor RGB: 100->rosu, 010->verde, 001->albastru, etc
int leds[] = {
  0b100, 0b010, 0b001, 0b101, 0b110, 0b011, 0b111, 0b001, 0b010, 0b100
};

unsigned long noteMillis = 0;
unsigned long alarmMillis = 0;
unsigned long distanceMillis = 0;

const long noteInterval = 200;
const long alarmInterval = 100;
const long distanceInterval = 300;

int melodyIndex = 0;
int alarmIndex = 0;
int echoIndex = 0;
int alarmState = 0;

float minDist = 1000; //distanta minima pana la primul obiect detectata de senzori
int lumPercent = 100; //lumina in procente

void setup() {
  // Seteaz pinii LED-urilor ca iesiri, a senzorul ultrasonic, a  difuzorului ca iesire, senzorului de lumina ca intrare
  for(int i = 0; i < 4; i++) {
    pinMode(echoPins[i], INPUT);
  }

  pinMode(trigger, OUTPUT);
 
  pinMode(ex, red, OUTPUT);
  pinMode(ex, green, OUTPUT);
  pinMode(ex, blue, OUTPUT);

  digitalWrite(ex, red, LOW);
  digitalWrite(ex, green, LOW);
  digitalWrite(ex, blue, LOW);
 
  pinMode(buzzer, OUTPUT);
  pinMode(lum, INPUT);

  // Initializare numar coloane si randuri LCD
  lcd.begin(16,2);
  #ifdef DEBUG
    Serial.begin(9600);
  #endif
}

// Functie pentru calcularea distantei folosind senzorul ultrasonic
float distance(int echo) {
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);

  int duration = pulseIn(echo, HIGH);
  float distance = duration * 0.034 / 2;

  return distance;
}


// // Functie pentru redarea unei melodii cu difuzorul
void playMelody() {
  if(millis() - noteMillis >= noteInterval) {
    noteMillis = millis();
    tone(buzzer, melody[melodyIndex]);
    melodyIndex = (melodyIndex + 1) % 10;
  }
}

// Functie pentru activarea alarmei complete melodie si LED-ul in toate culorile RGB
void alarm() {
  if(millis() - alarmMillis >= alarmInterval) {
    alarmMillis = millis();

    switch(alarmState) {
      case 0:
        tone(buzzer, melody[alarmIndex]);
        digitalWrite(ex, red, leds[alarmIndex] & 0b100 ? HIGH : LOW);
        break;
      case 1:
        digitalWrite(ex, green, leds[alarmIndex] & 0b010 ? HIGH : LOW);
        break;
      case 2:
        digitalWrite(ex, blue, leds[alarmIndex] & 0b001 ? HIGH : LOW);
        alarmIndex = (alarmIndex + 1) % 10;
        break;
    }

    alarmState = (alarmState + 1) % 3;
  }
}

// Functie pentru oprirea sunetului de la difuzor si a LED-ului
void clear() {
  noTone(buzzer);
  ex.write(0x00);
}


void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - distanceMillis >= distanceInterval) {
    distanceMillis = currentMillis;
    distances[echoIndex] = distance(echoPins[echoIndex]);// citire valoare distanta
    // Pentru distanta negativa, folosim ultima valoare masurată pozitiv
    if (distances[echoIndex] <= 0) {
      distances[echoIndex] = prev[echoIndex];
    } else {
      prev[echoIndex] = distances[echoIndex];
    }

    #ifdef DEBUG
      Serial.print("distanta ");
      Serial.print(echoIndex + 1);
      Serial.print(": ");
      Serial.println(distances[echoIndex]);
    #endif

    echoIndex++;

    if (echoIndex >= 4) {
      echoIndex = 0;

      lcd.clear(); // curatare eran LCD la fiecare ciclu

      int l = analogRead(lum); // citire valoare lumina
      lumPercent = map(l,0,800,0,100);  // mapez valoarea luminii pentru a lucra cu procent de la 0-100%

      #ifdef DEBUG
        Serial.print("lumina: ");
        Serial.print(l);
        Serial.print(" : ");
        Serial.print(lumPercent);
        Serial.println("%");
      #endif

      // Afiseaza procentul luminii pe LCD
      lcd.setCursor(0,0);
      lcd.print("L:");
      lcd.print(lumPercent);
      lcd.print("%");

      minDist = distances[0];
      int minIndex = 0;
      for (int i = 1; i < 4; i++) {
        if (distances[i] < minDist) {
          minDist = distances[i];
          minIndex = i;
        }
      }

      if(minDist < 200) {
        // Afiseaza distanta in metri pe LCD
        lcd.setCursor(8,0);
        lcd.print("D:");
        lcd.print(minDist/100);
        lcd.print("m");

        // Afisez un mesaj pe LCD
        lcd.setCursor(1,1);
        if (minDist < 20) {
          lcd.print("ALARM");
        } else {
          lcd.print("WARNING");
        }

        lcd.setCursor(12, 1);
        lcd.print("S:");
        lcd.print(minIndex + 1);
      }

      clear();

      if (minDist > 30 && lumPercent <= 25) { 
        digitalWrite(ex, green, HIGH); 
      }
    }
  }

  if (minDist <= 30) {
    if (lumPercent > 25) {
      playMelody();
    } else {
      if (minDist > 10) {
        digitalWrite(ex, red, HIGH);
        playMelody();
      } else {
        alarm();
      }
    }
  }
}




