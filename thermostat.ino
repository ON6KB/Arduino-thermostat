/* Thermostat arduino commandant des relais pour activer la chauffe ou le refroidissement.
Capteur de température DS18B20.
Module 2 relais 5V
Encodeur rotatif KY-040
Ecran LCD I2C
Bouton poussoir menu

*/

#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

//DS18B20 DQ connecté sur la pin D10
#define ONE_WIRE_BUS 10

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int const relais1 = 7;
int const relais2 = 8;
int const bouton_menu = 2;
int const pinCLK = 4;
int const pinDT = 5;
int const pinSW = 6;
int boutonSW = 0;
int debounce = 10;
int Compteur = 0;
int pinCLKLast;
int clkVal;
volatile boolean menu_principal = false;
boolean marche_select = true;
boolean tempMin_select = false;
boolean tempMax_select = false;
int sousmenu = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);

static const unsigned long STRUCT_MAGIC = 123456789;
static const byte STRUCT_VERSION = 1;

struct UserData {
  unsigned long magic;
  byte struct_version;
  float tempMin;
  float tempMax;
};
UserData ud;

void setup() {
  chargeEEPROM();
  pinMode(relais1, OUTPUT);
  pinMode(relais2, OUTPUT);
  pinMode(bouton_menu, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(bouton_menu), actionMenu, FALLING);
  pinMode(pinCLK, INPUT_PULLUP);
  pinMode(pinDT, INPUT_PULLUP);
  pinMode(pinSW, INPUT_PULLUP);
  pinCLKLast = digitalRead(pinCLK);
  sensors.begin();
  lcd.init();
  lcd.backlight();
}

void loop() {
  if (!menu_principal) {
    if (marche_select) {
      marche();
    } else if (tempMin_select) {
      reglagetempMin();
    } else if (tempMax_select) {
      reglagetempMax();
    }
  } else {
    menu();
  }
}

void sauvegardeEEPROM() {
  ud.magic = STRUCT_MAGIC;
  ud.struct_version = STRUCT_VERSION;
  EEPROM.put(0, ud);
}

void chargeEEPROM() {
  EEPROM.get(0, ud);
  if (ud.magic != STRUCT_MAGIC || ud.struct_version < 1) {
    ud.tempMin = 20.0;
    ud.tempMax = 22.0;
    sauvegardeEEPROM();
  }
}

void rotation() {
  clkVal = digitalRead(pinCLK);
  if (clkVal != pinCLKLast) {
    if (digitalRead(pinDT) != clkVal) {
      Compteur++;
    } else {
      Compteur--;
    }
    pinCLKLast = clkVal;
  }
}

void encodeur_menu() {
  rotation();
  if (Compteur < 0) {
    Compteur = 2;
  } else if (Compteur > 2) {
    Compteur = 0;
  }
  sousmenu = Compteur;
}

void encodeurTemp() {
  rotation();
  if (Compteur < 0) {
    Compteur = 45;
  } else if (Compteur > 45) {
    Compteur = 0;
  }
}

void actionMenu() {
  menu_principal = !menu_principal;
}

void reglagetempMin() {
  lcd.clear();
  lcd.print("Température mini");
  lcd.setCursor(0, 1);
  lcd.print(ud.tempMin);
  encodeurTemp();
  boutonSW = digitalRead(pinSW);
  if (boutonSW == LOW) {
    delay(debounce);
    if (boutonSW == LOW) {
      ud.tempMin = Compteur;
      sauvegardeEEPROM();
      tempMin_select = false;
      menu_principal = true;
    }
  }
}

void reglagetempMax() {
  lcd.clear();
  lcd.print("Température max");
  lcd.setCursor(0, 1);
  lcd.print(ud.tempMax);
  encodeurTemp();
  boutonSW = digitalRead(pinSW);
  if (boutonSW == LOW) {
    delay(debounce);
    if (boutonSW == LOW) {
      ud.tempMax = Compteur;
      sauvegardeEEPROM();
      tempMax_select = false;
      menu_principal = true;
    }
  }
}

void marche() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  lcd.clear();
  lcd.print("Temp: ");
  lcd.print(tempC);
  lcd.setCursor(0, 1);
  if (tempC < ud.tempMin) {
    digitalWrite(relais1, HIGH);
    digitalWrite(relais2, LOW);
    lcd.print("Chauffe");
  } else if (tempC > ud.tempMax) {
    digitalWrite(relais2, HIGH);
    digitalWrite(relais1, LOW);
    lcd.print("Refroidissement");
  } else {
    digitalWrite(relais1, LOW);
    digitalWrite(relais2, LOW);
    lcd.print("Temp correcte");
  }
  delay(2000);
  lcd.clear();
}

void menu() {
  marche_select = false;
  tempMin_select = false;
  tempMax_select = false;
  encodeur_menu();
  lcd.clear();
  if (sousmenu == 0) {
    lcd.print("Marche");
    boutonSW = digitalRead(pinSW);
    if (boutonSW == LOW) {
      delay(debounce);
      if (boutonSW == LOW) {
        marche_select = true;
        menu_principal = false;
      }
    }
  } else if (sousmenu == 1) {
    lcd.print("Reglage temp min");
    boutonSW = digitalRead(pinSW);
    if (boutonSW == LOW) {
      delay(debounce);
      if (boutonSW == LOW) {
        tempMin_select = true;
        menu_principal = false;
      }
    }
  } else if (sousmenu == 2) {
    lcd.print("Reglage temp max");
    boutonSW = digitalRead(pinSW);
    if (boutonSW == LOW) {
      delay(debounce);
      if (boutonSW == LOW) {
        tempMax_select = true;
        menu_principal = false;
      }
    }
  }
}
