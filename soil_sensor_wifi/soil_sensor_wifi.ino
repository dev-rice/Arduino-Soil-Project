#include <WiFi.h>
#include <Twitter.h>

char ssid[] = "Apartment";
char pass[] = "tenretni";
int status = WL_IDLE_STATUS;

const int EXCITATION_PIN = 2;
const int SERIAL_CONTROL_PIN = 3;

const double POLLING_PERIOD = 25; // In seconds, should be greater than 3

char incomingByte = 0;   // for incoming serial data
long last_update_millis = 0;

boolean ready_for_update = true;

double dielectric_permittivity = 0;
double electrical_conductivity = 0;
double temperature = 0;

WiFiClient client;

Twitter twitter("1431845773-dI6Yac0a9lspUKJak33cQ8ojYCtasQwOyrKCGeb");

void setup() {
  pinMode(SERIAL_CONTROL_PIN, OUTPUT);
  pinMode(EXCITATION_PIN, OUTPUT);
  
  digitalWrite(EXCITATION_PIN, HIGH);
  digitalWrite(0, LOW);
  
  Serial.begin(1200);     // opens serial port, sets data rate to 9600 bps
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // don't continue:
    while(true);
  } 
  
 // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:    
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
   
  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  printCurrentNet();
  printWifiData();
  
  cycleSerialLine();
  
  last_update_millis = millis();
}

void loop() {
  // Check for received data only when it's time
  if ((millis() - last_update_millis) >= (1000 * POLLING_PERIOD) && ready_for_update == true){
    last_update_millis = millis();
    ready_for_update = false;
    
    exciteSensor();  
        
    char data[20] = {0};
    int data_index = 0;
    
    while (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();
      data[data_index] = incomingByte;
      ++data_index;
    }
    
    dielectric_permittivity = calculateDielectricPermittivity(data);
    electrical_conductivity = calculateElectricalConductivity(data);
    temperature = calculateTemperature(data);

    tweetData(dielectric_permittivity, electrical_conductivity, temperature);
    printData(dielectric_permittivity, electrical_conductivity, temperature);
    
    Serial.println("Done reading.");
    Serial.println("---------------");
    
    cycleSerialLine();
  }
  
}

void tweetData(double permittivity, double conductivity, double temp){
  String tweet = "Permittivity: ";
  tweet += (int)permittivity;
  tweet += ", Conductivity: ";
  tweet += (int)conductivity;
  tweet += ", Temp: ";
  tweet += (int)temp;
  char charBuf[140];
  tweet.toCharArray(charBuf, 140);
  Serial.println("tweeting ...");
  if (twitter.post(charBuf)) {
    int status = twitter.wait(&Serial);
    if (status == 200) {
      Serial.println("OK.");
    } else {
      Serial.print("failed : code ");
      Serial.println(status);
    }
  } else {
    Serial.println("connection failed.");
  }
}

void printData(double permittivity, double conductivity, double temp){
  Serial.print("\tDielectric Permittivity: ");
  Serial.println(permittivity);
    
  Serial.print("\tElectrical Conductivity: ");
  Serial.print(conductivity);
  Serial.println(" dS/m");
    
  Serial.print("\tTemperature: ");
  Serial.print(temp);
  Serial.println(" C");  
}

//---------------------
// 5TE Stuff
//---------------------

void cycleSerialLine(){
  digitalWrite(SERIAL_CONTROL_PIN, HIGH);
  delay(2500);
  digitalWrite(SERIAL_CONTROL_PIN, LOW);
  Serial.println("Cycled Serial Line.");
  ready_for_update = true;
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

//---------------------
// Wifi Stuff
//---------------------

void printWifiData() {
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);
  
  // print your MAC address:
  byte mac[6];  
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  Serial.print(mac[5],HEX);
  Serial.print(":");
  Serial.print(mac[4],HEX);
  Serial.print(":");
  Serial.print(mac[3],HEX);
  Serial.print(":");
  Serial.print(mac[2],HEX);
  Serial.print(":");
  Serial.print(mac[1],HEX);
  Serial.print(":");
  Serial.println(mac[0],HEX);
 
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);    
  Serial.print("BSSID: ");
  Serial.print(bssid[5],HEX);
  Serial.print(":");
  Serial.print(bssid[4],HEX);
  Serial.print(":");
  Serial.print(bssid[3],HEX);
  Serial.print(":");
  Serial.print(bssid[2],HEX);
  Serial.print(":");
  Serial.print(bssid[1],HEX);
  Serial.print(":");
  Serial.println(bssid[0],HEX);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption,HEX);
  Serial.println();
}

