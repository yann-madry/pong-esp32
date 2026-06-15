#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WiFi.h>
#include <esp_now.h>
#include "FeatherShieldPinouts.h"
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
Menu menu;
Balle balle;

bool joystickAppuyer=false;
int tDep=0; //durée d'appui sur le joystick
int vie=3;

///**************Multi****************//////
uint8_t adresseEsclave[] = {0xC4, 0x4F, 0x33, 0x64, 0x8D, 0x11};

struct MessageEsclave{ //struct = class mais avec tout en public par défaut
    int pisition_joystick_esclave;
};

struct MessageMaitre{
    int raquetteX_J1; // barre maitre en X
    int raquetteX_J2; // barre esclcave en X
    int balleX;
    int balleY;
};

MessageEsclave messE; // contient position joystick thomas
MessageMaitre messM; // contient les coordonées de ma raquette de la raquette de thomas et de la balle

Barre barreJ1; // Barre maitre
Barre barreJ2; // barre esclave

//on recoit les info du x de l'esclave
void esclaveDonneX( const uint8_t *mac, const uint8_t *temp, int len){
    memcpy(&messE, temp,sizeof(messE));
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

    WiFi.mode(WIFI_STA);
    if (esp_now_init() == ESP_OK) {
        esp_now_register_recv_cb(esp_now_recv_cb_t(esclaveDonneX));
        esp_now_peer_info_t peerInfo;
        memcpy(peerInfo.peer_addr, adresseEsclave, 6); 
        peerInfo.channel = 10;
        peerInfo.encrypt = false; 
        esp_now_add_peer(&peerInfo);
    }
    barreJ2.y = 0; // Force la raquette de Thomas tout en haut
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
      if (vie==0) mode = GameOver;
      if (balle.perdu()){
        vie--;
        balle.reset();
        barre.reset();
      }
      else {
        barre.deplacement(analogRead(A3));
        balle.deplacement(barre);
        if(!digitalRead(BUTTON_C)){
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
          else {
            if (millis()-tDep>=1000) { 
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
      // ======= LOGIQUE DU MAÎTRE MULTIJOUEUR EN RESEAU =======
      barreJ1.deplacement(analogRead(A2)); // Ton joystick
      barreJ2.deplacement(messE.pisition_joystick_esclave); // Le joystick de Thomas via radio

      balle.deplacement(barreJ1); // Mouvement de la balle

      // Remplissage du paquet de synchronisation
      messM.raquetteX_J1 = barreJ1.x;
      messM.raquetteX_J2 = barreJ2.x;
      messM.balleX = balle.x;
      messM.balleY = balle.y;

      // Envoi à l'esclave
      esp_now_send(adresseEsclave, (uint8_t *) &messM, sizeof(messM));

      // Affichage local sur ton écran
      display.clearDisplay();
      barreJ1.afficher(display);
      barreJ2.afficher(display);
      balle.afficher(display);
      display.display();

      if(!digitalRead(BUTTON_C)){
        barreJ1.reset();
        barreJ2.reset();
        balle.reset();
        menu.reset();
        mode=MenuAcceuil;
      }
      break;

    case GameOver:
      display.clearDisplay();
      vie=3;
      display.setCursor(0, 0);
      display.println("Perdue");
      display.println("Cliquer sur le joystick pour rejouer");
      display.print("Ou C pour retourner au Menu");
      display.display();

      delay(250);
      if (analogRead(A2)==4095){
        mode=GameSolo;
        barre.reset();
        balle.reset();
      } 
      if(!digitalRead(BUTTON_C)){
        mode=MenuAcceuil;
        barre.reset();
        balle.reset();
      }   
      break;

    case PauseGame:
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
      break;
  }
}