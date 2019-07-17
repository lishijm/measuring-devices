#include<Arduino.h>

int echo=2;
int trig=3;

void step(){
    Serial.begin(9600);
    pinMode(echo,OUTPUT);
    pinMode(echo,OUTPUT);
    pinMode(trig,OUTPUT);
}
void loop(){
    digitalWrite(trig,LOW);
    delayMicroseconds(2);
    digitalWrite(trig,HIGH);

    delayMicroseconds(10);
    digitalWrite(trig,LOW);
    int distance=pulseIn(echo,HIGH);
    distance=distance/58;
    Serial.println(distance);
    delay(50);
}
