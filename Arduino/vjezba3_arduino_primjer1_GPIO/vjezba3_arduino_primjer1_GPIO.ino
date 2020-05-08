// program koji cita stanje tipkala spojenog na PA0 nožicu i
// prikazuje stanje tipkala na svijetlecoj diodi spojenoj na PB1

#define BUTTON PA0
#define LED PB1

int value = 0;

// funckija setup() izvrsava se samo jednom na pocetku programa 
void setup() {
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
}

// funkcija loop() izvrsava se ciklicki 
void loop() {
  value = digitalRead(BUTTON);
  digitalWrite(LED, value);
}
