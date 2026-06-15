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

Barre barre;
Barre adverse;
Balle balle;
Menu menu;

bool joystickAppuyer = false;
int tDep = 0;
int vie = 3;
bool balleLancee = false; 

unsigned long dernierSignalRecu = 0;

// Mets ici l'adresse MAC réelle de l'ESP Esclave (Thomas)
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

  Wire.setClock(400000);

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
  // Envoi régulier du Maître (toutes les 40ms)
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
      display.println(" En attente du J2...");
      display.println(" ----------------");
      
      if (millis() - dernierSignalRecu < 1000 && (messE.modeEsclave == AttenteJoueur || messE.modeEsclave == GameMulti)) {
         display.println("\n CONNEXION REUSSIE !");
         display.display();
         delay(1500); // Même durée d'attente fluide que l'esclave
         
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

      if(!digitalRead(BUTTON_C)) mode = MenuAcceuil;
      delay(20);
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
          adverse.deplacement(balle.x);
          
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
      // SÉCURITÉ : Si l'esclave ne répond plus depuis plus de 2 secondes, on repasse en attente
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
        if (analogRead(A2) == 4095) {
          balleLancee = true;
          balle.sensY = -1; 
          balle.sensX = (random(0, 2) == 0) ? -1 : 1; 
        }
      }

      if (balleLancee) {
        int nextX = balle.x + balle.sensX * balle.speed;
        if (nextX - balle.r <= 0 || nextX + balle.r >= 128) { balle.sensX = -balle.sensX; }
        balle.x += balle.sensX * balle.speed;
        balle.y += balle.sensY * balle.speed;

        if (balle.y + balle.r >= barre.y && balle.x >= barre.x && balle.x <= barre.x + barre.width) { balle.sensY = -1; }
        if (balle.y - balle.r <= adverse.y + adverse.height && balle.x >= adverse.x && balle.x <= adverse.x + adverse.width) { balle.sensY = 1; }
        if (balle.y > 128 || balle.y < 0) { balleLancee = false; balle.speed = 1; }
      }

      // Dessin de la balle (uniquement sur la moitié inférieure pour le Maître, Y >= 64)
      if (balle.y >= 64) { display.fillCircle(balle.x, balle.y - 64, balle.r, SH110X_WHITE); }
      display.fillRoundRect(barre.x, barre.y - 64, barre.width, barre.height, 4, SH110X_WHITE);
      display.display();

      if(!digitalRead(BUTTON_C)) mode=MenuAcceuil;
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
  display.clearDisplay(); display.setCursor(0, 0);
  display.println("Perdue"); display.display();
  delay(250);
  if (analogRead(A2)==4095){ mode=GameSolo; vie=3; barre.reset(); balle.reset(); } 
  if(!digitalRead(BUTTON_C)){ mode=MenuAcceuil; }    
}

void pauseGame(){
  display.clearDisplay(); display.setCursor(0, 0);
  display.println("Pause"); display.display();
  delay(250);
  if (analogRead(A2)==4095){ mode=GameSolo; barre.reset(); balle.reset(); }  
}