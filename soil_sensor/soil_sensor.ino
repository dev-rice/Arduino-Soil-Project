#include <WiFi.h>
#include <Decagon5TE.h>

char ssid[] = "Apartment";
char pass[] = "tenretni";
int status = WL_IDLE_STATUS;

char device_id[] = "vFB218A4E2233052";

long last_update_millis = 0;

WiFiClient client;

Decagon5TE sensor(2,3);

void setup() {
  
  Serial.begin(1200);     // opens serial port, sets data rate to 1200 bps as per the 5TE's spec.
  
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
  
  last_update_millis = millis();
}

void loop() {
  sensor.readData();  
  
  double dielectric_permittivity = sensor.getDielectricPermittivity();
  double electrical_conductivity = sensor.getElectricalConductivity();
  double temperature = sensor.getTemperature();
  
  printData(dielectric_permittivity, electrical_conductivity, temperature);
    
  if (sendToPushingBox(dielectric_permittivity, electrical_conductivity, temperature)){
    Serial.println("Upload successful");
  }
  else {
    Serial.println("Upload failed.");
  } 
  
  delay(10000);
}

boolean sendToPushingBox(double permittivity, double conductivity, double temperature){
  boolean success = false;

  char permittivity_buffer[6] = {'/0'};
  char conductivity_buffer[6] = {'/0'};
  char temperature_buffer[7] = {'/0'};
  
  String data = "&permittivity=";
  data += dtostrf(permittivity,2,2,permittivity_buffer);
  data += "&conductivity=";
  data += dtostrf(conductivity,2,2,conductivity_buffer);
  data += "&temperature=";
  data += dtostrf(temperature,2,2,temperature_buffer);
  
  char data_buffer[data.length()];  
  data.toCharArray(data_buffer, data.length()+1);
  
  //Serial.println(data_buffer);
  
  delay(500);
  
  Serial.println("\nStarting connection to server...");
  
  if (client.connect("api.pushingbox.com", 80)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    //client.println("GET /pushingbox?devid=vFB218A4E2233052&permittivity=1.52&conductivity=23.02 HTTP/1.1");
    client.print("GET /pushingbox?devid=");
    client.print(device_id);
    client.print(data);
    client.println(" HTTP/1.1");
    client.println("Host: api.pushingbox.com");
    client.println("Connection: close");
    client.println();
    
    success = true;
  }
  else {
    success = false;
  }
  
  client.flush();
  client.stop();
  
  return success; 
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

