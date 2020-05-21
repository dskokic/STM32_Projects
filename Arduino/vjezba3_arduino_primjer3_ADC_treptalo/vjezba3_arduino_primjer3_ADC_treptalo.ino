/* Program mijenja frekvenciju treptanja svijetlece diodi (LED) 
 * koja je spojena na nozicu PB1, ovisno o kutu zakreta potenciometra
 * (POT) otpora 10 KOhma čiji je klizac  spojen na PA0 nozicu mikrokontrolera.
 * Nozicu PA0 (A0) konfigurirati kao analogni ulaz
 * Napomena: funkcija analogRead() za citanje vrijednosti napona s potenciometra
 *           daje vrijednosti u rasponu [0, 1023]. Tu vrijednost uzeti kao 
 *           vrijeme trajanje pauze pri promjeni stanja na LED. 
 *           Radi se o dakle o pretvorbi napona u vrijeme (trajanje pauze)
 */

#define POT PA0
#define LED PB1

int pot;

void setup() {
  // inicijalizacija LED nozice (PB1) kao izlazne
  pinMode(LED, OUTPUT);
}

void loop() {
  pot = analogRead(POT);
  
  digitalWrite(LED, HIGH);
  delay(pot);
  
  digitalWrite(LED, LOW);
  delay(pot);
}
