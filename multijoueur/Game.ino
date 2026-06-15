#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WiFi.h>
#include <esp_now.h>
#include "FeatherShieldPinouts.h"
#include "barre.h"
#include "balle.h"

// ============================================================================
//   CONTOURNEMENT DU CODE DÉFECTUEUX DU MENU (Sans toucher à ses fichiers)
// ============================================================================
#define MENU_H // Bloque l'inclusion du fichier menu.h original qui ne compile pas

class Menu {
  private:
    int r = 4;
    int currentIndex = 0;
    unsigned long lastMoveTime = 0;
    static const int NUM_ITEMS = 3;
    const char* menuItems[NUM_ITEMS] = {"MULTI", "SOLO", "PARAMETRES"};

  public:
    Menu() {}
    
    void reset() { 
      currentIndex = 0; 
    }
    
    int getSelectedIndex() const { 
      return currentIndex; 
    }

    bool handleJoystick(Adafruit_SH1107& display) {
      int val = analogRead(34); // Pin A2 = 34 selon FeatherShieldPinouts.h
      if (millis() - lastMoveTime > 250) {
        if (val < 1000 && currentIndex > 0) {
          currentIndex--;
          lastMoveTime = millis();
        }
        else if (val > 2000 && val < 4000 && currentIndex < NUM_ITEMS - 1) { 
          currentIndex++;
          lastMoveTime = millis();
        }
        else if (val == 4095) {
          lastMoveTime = millis();
          return true;
        }
      }
      return false;
    }

    void drawMenu(Adafruit_SH1107& display) {
      display.clearDisplay();
      display.setTextSize(1);

      int hY = (currentIndex == 0) ? 2 : (currentIndex == 1) ? 20 : 41;
      display.fillRoundRect(2, hY, 123, (currentIndex == 1) ? 24 : 20, r, SH110X_WHITE);

      for (int i = 0; i < NUM_ITEMS; i++) {
        bool isSel = (currentIndex == i);
        display.setTextColor(isSel ? SH110X_BLACK : SH110X_WHITE);
        display.setCursor(8, 8 + (i * 20));

        if (isSel) display.write(0x10);
        else if (i == 0 && currentIndex > 0) display.write(0x18);
        else if (i == 2 && currentIndex < 2) display.write(0x19);
        else display.print(" ");
        
        display.print("  "); display.print(menuItems[i]);
      }
      display.drawRoundRect(0, 0, 127, 63, r, SH110X_WHITE);
      display.display();
    }

    void executeAction(int index, Adafruit_SH1107& display) {
      Serial.printf("Action : %s\n", menuItems[index]);
      for(int i = 0; i < 2; i++) {
        display.invertDisplay(true); delay(80);
        display.invertDisplay(false); delay(80);
      }
    }
};
// ============================================================================

#define BUTTON_A 15
#define BUTTON_B 32
#define BUTTON_C 14

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

enum State {
  MenuAcceuil,
  GameSolo,
  GameMulti,
  GameOver,
  PauseGame
};

State mode = MenuAcceuil;

Barre barre;   // Ta barre pour le mode Solo
Balle balle;   // La balle pour le mode Solo et Multi
Menu menu;

bool joystickAppuyer = false;
int tDep = 0;
int vie = 3;

// ============================================================================
//                      VARIABLES ET STRUCTURES MULTIJOUEUR
// ============================================================================
uint8_t adresseEsclave[] = {0xC4, 0x4F, 0x33, 0x64, 0x8D, 0x11}; // MAC de Thomas

struct MessageEsclave {
    int pisition_joystick_esclave; // Position du joystick de l'esclave
};

struct MessageMaitre {
    int raquetteX_J1; // Ta raquette (Maitre) en X
    int raquetteX_J2; // Raquette de Thomas (Esclave) en X
    int balleX;
    int balleY;
};

MessageEsclave messE; 
MessageMaitre messM; 

Barre barreJ1; // Ta raquette en bas (Multi)
Barre barreJ2; // Raquette de Thomas en haut (Multi)

// Fonction callback lors de la réception radio depuis l'esclave
void esclaveDonneX(const uint8_t *mac, const uint8_t *temp, int len) {
    memcpy(&messE, temp, sizeof(messE));
}

void setup() {
  Serial.begin(115200);
  display.begin(0x3C, true);

  display.display();
  delay(800);
  display.clearDisplay();
  display.display();

  display.setRotation(1);

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  // Configuration WiFi & ESP-NOW pour le multijoueur
  WiFi.mode(WIFI_STA);
  if (esp_now_init() == ESP_OK) {
      esp_now_register_recv_cb(esp_now_recv_cb_t(esclaveDonneX));
      esp_now_peer_info_t peerInfo;
      memcpy(peerInfo.peer_addr, adresseEsclave, 6); 
      peerInfo.channel = 10;
      peerInfo.encrypt = false; 
      esp_now_add_peer(&peerInfo);
  }
  
  barreJ2.y = 0; // On force la raquette de Thomas à se mettre tout en haut de l'écran
}

void loop() {
  switch (mode) {
    case MenuAcceuil:
      menu.drawMenu(display);
      if (menu.handleJoystick(display)) {
        menu.executeAction(menu.getSelectedIndex(), display);
        int choix = menu.getSelectedIndex();
        if (choix == 0) mode = GameMulti;
        if (choix == 1) mode = GameSolo;
      }
      delay(10);
      break;

    case GameSolo:
      display.clearDisplay();
      if (vie == 0) {
        mode = GameOver;
        break;
      }
      if (balle.perdu()){
        vie--;
        balle.reset();
        barre.reset();
      }
      else {
        barre.deplacement(analogRead(39)); // Pin A3 = 39
        balle.deplacement(barre);
        
        if(!digitalRead(BUTTON_C)){
          barre.reset();
          balle.reset();
          menu.reset();
          mode = MenuAcceuil;
        }

        if (analogRead(34) == 4095) { // Pin A2 = 34
          if (joystickAppuyer == false) {
            joystickAppuyer = true;
            tDep = millis();
          } 
          else {
            if (millis() - tDep >= 1000) { 
              joystickAppuyer = false;
              mode = PauseGame; 
            }
          }
        } 
        else {
          joystickAppuyer = false;
        }
        balle.afficher(display);
        barre.afficher(display);
        display.display();
      }      
      break;

    case GameMulti:
      display.clearDisplay();

      // 1. Déplacement des deux raquettes
      barreJ1.deplacement(analogRead(34));                 // Ton joystick local (A2)
      barreJ2.deplacement(messE.pisition_joystick_esclave); // Joystick reçu par radio depuis l'esclave

      // 2. Gestion de la physique de la balle (Calculée par le Maître)
      balle.deplacement(barreJ1); 

      // 3. Remplissage des données à envoyer à Thomas
      messM.raquetteX_J1 = barreJ1.x;
      messM.raquetteX_J2 = barreJ2.x;
      messM.balleX = balle.x;
      messM.balleY = balle.y;

      // 4. Envoi du paquet réseau
      esp_now_send(adresseEsclave, (uint8_t *) &messM, sizeof(messM));

      // 5. Rendu graphique sur ton écran
      barreJ1.afficher(display);
      barreJ2.afficher(display);
      balle.afficher(display);
      display.display();

      // Retour au menu via le bouton C
      if(!digitalRead(BUTTON_C)){
        barreJ1.reset();
        barreJ2.reset();
        balle.reset();
        menu.reset();
        mode = MenuAcceuil;
      }
      break;

    case GameOver:
      gameOver();
      break;

    case PauseGame:
      pauseGame();
      break;
  }
}

void gameOver(){
  display.clearDisplay();
  vie = 3;
  display.setCursor(0, 0);
  display.println("Perdue");
  display.println("Cliquer sur le joystick pour rejouer");
  display.print("Ou C pour retourner au Menu");
  display.display();

  delay(250);
  if (analogRead(34) == 4095){
    mode = GameSolo;
    barre.reset();
    balle.reset();
  } 
  if(!digitalRead(BUTTON_C)){
    mode = MenuAcceuil;
    barre.reset();
    balle.reset();
  }   
}

void pauseGame(){
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Pause");
  display.println("Cliquer sur le joystick pour reprendre");
  display.print("Ou maintener le pour aller au Menu");
  display.display();

  delay(250);
  if (analogRead(34) == 4095){
    mode = GameSolo;
    barre.reset();
    balle.reset();
  }  
}