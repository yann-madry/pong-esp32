#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WiFi.h>          
#include <esp_now.h>       
#include "barre.h"
#include "balle.h"
#include "menu.h"  
  
#define BUTTON_C 14
#define PIN_BUZZER A0

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

enum State {
  MenuAcceuil, 
  GameSolo,
  GameMulti,
  GameOver,
  PauseGame,
  AttenteJoueur,
  Parametres
};

State mode = MenuAcceuil; 
unsigned long derniereReception = 0;
unsigned long tempsValidation = 0;
bool enCoursDeValidation = false;
bool ignorePremierAppuiPause = false;

Barre barre;       
Barre adverse;     
Balle balle;

MenuItem menuPrincipal[] = {
    {"MULTI", GameMulti},
    {"SOLO", GameSolo},
    {"PARAMETRES", Parametres}
};

MenuItem menuPause[] = {
    {"REPRENDRE", GameSolo},
    {"RETOUR MENU", MenuAcceuil}
};

MenuItem menuPerduSolo[] = {
    {"REJOUER", GameSolo},
    {"RETOUR MENU", MenuAcceuil}
};

Menu menu(menuPrincipal);

bool joystickAppuyer = false;
int tDep = 0;
int vie = 3;
int score = 0;
int vieJ2=0;

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

  messE.pisition_joystick_esclave = analogRead(A3); 
  messE.modeEsclave = mode; 
  esp_now_send(adresseMaitre, (uint8_t *) &messE, sizeof(messE));
}

void setup() {
  Serial.begin(115200);
  display.begin(0x3C, true);

  display.clearDisplay();
  display.setRotation(1);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  pinMode(BUTTON_C, INPUT_PULLUP);

  pinMode(PIN_BUZZER, OUTPUT); //speaker

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
      menu.drawMenu(display);
      
      if (menu.handleJoystick()) {
        int action = menu.getSelectedValue(); 
        
        if (action == GameMulti) mode = AttenteJoueur;
        else if (action == GameSolo) { 
          vie = 3; 
          score = 0;
          barre.reset(); 
          balle.reset(); 
          mode = GameSolo; 
        }
        else if (action == Parametres) mode = Parametres;
      }
      delay(10);
      break;

    case AttenteJoueur:
      display.clearDisplay();
      display.setCursor(0, 10);
      display.println("   MODE MULTI");
      display.println(" En attente du J1...");
      display.println(" ----------------");

      if (millis() - derniereReception < 1000 && messM.commandeMode == GameMulti && !enCoursDeValidation) {
        enCoursDeValidation = true;
        tempsValidation = millis();
      }

      if (enCoursDeValidation) {
        display.println("\n CONNEXION REUSSIE !");
        display.display();

        if (millis() - tempsValidation >= 1500) {
          mode = GameMulti;
          vieJ2=3;
        }
      } 
      else {
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
      if (vie == 0) {
        menu.setItems(menuPerduSolo);
        mode = GameOver;
      }
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
          adverse.suivreBalle(balle.x);
          
          if(!digitalRead(BUTTON_C)) {
            menu.setItems(menuPrincipal);
            mode = MenuAcceuil;
          }

          // Clic Joystick pour Pause
          if (analogRead(A2) >= 4090) {
            if (!joystickAppuyer) { joystickAppuyer = true; tDep = millis(); } 
            else if (millis() - tDep >= 1000) { 
              joystickAppuyer = false; 
              menu.setItems(menuPause);
              mode = PauseGame; 
            }
          } else joystickAppuyer = false;

          // Détection du rebond sur la barre joueur
          if (balle.toucheBarre(barre)){
            score++;
            bip();
          } 

          display.setCursor(0,0);
          for (int i=0; i<vie; i++){
            display.setCursor(0, 20+i*8);
            display.write(0x03);
          }

          // Affichage du score à droite
          display.setCursor(90, 0);
          display.print("S:");
          display.print(score);

          balle.afficher(display);
          adverse.afficher(display);
          barre.afficher(display);
          display.display();
        }
      }  
      break;

    case GameMulti:
      if (millis() - derniereReception > 2000 || messM.commandeMode == MenuAcceuil) {
        mode = AttenteJoueur;
        enCoursDeValidation = false;
        vieJ2=3;
        break;
      }

      display.clearDisplay();
      if (messM.balleY < 64) {
        display.fillCircle(messM.balleX, messM.balleY, 2, SH110X_WHITE); 
      }

      if (messM.balleY > 128 || messM.balleY < 0) vieJ2--;

      display.fillRoundRect(messM.raquetteX_J2, 4, 30, 4, 4, SH110X_WHITE);

      for (int i=0; i<vieJ2; i++){
        display.setCursor(0, 20+i*8);
        display.write(0x03);
      }

      display.display();
      
      if(!digitalRead(BUTTON_C)) {
        menu.setItems(menuPrincipal);
        vieJ2=3;
        mode = MenuAcceuil;
      }
      break;

    case GameOver:
      gameOver();
      break;

    case PauseGame:
      pauseGame();
      break;
    
    case Parametres:
      display.clearDisplay();

      display.setTextColor(SH110X_WHITE);
      display.setCursor(0, 0);

      display.println("PARAMETRES");
      display.println("----------------");
      display.println("Fonctionnalites");
      display.println("a venir...");

      display.display();
      if(!digitalRead(BUTTON_C)) {
        menu.setItems(menuPrincipal);
        mode = MenuAcceuil;
      }
      delay(20);
      break;
  }
}

void gameOver(){
  menu.drawMenu(display);
  
  if (menu.handleJoystick()) {
    int action = menu.getSelectedValue();
    // Bouton Rejouer
    if (action == GameSolo || action == GameMulti) {
      mode = GameSolo; 
      vie = 3; 
      score=0;
      barre.reset(); 
      balle.reset();
    } else { // Bouton Retour Menu
      menu.setItems(menuPrincipal);
      mode = MenuAcceuil;
    }
  }
}

void pauseGame(){
  menu.drawMenu(display);

    if (ignorePremierAppuiPause) {
      if (analogRead(A2) < 4000) { // bouton relâché
        ignorePremierAppuiPause = false;
      }
    return;
  }
  
  if (menu.handleJoystick()) {
    int action = menu.getSelectedValue();
    // Bouton Reprendre
    if (action == GameSolo || action == GameMulti) { 
      mode = GameSolo;
    } else { // Bouton Retour Menu
      menu.setItems(menuPrincipal);
      mode = MenuAcceuil;
    }
  }
}

void bip() {
  tone(PIN_BUZZER, 1200, 30);
}