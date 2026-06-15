#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WiFi.h>          // Ajouté pour la communication radio
#include <esp_now.h>       // Ajouté pour la communication radio
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
  PauseGame
};

State mode = MenuAcceuil;

Barre barre;
Barre adverse;
Balle balle;
Menu menu;

bool joystickAppuyer=false;
int tDep=0;
int vie=3;
int nextX = 0; // Déclaré ici pour éviter l'erreur "jump to case label"

// ============================================================================
//                       STRUCTURES RÉSEAU MAÎTRE / ESCLAVE
// ============================================================================
uint8_t adresseEsclave[] = {0xC4, 0x4F, 0x33, 0x64, 0x8D, 0x11}; // MAC de l'ESP de Thomas

struct MessageEsclave {
    int pisition_joystick_esclave; // Position du joystick envoyée par l'esclave
};

struct MessageMaitre {
    int commandeMode; // Envoie le mode (Ex: GameMulti) pour FORCER l'esclave à changer d'écran
    int raquetteX_J1; // Position X de ta raquette (Maître)
    int raquetteX_J2; // Position X de la raquette de Thomas (Esclave)
    int balleX;       // Coordonnée X globale de la balle
    int balleY;       // Coordonnée Y globale de la balle (sur un terrain virtuel de 0 à 128)
};

MessageEsclave messE; 
MessageMaitre messM; 

// Callback de réception : appelé automatiquement quand Thomas envoie son joystick
void esclaveDonneX(const uint8_t *mac, const uint8_t *temp, int len) {
    memcpy(&messE, temp, sizeof(messE));
}
// ============================================================================

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

  pinMode(BUTTON_C, INPUT_PULLUP);

  // Configuration de l'ESP-NOW pour le sans-fil
  WiFi.mode(WIFI_STA);
  if (esp_now_init() == ESP_OK) {
      esp_now_register_recv_cb(esp_now_recv_cb_t(esclaveDonneX));
      esp_now_peer_info_t peerInfo;
      memcpy(peerInfo.peer_addr, adresseEsclave, 6); 
      peerInfo.channel = 10;
      peerInfo.encrypt = false; 
      esp_now_add_peer(&peerInfo);
  }
}

void loop() {
  // On envoie en continu le mode au drone Esclave pour le téléguider à distance
  messM.commandeMode = mode;
  if (mode != GameMulti) {
    esp_now_send(adresseEsclave, (uint8_t *) &messM, sizeof(messM));
  }

  switch (mode) {
    case MenuAcceuil:
      menu.drawMenu(display);
      if (menu.handleJoystick(display)) {
        menu.executeAction(menu.getSelectedIndex(), display);
        int choix = menu.getSelectedIndex();
        if (choix == 0) {
          // --- INITIALISATION DU TERRAIN MULTIJOUEUR ÉLARGI (0 à 128) ---
          barre.reset();        
          adverse.reset();      
          balle.reset();        
          
          barre.y = 120;   // Ta raquette se place tout au bout du terrain virtuel (bas de l'écran 2)
          adverse.y = 4;   // La raquette de Thomas au tout début du terrain virtuel (haut de l'écran 1)
          balle.y = 64;    // La balle démarre pile au milieu (entre vos deux écrans)
          balle.sensY = 1; // Elle se dirige d'abord vers toi
          
          mode = GameMulti;
        }
        if (choix == 1) mode = GameSolo;
      }
      delay(10);
      break;

    case GameSolo:
      display.clearDisplay();
      if (vie==0) mode=GameOver;
      else {
        if (balle.perdu()){
          vie--;
          balle.reset();
          barre.reset();
        }
        else {
          adverse.y=0;
          barre.deplacement(analogRead(A3));
          balle.deplacement(barre);
          adverse.deplacement(balle.x);
          
          if(!digitalRead(BUTTON_C)){
            vie=3;
            barre.reset();
            balle.reset();
            menu.reset();
            mode=MenuAcceuil;
          }

          if (analogRead(A2)==4095) {
            if (joystickAppuyer==false) {
              joystickAppuyer=true;
              tDep = millis();
            } 
            else if (millis()-tDep>=1000) { 
              joystickAppuyer = false;
              mode = PauseGame;
            }
          } 
          else {
            joystickAppuyer = false;
          }
          display.setCursor(0,0);
          for (int i=0; i<vie; i++){
            display.write(0x03);
          }
          balle.afficher(display);
          adverse.afficher(display);
          barre.afficher(display);
          display.display();
        }
      }  
      break;

    case GameMulti:
      display.clearDisplay();

      // 1. Gestion des mouvements des raquettes
      barre.deplacement(analogRead(A2));                  // Ton joystick fait bouger ton joueur (J1)
      adverse.deplacement(messE.pisition_joystick_esclave); // Le joystick reçu par radio fait bouger Thomas (J2)

      // 2. PHYSIQUE DU DOUBLE-ÉCRAN (Calculée à 100% par le Maître)
      // Mouvement sur l'axe X (Rebonds latéraux gauche/droite)
      nextX = balle.x + balle.sensX * balle.speed;
      if (nextX - balle.r <= 0 || nextX + balle.r >= 128) {
        balle.sensX = -balle.sensX;
      }
      balle.x += balle.sensX * balle.speed;

      // Mouvement sur l'axe Y (La balle voyage de y=0 à y=128)
      balle.y += balle.sensY * balle.speed;

      // Collision avec TA raquette (Maître, située en bas du terrain global à y=120)
      if (balle.y + balle.r >= barre.y && balle.x >= barre.x && balle.x <= barre.x + barre.width) {
        balle.sensY = -1; // Elle repart vers le haut (direction l'écran de Thomas)
        if (balle.sensX == 0) balle.sensX = 1;
      }

      // Collision avec la raquette de THOMAS (Esclave, située en haut du terrain global à y=4)
      if (balle.y - balle.r <= adverse.y + adverse.height && balle.x >= adverse.x && balle.x <= adverse.x + adverse.width) {
        balle.sensY = 1; // Elle repart vers le bas (direction ton écran)
      }

      // Gestion des points manqués
      if (balle.y > 128) { // Tu as loupé la balle
        balle.reset();
        balle.y = 64; balle.sensY = -1; // On réinitialise au milieu, elle part vers Thomas
      }
      if (balle.y < 0) { // Thomas a loupé la balle
        balle.reset();
        balle.y = 64; balle.sensY = 1; // On réinitialise au milieu, elle part vers toi
      }

      // 3. ENVOI DES COORDONNÉES RÉELLES À THOMAS POUR APPARITION DE LA BALLE
      messM.raquetteX_J1 = barre.x;
      messM.raquetteX_J2 = adverse.x;
      messM.balleX = balle.x;
      messM.balleY = balle.y; // Très important pour que l'Esclave sache où elle est
      esp_now_send(adresseEsclave, (uint8_t *) &messM, sizeof(messM));

      // 4. RENDU SUR TON ÉCRAN (MAÎTRE)
      // Rappel : Toi tu n'affiches que la moitié basse du terrain virtuel (de y=64 à y=128)
      // On soustrait donc -64 au Y de la balle et de ta raquette pour l'affichage local.
      
      if (balle.y >= 64) { // La balle n'apparaît chez toi QUE s'il elle a franchi la moitié basse
        display.fillCircle(balle.x, balle.y - 64, balle.r, SH110X_WHITE);
      }
      
      // Dessin de ta raquette recalibrée pour ton écran physique de 64px
      display.fillRoundRect(barre.x, barre.y - 64, barre.width, barre.height, 4, SH110X_WHITE);
      display.display();

      if(!digitalRead(BUTTON_C)){
        barre.reset();
        balle.reset();
        menu.reset();
        mode=MenuAcceuil;
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
  display.setCursor(0, 0);
  display.println("Perdue");
  display.println("Cliquer sur le joystick pour rejouer");
  display.print("Ou C pour retourner au Menu");
  display.display();
  delay(250);
  if (analogRead(A2)==4095){
    mode=GameSolo;
    vie=3;
    barre.reset();
    balle.reset();
  } 
  if(!digitalRead(BUTTON_C)){
    mode=MenuAcceuil;
    vie=3;
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
  if (analogRead(A2)==4095){
    mode=GameSolo;
    barre.reset();
    balle.reset();
  }  
}