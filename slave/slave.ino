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
  MenuAcceuil, // Attention à l'orthographe pour correspondre au reste
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

Barre barre;       
Barre adverse;     
Balle balle;

// --- 1. DÉCLARATION DES MENUS (Avant l'objet Menu) ---
MenuItem menuPrincipal[] = {
    {"MULTI", GameMulti},
    {"SOLO", GameSolo},
    {"PARAMETRES", Parametres}
};

MenuItem menuPause[] = {
    {"REPRENDRE", GameSolo}, // Modifié en GameSolo ou GameMulti selon votre choix
    {"RETOUR MENU", MenuAcceuil}
};

MenuItem menuPerduSolo[] = {
    {"REJOUER", GameSolo},
    {"RETOUR MENU", MenuAcceuil}
};

// --- 2. INSTANCE UNIQUE DU MENU ---
// On l'initialise par défaut avec le menu principal
Menu menu(menuPrincipal);

bool joystickAppuyer = false;
int tDep = 0;
int vie = 3;
int score = 0;

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
      menu.drawMenu(display); // On utilise l'objet générique 'menu'
      
      if (menu.handleJoystick()) { // Pas d'argument ici !
        // On récupère directement la valeur (GameMulti, GameSolo, etc.)
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
      if (vie == 0) {
        menu.setItems(menuPerduSolo); // 🔄 On bascule le menu sur l'écran de défaite
        mode = GameOver;
      }
      else {
        if (balle.perdu()){ vie--; balle.reset(); barre.reset(); }
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
          if (analogRead(A2) >= 4090) { // Changé à 4090 pour matcher votre menu.cpp
            if (!joystickAppuyer) { joystickAppuyer = true; tDep = millis(); } 
            else if (millis() - tDep >= 1000) { 
              joystickAppuyer = false; 
              menu.setItems(menuPause); // 🔄 On charge le menu de pause
              mode = PauseGame; 
            }
          } else { joystickAppuyer = false; }

          // Détection du rebond sur la barre joueur
          if (balle.toucheBarre(barre)) {
            score++;
          }

          display.setCursor(0,0);
          for (int i=0; i<vie; i++){ display.write(0x03); }

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
  // Utilisation de votre classe Menu au lieu du texte fixe d'avant !
  menu.drawMenu(display);
  
  if (menu.handleJoystick()) {
    int action = menu.getSelectedValue();
    if (action == GameSolo || action == GameMulti) { // Bouton Rejouer
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
  // Utilisation de votre classe Menu pour la pause !
  menu.drawMenu(display);
  
  if (menu.handleJoystick()) {
    int action = menu.getSelectedValue();
    if (action == GameSolo || action == GameMulti) { // Bouton Reprendre
      mode = GameSolo; // Reprend la partie en cours
    } else { // Bouton Retour Menu
      menu.setItems(menuPrincipal);
      mode = MenuAcceuil;
    }
  }
}