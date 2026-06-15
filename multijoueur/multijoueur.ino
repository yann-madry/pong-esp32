#include "FeatherShieldPinouts.h"

//connexion inter cartes
#include <WiFi.h>
#include <esp_now.h> // mode sans connexion

//ecran
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// adresse MAC de l'ESP de Thomas :
uint8_t adresseEsclave[] = {0xC4, 0x4F, 0x33, 0x64, 0x8D, 0x11}; //pas int car taille prise dans la mémoire pas adaptée

struct MessageEsclave{ //struct = class mais avec tout en public par défaut
    int raquetteX_J2;
};

struct MessageMaitre{
    int raquetteX_J1;
    int raquetteX_J2;
    int balleX;
    int balleY;
};

MessageEsclave messE;
MessageMaitre messM;

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

//on recoit les info du x de l'esclave
void esclaveDonneX( const uint8_t *mac, const uint8_t *temp, int len){
    memcpy(&messE, temp,sizeof(messE));
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    if(esp_now_init() != ESP_OK){
        return;
    } else{
        esp_now_register_recv_cb(esp_now_recv_cb_t(esclaveDonneX));
        esp_now_peer_info_t peerInfo;
        memcpy(peerInfo.peer_addr,adresseEsclave,6); //6 car adresse MAC sur 6 octets
        peerInfo.channel = 0;
    }
}

void loop() {
    messM.raquetteX_J1 = analogRead(A2); // comportement du joystick
    esp_now_send(adresseEsclave, (uint8_t *) &messE, sizeof(messE));
    delay(16);
}

























