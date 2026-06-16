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

enum Statut {
  MenuAcceuil, 
  GameSolo,
  GameMulti,
  GameOver,
  PauseGame,
  AttenteJoueur,
  Parametres
};

Statut mode = MenuAcceuil;

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
int score= 0;
bool balleLancee = false; 

unsigned long dernierSignalRecu = 0;

uint8_t adresseEsclave[] = {0xC4, 0x4F, 0x33, 0x64, 0x8D, 0x11}; 

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

void esclaveDonneX(const esp_now_recv_info_t *recvInfo, const uint8_t *temp, int len) {
    memcpy(&messE, temp, sizeof(messE));
    dernierSignalRecu = millis(); 
}

void setup() {
  Serial.begin(115200);
  display.begin(0x3C, true);

  display.clearDisplay();
  display.setRotation(1);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  pinMode(BUTTON_C, INPUT_PULLUP);

  WiFi.mode(WIFI_AP_STA);
  
  if (esp_now_init() == ESP_OK) {
      esp_now_register_recv_cb(esclaveDonneX);
      
      esp_now_peer_info_t peerInfo = {};
      memcpy(peerInfo.peer_addr, adresseEsclave, 6); 
      peerInfo.channel = 0; 
      peerInfo.encrypt = false; 
      esp_now_add_peer(&peerInfo);
  }
  
  messE.pisition_joystick_esclave = 2048; 
  messE.modeEsclave = MenuAcceuil;

}

void loop() {
  // Envoi régulier du Maître 
  static unsigned long lastSend = 0;
  if (millis() - lastSend > 40) {
    messM.commandeMode = mode;
    messM.raquetteX_J1 = barre.x;
    messM.raquetteX_J2 = adverse.x;
    messM.balleX = balle.x;
    messM.balleY = balle.y;
    esp_now_send(adresseEsclave, (uint8_t *) &messM, sizeof(messM));
    lastSend = millis();
  }

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
      display.println(" En attente du J2...");
      display.println(" --------------------");
      
      if (millis() - dernierSignalRecu < 1000 && (messE.modeEsclave == AttenteJoueur || messE.modeEsclave == GameMulti)) {
         display.println("\n CONNEXION REUSSIE !");
         display.display();
         delay(1500); 
         
         barre.reset();        
         adverse.reset();      
         balle.reset();        
         barre.y = 120;   
         adverse.y = 4;   
         balle.y = 114;       
         balle.x = barre.x + (barre.width / 2); 
         balle.sensY = -1;    
         balle.sensX = 1;     
         balleLancee = false; 
         
         mode = GameMulti;
      } else {
         display.println(" Statut: RECHERCHE...");
         display.display();
      }

      if(!digitalRead(BUTTON_C)) {
        menu.setItems(menuPrincipal); 
        mode = MenuAcceuil;
      }
      delay(20);
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
          } else { joystickAppuyer = false; }

          // Détection du rebond sur la barre joueur
          if (balle.toucheBarre(barre)) {
            score++;
          }

          display.setCursor(0,0);
          for (int i=0; i<vie; i++){
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
      if (millis() - dernierSignalRecu > 2000) {
        mode = AttenteJoueur;
        break;
      }

      display.clearDisplay();
      barre.deplacement(analogRead(A3)); 
      adverse.deplacement(messE.pisition_joystick_esclave); 

      if (!balleLancee) {
        balle.x = barre.x + (barre.width / 2);
        balle.y = barre.y - balle.r - 2; 
        if (analogRead(A2) >= 4090) {
          balleLancee = true;
          balle.sensY = -1; 
          balle.sensX = (random(0, 2) == 0) ? -1 : 1; 
        }
      }

      if (balleLancee) {
        int nextX = balle.x + balle.sensX * balle.speed;
        if (nextX - balle.r <= 0 || nextX + balle.r >= 128) { 
          balle.sensX = -balle.sensX; 
        }
        balle.x += balle.sensX * balle.speed;
        balle.y += balle.sensY * balle.speed;

        if (balle.y + balle.r >= barre.y && balle.x >= barre.x && balle.x <= barre.x + barre.width) { 
          balle.sensY = -1; 
        }
        if (balle.y - balle.r <= adverse.y + adverse.height && balle.x >= adverse.x && balle.x <= adverse.x + adverse.width) { 
          balle.sensY = 1; 
        }
        if (balle.y > 128 || balle.y < 0) { 
          balleLancee = false; 
          balle.speed = 1;
        }
      }

      if (balle.y >= 64) { 
        display.fillCircle(balle.x, balle.y - 64, balle.r, SH110X_WHITE); 
      }
      display.fillRoundRect(barre.x, barre.y - 64, barre.width, barre.height, 4, SH110X_WHITE);
      display.display();

      if(!digitalRead(BUTTON_C)) {
        menu.setItems(menuPrincipal);
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
      display.setCursor(0,10);
      display.println("   PARAMETRES");
      display.println("\n (Pas d'options ici)");
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
      barre.reset(); 
      balle.reset();
      
    } else {  // Bouton Retour Menu
      menu.setItems(menuPrincipal);
      mode = MenuAcceuil;
    }
  }
}

void pauseGame(){
  menu.drawMenu(display);
  
  if (menu.handleJoystick()) {
    int action = menu.getSelectedValue();

    // Bouton Reprendre
    if (action == GameSolo || action == GameMulti) { 
      mode = GameSolo;

    } else {  // Bouton Retour Menu
      menu.setItems(menuPrincipal);
      mode = MenuAcceuil;
    }
  }
}