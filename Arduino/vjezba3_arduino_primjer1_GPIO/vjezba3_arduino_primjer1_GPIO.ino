/* Program  cita stanje tipkala spojenog na nozicu PA0 i 
 * prikazuje stanje tipkala na svijetlecoj diodi spojenoj na PB1.
 */
// umjesto naziva PA0 i PB1 zelimo koristiti nazive BUTTON i LED
#define BUTTON PA0
#define LED PB1

// varijabla za pohranu stanja tipkala
int value = 0;

// funkcija setup() izvrsava se samo jednom na pocetku programa 
void setup() {
  // inicijalizacija BUTTON nozice (PA0) kao ulazne na koju je spojeno tipkalo
  pinMode(BUTTON, INPUT_PULLUP);
  // inicijalizacija LED nozice (PB1) kao izlazne
  pinMode(LED, OUTPUT);
}

// funkcija loop() izvrsava se ciklicki 
void loop() {
  // citanje stanja tipkala (LOW ili HIGH) i ispis na LED
  value = digitalRead(BUTTON);
  digitalWrite(LED, value);
}
