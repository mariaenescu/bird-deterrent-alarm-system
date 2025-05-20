// Enescu Maria 331CA

#include <LiquidCrystal.h>

#include <pcf8574.h>

PCF8574 ex(0x20);

#define DEBUG;

LiquidCrystal lcd(7, 6, 5, 4, 3, 2); // initializez LCDul si specific pinii corespunzatori de pe Arduino

// Definirea pinilor pentru LED-ul RGB 
int red = 0;
int blue = 1;
int green = 2;

// Pinii pentru senzorul ultrasonic
int trigger = 13;
int echo1 = 9;
int echo2 = 10;
int echo3 = 11;
int echo4 = 12;

// Pinul pentru difuzor
int buzzer = 8;

// Pinul analogic pentru senzorul de lumina
int lum = A0;

float prev[] = {1000, 1000, 1000, 1000};
float echo[] = {echo1, echo2, echo3, echo4};

// Vector cu frecventele sunetului care va fi redat
int melody[] = {
  30000, 40000, 20000, 10000, 5000, 25000, 15000, 50000, 1000, 12500
};

// Vector cu jocul luminilor RGB: 100->rosu, 010->verde, 001->albastru, etc
int leds[] = {
  0b100, 0b010, 0b001, 0b101, 0b110, 0b011, 0b111, 0b001, 0b010, 0b100
};


void setup() {
  // Seteaz pinii LED-urilor ca iesiri, a senzorul ultrasonic, a  difuzorului ca iesire, senzorului de lumina ca intrare
  pinMode(echo1, INPUT);
  pinMode(echo2, INPUT);
  pinMode(echo3, INPUT);
  pinMode(echo4, INPUT);

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


// Functie pentru redarea unei melodii cu difuzorul
void playMelody() {
  for (int i = 0; i < 10; i++) {
    tone(buzzer, melody[i]);
    delay(200); // pune o pauza scurta intre note
  }
}

// Functie pentru activarea alarmei complete melodie si LED-ul in toate culorile RGB
void alarm() {
  for (int i = 0; i < 10; i++) {
    tone(buzzer, melody[i]);
    digitalWrite(ex, red, leds[i] & 0b100 ? HIGH : LOW);
    delay(100); // pune o pauza scurta intre culorile LED-ului
    digitalWrite(ex, green, leds[i] & 0b010 ? HIGH : LOW);
    delay(100); // pune o pauza scurta intre culorile LED-ului
    digitalWrite(ex, blue, leds[i] & 0b001 ? HIGH : LOW);
    delay(100); // pune o pauza scurta intre culorile LED-ului
  }
}

// Functie pentru oprirea sunetului de la difuzor si a LED-ului
void clear() {
  noTone(buzzer);
  ex.write(0x00);
  // digitalWrite(ex, red, LOW);
  // digitalWrite(ex, blue, LOW);
  // digitalWrite(ex, green, LOW);
}

void loop() {
  lcd.clear(); // curatare eran LCD la fiecare ciclu

  int l = analogRead(lum); // citire valoare lumina
  int lp = map(l,0,800,0,100);  // mapez valoarea luminii la pentru a lucra cu procent de la 0-100%

#ifdef DEBUG
    Serial.print("lumina: ");
    Serial.println(lp);
#endif

  // Afiseaza procentul luminii pe LCD
  lcd.setCursor(0,0);
  lcd.print("L:");
  lcd.print(lp);
  lcd.print("%");

  float min = 1000;
  int index = 0;
  for(int i = 0; i < 4; i++){
    float d = distance(echo[i]); // citire valoare distanta
    // Pentru distanta negativa, folosim ultima valoare masurată pozitiv
    if (d <= 0) {
      d = prev[i];
    } else {
      prev[i] = d;
    }

#ifdef DEBUG
      Serial.print("distanta ");
      Serial.print(i+1);
      Serial.print(": ");
      Serial.println(d);
#endif
    if (min > d){
      min = d;
      index = i;
    }
    //delay(300);
    delay(50);
  }
  
  if(min < 200) {
    // Afiseaza distanta in metri pe LCD
    lcd.setCursor(8,0);
    lcd.print("D:");
    lcd.print(min/100);
    lcd.print("m");

    // Afisez un mesaj pe LCD
    lcd.setCursor(1,1);
    if (min < 20) {
      lcd.print("ALARM");
    } else {
      lcd.print("WARNING");
    }
    

    lcd.setCursor(12, 1);
    lcd.print("S:");
    lcd.print(index + 1);
  }

  clear();
  
   // Conditii pentru activarea diferitelor componente in functie de distanta si lumina
  if (min > 30) {
    if (lp <= 25) {
      digitalWrite(ex, green, HIGH);
    } 
    delay(1000); // pauza de o secunda
  } else {
    if (lp > 25) {
      playMelody();
    } else {
      if (min > 10) {
        digitalWrite(ex, red, HIGH);
        playMelody();
      } else {
        alarm();
      }
    }
  }
}

