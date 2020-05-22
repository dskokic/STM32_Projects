/* 
 *  Prosirenje programa za treptalo s promjenjivim trajanjem pauze
 * iz primjera 3 s sipisom trajanja pauze na serijski port racunala
 */

#define POT PA0
#define LED PB1

int pot;

void setup() {
  // inicijalizacija LED nozice (PB1) kao izlazne
  pinMode(LED, OUTPUT);
  // inicijalizacija UART-a (Serial1) na brzinu pijenosa 115200 bita u sekundi
  Serial.begin(115200);
}

void loop() {
  pot = analogRead(POT);
  // ispis na serijski port racunala
  Serial.println(pot);
  
  digitalWrite(LED, HIGH);
  delay(pot);
  
  digitalWrite(LED, LOW);
  delay(pot);
}
