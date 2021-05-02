/* Thermostat arduino commandant des relais pour activer la chauffe ou le refroidissement.
Capteur de température DS18B20.
Module 2 relais 5V
Encodeur rotatif KY-040
Ecran LCD I2C

*/

// Wire for avr - Version: Latest 
#include <Wire.h>

// OneWire - Version: Latest 
#include <OneWire.h>

// DallasTemperature - Version: Latest 
#include <DallasTemperature.h>


// LiquidCrystal I2C - Version: Latest 
#include <LiquidCrystal_I2C.h>

#include <EEPROM.h>


//DS18B20 DQ connecté sur la pin D10
#define ONE_WIRE_BUS 10

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Constante des relais
int const relais1 = 2;
int const relais2 = 3;

//CLK KY-040 sur D4
int const pinCLK = 4;
//DT KY-040 sur D5
int const pinDT = 5;
//bouton
int const pinSW = 6;
//Compteur position encodeur rotatif
int Compteur = 0;
//dernière valeur de CLK
int pinCLKLast;
int clkVal;
boolean Direction;
boolean menu_principal = false;
boolean marche_select = true;
boolean tempMin_select = false;
boolean tempMax_select = false;
int sousmenu = 0;

//I2C LCD 16x2
LiquidCrystal_I2C lcd(0x27, 16, 2);

/** Le nombre magique et le numéro de version actuelle */
static const unsigned long STRUCT_MAGIC = 123456789;
static const byte STRUCT_VERSION = 1;


//Structure pour garder paramètre en mémoire
struct UserData {
  unsigned long magic;
  byte struct_version;
  float tempMin;
  float tempMax;
};
//Instance de la structure
UserData ud;

void setup() {
  // Charge le contenu de la mémoire
  chargeEEPROM();
  //Pin du relais
  pinMode (relais1, OUTPUT);
  pinMode (relais2, OUTPUT);
  //pin du KY-040
  pinMode (pinCLK, INPUT_PULLUP);
  pinMode (pinDT, INPUT_PULLUP);
  pinMode (pinSW, INPUT_PULLUP);
  // Démarage du capteur de température
  sensors.begin();
  //démarrage du LCD
  lcd.init();

}

void loop() { 
  //par défaut démarrage sur fct marche
  if (menu_principal == false) {
    if (marche_select == true){
    marche();
      
    }
    //réglage de la température mini
    else if (tempMin_select == true){
      reglagetempMin();
    }
    //réglage de la température max
    else if (tempMax_select == true){
      reglagetempMax();
    }
  }

  //entrée dans le menu
  else {
    menu();
    
  }

    
}

/** Sauvegarde en mémoire EEPROM le contenu actuel de la structure */
void sauvegardeEEPROM() {
  // Met à jour le nombre magic et le numéro de version avant l'écriture
  ud.magic = STRUCT_MAGIC;
  ud.struct_version =  STRUCT_VERSION;
  EEPROM.put(0, ud);
}

/** Charge le contenu de la mémoire EEPROM dans la structure */
void chargeEEPROM() {

  // Lit la mémoire EEPROM
  EEPROM.get(0, ud);
  
  // Détection d'une mémoire non initialisée
  byte erreur = ud.magic != STRUCT_MAGIC;
  // Valeurs par défaut struct_version == 0
  if (erreur) {

    // Valeurs par défaut pour les variables de la version 0
    ud.tempMin = 20.0;
  }
  
  // Valeurs par défaut struct_version == 1
  if (ud.struct_version < 1 || erreur) {

    // Valeurs par défaut pour les variables de la version 1
    ud.tempMax = 22.0;
  }
 // Sauvegarde les nouvelles données
  sauvegardeEEPROM();
}
void reglagetempMin (){
  int tempMin = ud.tempMin;
  encodeur_temp();
  lcd.print("Température mini");
  lcd.setcursor(0,1);
  lcd.print(tempMin)
}
void reglagetempMax (){
}
void marche (){
    //commande pour obtenir la température
  sensors.requestTemperatures();
  //Variable avec la température actuel en degré celsius
  float tempC = sensors.getTempCByIndex(0);
  //affichage de la température actuel
  lcd.print(tempC);
  
  //Compare la température actuel avec la température mini
  if (tempC<ud.tempMin) {
    //active le relais de chauffe
    digitalWrite(relais1,HIGH);
    digitalWrite(relais2,LOW);
    //affiche que la chauffe est en cours
    lcd.setCursor(0, 1);
    lcd.print("Chauffe");
  }

  //compare la température actuel avec la température max
  else if(tempC>ud.tempMax){
    //active le relais de refroidissement
    digitalWrite(relais2,HIGH);
    digitalWrite(relais1,LOW);
    //affiche que le refroidissement est en cours
    lcd.setCursor(0,1);
    lcd.print("Refroidissement");
  }

  else { 
    //coupe tout relais
    digitalWrite(relais1,LOW);
    digitalWrite(relais2,LOW);
    //affiche que la température est bonne
    lcd.setCursor(0,1);
    lcd.print("température correcte");
  }

  lcd.clear();
  
}
void menu (){
  //remise à false des variables
  boolean marche_select = false;
  boolean tempMin_select = false;
  boolean tempMax_select = false;
  //Appel fct encodeur menu
  encodeur_menu ();
  if (sousmenu = 0){
    lcd.clear();
    lcd.print("Marche");
  }
  else if (sousmenu = 1)
  {
    lcd.clear();
    lcd.print("Réglage température min");
  }
  else if (sousmenu = 2)
  {
    lcd.clear();
    lcd.print("réglage température max");
  }
  
}
void encodeur_menu () {
//retour à 2 si compteur<0
if (compteur<0){
  compteur = 2;
}
//retour à 0 si compteur>2
else if (compteur>2){
  compteur = 0;
  
}
else {
  sousmenu = compteur;
}

}
void encodeur_temp (){
  //limite les réglages entre 0 et 45
  if(compteur<0) {
    compteur=45;
  } 
  if (compteur>45) {
    compteur=0;
  }
  
  
}
