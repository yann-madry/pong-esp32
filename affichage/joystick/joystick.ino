void setup()
{
    Serial.begin(9600);
}

void loop()
{
    int sensorValue1 = analogRead(A2);
    int sensorValue2 = analogRead(A3);

    Serial.print("Position joystick: ");
    if (sensorValue1<1000){
      Serial.print("Haut");
    }
    if (sensorValue1>2000){
      Serial.print("Bas");
    }
    Serial.print(" ; ");
    if (sensorValue2<1000){
      Serial.print("Gauche");
    }
    if (sensorValue2>2000){
      Serial.print("Droite");
    }
    Serial.println(" ");
    delay(500);
}