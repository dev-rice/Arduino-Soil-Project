const int EXCITATION_PIN = 2;
const int SERIAL_CONTROL_PIN = 3;

const double POLLING_PERIOD = 5; // In seconds, should be greater than 3

char incomingByte = 0;   // for incoming serial data
long last_update_millis = 0;

boolean due_for_update = true;

char data[20] = {0};
int data_index = 0;

double dielectric_permittivity = 0;
double electrical_conductivity = 0;
double temperature = 0;

void setup() {
  pinMode(SERIAL_CONTROL_PIN, OUTPUT);
  pinMode(EXCITATION_PIN, OUTPUT);
  
  digitalWrite(EXCITATION_PIN, HIGH);
  digitalWrite(0, LOW);
  
  Serial.begin(1200);     // opens serial port, sets data rate to 9600 bps
  
  cycleSerialLine();
  
  last_update_millis = millis();
}

void loop() {
  // Check for received data only when it's time
  if ((millis() - last_update_millis) >= (1000 * POLLING_PERIOD) && due_for_update == true){
    last_update_millis = millis();
    due_for_update = false;
    
    exciteSensor();  
    
    while (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();
      data[data_index] = incomingByte;
      ++data_index;
      //Serial.print(incomingByte);
    }
    
    dielectric_permittivity = calculateDielectricPermittivity(data);
    electrical_conductivity = calculateElectricalConductivity(data);
    temperature = calculateTemperature(data);

    Serial.print("\tDielectric Permittivity: ");
    Serial.println(dielectric_permittivity);
    
    Serial.print("\tElectrical Conductivity: ");
    Serial.print(electrical_conductivity);
    Serial.println(" dS/m");
    
    Serial.print("\tTemperature: ");
    Serial.print(temperature);
    Serial.println(" C");
    
    Serial.println("Done reading.");
    Serial.println("---------------");
    
    cycleSerialLine();
    
    data_index = 0;
  }
}

void cycleSerialLine(){
  digitalWrite(SERIAL_CONTROL_PIN, HIGH);
  delay(2500);
  digitalWrite(SERIAL_CONTROL_PIN, LOW);
  Serial.println("Cycled Serial Line.");
  due_for_update = true;
}

void exciteSensor(){
  digitalWrite(EXCITATION_PIN, LOW);
  delay(200);
  digitalWrite(EXCITATION_PIN, HIGH);
}

double calculateDielectricPermittivity(char data[]) {
  char permittivity[4] = {'/0'};
  int perm_index = 0;
  int index = 0;
  
  int raw_dielectric = 0;
  
  while (data[index] != ' '){
    ++index;
  }
  ++index;
  
  while (data[index] != ' '){
    permittivity[perm_index] = data[index];
    ++perm_index;
    ++index;
  }
  
  raw_dielectric = atoi(permittivity);
  
  if (raw_dielectric != 4095){
    return (double)raw_dielectric / 50.0;
  }
  else {
    return 4095;
  }
}

double calculateElectricalConductivity(char data[]) {
  char conductivity[4] = {'/0'};
  int cond_index = 0;
  int index = 0;
  
  int raw_conductivity = 0;
  
  for (int i = 0; i < 2; ++i){
    while (data[index] != ' '){
      ++index;
    }
    ++index;
  }
  
  while (data[index] != ' '){
    conductivity[cond_index] = data[index];
    ++cond_index;
    ++index;
  }
  
  raw_conductivity = atoi(conductivity);
  
  if (raw_conductivity <= 700){
    return (double)raw_conductivity / 100.0;
  }
  else if (raw_conductivity != 1023) {
    return (double)(700 + 5 * (raw_conductivity - 700)) / 100;
  }
  else {
    return 1023;
  }
}

double calculateTemperature(char data[]) {
  char temperature[4] = {'/0'};
  int temp_index = 0;
  int index = 0;
  
  int raw_temperature = 0;
  
  for (int i = 0; i < 3; ++i){
    while (data[index] != ' '){
      ++index;
    }
    ++index;
  }
  
  while (data[index] != 'z'){
    temperature[temp_index] = data[index];
    ++temp_index;
    ++index;
  }
  
  raw_temperature = atoi(temperature);
  
  if (raw_temperature > 900 && raw_temperature != 1023){
    raw_temperature = 900.0 + 5.0 * (raw_temperature - 900.0);
  }
  else if (raw_temperature == 1023) {
    return 1023;
  }
  
  double celsius = (double)(raw_temperature - 400) / 10.0;
  return celsius;
}
