/* Program svakih pola sekunde mijenja stanje na 
 * svijetlecoj diodi (LED) koja je spojenan na nozicu PB1.
 */

// umjesto naziva PB1 zelimo koristiti LED
#define LED PB1


// funkcija setup() izvrsava se samo jednom na pocetku programa  
void setup() {
  // inicijalizacija LED nozice (PB1) kao izlazne
  pinMode(LED, OUTPUT);
}

// funkcija loop() izvrsava se ciklicki 
void loop() {
  // upis HIGH ('1') na LED
  digitalWrite(LED, HIGH);
  // pauza 500 ms
  delay(500);
  // upis LOW ('0') na LED
  digitalWrite(LED, LOW);
  // pauza 500 ms
  delay(500);
}
