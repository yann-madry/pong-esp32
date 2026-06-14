#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire, -1);
const int JOYSTICK_PIN = A2, NUM_ITEMS = 3, r = 4;
const char* menuItems[NUM_ITEMS] = {"MULTI", "SOLO", "PARAMETRES"};
int currentIndex = 0;
unsigned long lastMoveTime = 0;

void setup() {
  Serial.begin(115200);
  if (!display.begin(0x3C, true)) while (1);
  display.setRotation(1);
}

void loop() {
  handleJoystick();
  drawMenu();
  delay(10); 
}

void handleJoystick() {
  int val = analogRead(JOYSTICK_PIN);
  if (millis() - lastMoveTime > 250) {
    if (val < 1000 && currentIndex > 0) { currentIndex--; lastMoveTime = millis(); }
    else if (val > 2000 && val < 4000 && currentIndex < NUM_ITEMS - 1) { currentIndex++; lastMoveTime = millis(); }
    else if (val == 4095) { executeAction(currentIndex); lastMoveTime = millis(); }
  }
}

void drawMenu() {
  display.clearDisplay();
  display.setTextSize(1);

  // 1. Calcul et dessin unique du rectangle de surbrillance
  int hY = (currentIndex == 0) ? 2 : (currentIndex == 1) ? 20 : 41;
  display.fillRoundRect(2, hY, 123, (currentIndex == 1) ? 24 : 20, r, SH110X_WHITE);

  // 2. Boucle unique pour afficher TOUS les éléments du menu
  for (int i = 0; i < NUM_ITEMS; i++) {
    bool isSel = (currentIndex == i);
    display.setTextColor(isSel ? SH110X_BLACK : SH110X_WHITE);
    
    // Calcul mathématique de la position du texte Y (Haut: 8, Centre: 28, Bas: 48)
    display.setCursor(8, 8 + (i * 20));

    // Gestion des icônes/flèches
    if (isSel) display.write(0x10);                  // Icône de sélection (►)
    else if (i == 0 && currentIndex > 0) display.write(0x18); // Flèche Haut
    else if (i == 2 && currentIndex < 2) display.write(0x19); // Flèche Bas
    else display.print(" ");
    
    display.print("  "); display.print(menuItems[i]);
  }

  display.drawRoundRect(0, 0, 127, 63, r, SH110X_WHITE);
  display.display();
}

void executeAction(int index) {
  Serial.printf("Action : %s\n", menuItems[index]);
  for(int i = 0; i < 2; i++) {
    display.invertDisplay(true); delay(80);
    display.invertDisplay(false); delay(80);
  }
}