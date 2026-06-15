#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WiFi.h>          
#include <esp_now.h>       
#include "barre.h"
#include "balle.h"
#include "menu.h"  
  
#define BUTTON_A 15
#define BUTTON_B 32
#define BUTTON_C 14

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

enum State {
  MenuAcceuil,
  GameSolo,
  GameMulti,
  GameOver,
  PauseGame,
  AttenteJoueur
};

State mode = MenuAcceuil; 
unsigned long derniereReception = 0;
unsigned long tempsValidation = 0;
bool enCoursDeValidation = false;

Barre barre;       
Barre adverse;     
Balle balle;
Menu menu;

bool joystickAppuyer = false;
int tDep = 0;
int vie = 3;

uint8_t adresseMaitre[] = {0x94, 0xB9, 0x7E, 0x5F, 0x19, 0x8C}; 

struct MessageEsclave {
    int pisition_joystick_esclave; 
    int modeEsclave;
};

struct MessageMaitre {
    int commandeMode; 
    int raquetteX_J1; 
    int raquetteX_J2; 
    int balleX;       
    int balleY;       
};

MessageEsclave messE; 
MessageMaitre messM; 

void maitreDonneInfos(const esp_now_recv_info_t *recvInfo, const uint8_t *temp, int len) {
    memcpy(&messM, temp, sizeof(messM));
    derniereReception = millis(); 

    // Réponse immédiate (tourne en tâche de fond, NE BLOQUE JAMAIS)
    messE.pisition_joystick_esclave = analogRead(A3); 
    messE.modeEsclave = mode; 
    esp_now_send(adresseMaitre, (uint8_t *) &messE, sizeof(messE));
}

void setup() {
  Serial.begin(115200);
  display.begin(0x3C, true);
  Wire.setClock(200000);   // essaie 150000, 200000, 250000... trouve le max stable
  Wire.setTimeOut(50);     // évite un blocage indéfini si une transaction I2C échoue

  Wire.setClock(400000);

  display.clearDisplay();
  display.setRotation(1);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  pinMode(BUTTON_C, INPUT_PULLUP);
  WiFi.mode(WIFI_AP_STA);
  
  if (esp_now_init() == ESP_OK) {
      esp_now_register_recv_cb(maitreDonneInfos);
      esp_now_peer_info_t peerInfo = {};
      memcpy(peerInfo.peer_addr, adresseMaitre, 6); 
      peerInfo.channel = 0; 
      peerInfo.encrypt = false; 
      esp_now_add_peer(&peerInfo);
  }
}

void loop() {
  switch (mode) {
    case MenuAcceuil:
      enCoursDeValidation = false;
      menu.drawMenu(display);
      if (menu.handleJoystick(display)) {
        int choix = menu.getSelectedIndex();
        if (choix == 0) mode = AttenteJoueur; 
        if (choix == 1) { vie = 3; barre.reset(); balle.reset(); mode = GameSolo; }
      }
      delay(10);
      break;

    case AttenteJoueur:
      display.clearDisplay();
      display.setCursor(0, 10);
      display.println("   MODE MULTI");
      display.println(" En attente du J1...");
      display.println(" ----------------");
      
      // ÉTAPE 1 : Détection du Maître en jeu
      if (millis() - derniereReception < 1000 && messM.commandeMode == GameMulti && !enCoursDeValidation) {
         enCoursDeValidation = true;
         tempsValidation = millis(); // On note l'heure du succès
      }

      // ÉTAPE 2 : Gestion de l'affichage SANS bloquer la radio (Pas de delay !)
      if (enCoursDeValidation) {
         display.println("\n CONNEXION REUSSIE !");
         display.display();
         
         // Après 1,5 seconde d'affichage fluide, on bascule en jeu
         if (millis() - tempsValidation >= 1500) {
            mode = GameMulti;
         }
      } else {
         if (millis() - derniereReception < 1000) {
            display.println(" Statut: DISPO (Attente J1)");
         } else {
            display.println(" Statut: RECHERCHE...");
         }
         display.display();
      }

      if(!digitalRead(BUTTON_C)) mode = MenuAcceuil;
      delay(10);
      break;

    case GameSolo:
      display.clearDisplay();
      if (vie==0) mode=GameOver;
      else {
        if (balle.perdu()){ vie--; balle.reset(); barre.reset(); }
        else {
          adverse.y=0;
          barre.deplacement(analogRead(A3));
          balle.deplacement(barre);
          adverse.suivreBalle(balle.x);
          if(!digitalRead(BUTTON_C)) mode=MenuAcceuil;
          if (analogRead(A2)==4095) {
            if (!joystickAppuyer) { joystickAppuyer=true; tDep = millis(); } 
            else if (millis()-tDep>=1000) { joystickAppuyer = false; mode = PauseGame; }
          } else { joystickAppuyer = false; }
          display.setCursor(0,0);
          for (int i=0; i<vie; i++){ display.write(0x03); }
          balle.afficher(display);
          adverse.afficher(display);
          barre.afficher(display);
          display.display();
        }
      }  
      break;

    case GameMulti:
      // Sécurité : Retour au menu uniquement si le maître quitte volontairement
      if (millis() - derniereReception > 2000 || messM.commandeMode == MenuAcceuil) {
        mode = AttenteJoueur;
        enCoursDeValidation = false;
        break;
      }

      display.clearDisplay();
      if (messM.balleY < 64) {
        display.fillCircle(messM.balleX, messM.balleY, 2, SH110X_WHITE); 
      }
      display.fillRoundRect(messM.raquetteX_J2, 4, 30, 4, 4, SH110X_WHITE); 
      display.display();
      
      if(!digitalRead(BUTTON_C)) mode=MenuAcceuil;
      delay(5);
      break;

    case GameOver:
      display.clearDisplay(); display.setCursor(0, 0);
      display.println("Perdue"); display.display();
      delay(250);
      if (analogRead(A2)==4095){ mode=GameSolo; vie=3; barre.reset(); balle.reset(); } 
      if(!digitalRead(BUTTON_C)){ mode=MenuAcceuil; } 
      break;

    case PauseGame:
      display.clearDisplay(); display.setCursor(0, 0);
      display.println("Pause"); display.display();
      delay(250);
      if (analogRead(A2)==4095){ mode=GameSolo; barre.reset(); balle.reset(); } 
      break;
  }
}