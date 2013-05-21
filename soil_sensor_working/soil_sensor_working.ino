#include <WiFi.h>
#include <Decagon5TE.h>
#include <SD.h>
#include <avr/sleep.h>

Decagon5TE sensor(2, 3, 10000);
const int chipSelect = 4;

const char FILENAME[] = "5TE_Data.txt";

char ssid[] = "Apartment";
char pass[] = "tenretni";
int status = WL_IDLE_STATUS;

char device_id[] = "vFB218A4E2233052";

WiFiClient client;

void setup(){
  Serial.begin(1200); // opens serial port, sets data rate to 1200 bps as per the 5TE's spec.
  
  Serial.println("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  
  SD.remove("datalog.txt");
  
  connectToNetwork(ssid,pass);
  
  Serial.println("-------------------------------");
}

void loop(){
  if (sensor.isReadyForReading()){
    sensor.readData();
    writeToSDCard(sensor);
   
    if (connectToNetwork(ssid, pass)){
      sendToPushingBox(sensor);
    }
    
    Serial.println("-------------------------------");
  }
}

boolean writeToSDCard(Decagon5TE sensor_data){
  //last_sd_time = millis();
 
  Serial.println("I'm going to write to the SD card");
  
  String dataString = "";
  
  dataString += doubleToString(sensor_data.getDielectricPermittivity());
  dataString += " ";
  dataString += doubleToString(sensor_data.getElectricalConductivity());
  dataString += " ";
  dataString += doubleToString(sensor_data.getTemperature());

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    Serial.println(dataString);
    dataFile.close();
    return true;
    // print to the serial port too:
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
    dataFile.close();
    return false;
  }  
}

boolean sendToPushingBox(Decagon5TE sensor_data){
  
  Serial.println("I'm going to upload to the spreadsheet");
  
  boolean success = false;
  
  String data = "&permittivity=";
  data += doubleToString(sensor_data.getDielectricPermittivity());
  data += "&conductivity=";
  data += doubleToString(sensor_data.getElectricalConductivity());
  data += "&temperature=";
  data += doubleToString(sensor_data.getTemperature());
  
  //char data_buffer[data.length()];  
  
  Serial.println("Starting connection to server...");
  
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
    Serial.println("Upload Successful");
  }
  else {
    success = false;  
    Serial.println("Upload Failed");
  }
  
  client.flush();
  client.stop();
  
  //last_upload_time = millis();
  
  return success; 
}

boolean connectToNetwork(char ssid[], char password[]){
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // don't continue:
    while(true);
  } 
  int failures = 0;
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED && failures < 2) {
  //Serial.println(failures); 
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
  // Connect to WPA/WPA2 network:    
    status = WiFi.begin(ssid, pass);
    ++failures;
  }
  if (status) {
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
  }
  return status;
}

String doubleToString(double input){
  char input_buffer[10] = {'/0'};
  return dtostrf(input, 2, 2, input_buffer);  
}

