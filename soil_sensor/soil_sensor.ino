#include <Decagon5TE.h>
#include <Reading.h>
#include <WiFi.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>
#include <RTC_DS3234.h>

const byte SD_CHIP_SELECT = 4;

Decagon5TE sensor(34,35);

RTC_DS3234 RTC(8);

WiFiClient client;

byte status = WL_IDLE_STATUS;
char ssid[] = "Apartment";
char password[] = "tenretni";

char device_id[] = "v38EB7DD137364CD";

char to_upload_filename[] = "TOUPLOAD.txt";
char datalog_filename[] = "DATALOG.csv";
char failed_upload_filename[] = "FAILUPLOAD.txt";

void setup(){
  Serial.begin(1200);
  
  SPI.begin();
  SD_init();
  
  connectToNetwork(ssid, password);
  
  RTC.begin();
  
  pinMode(53, OUTPUT);
  //RTC.adjust(DateTime(__DATE__, __TIME__));
  
  //SD.remove("datalog.csv");
  //SD.remove("fupload.csv");
  
  static char buf[32];
  Serial.print("Time: ");
  Serial.println(RTC.now().toString(buf,32));
  
  Serial.println("Finished Setup");
}

void loop(){
  if (sensor.isReadyForReading()){
    Serial.println("Reading data from sensor...");
    static char buf[32];
    //sensor.readData(RTC.now().toString(buf,32));
    //Serial.println(sensor.getData());
    
    //writeToSDCard(sensor.getUploadData(), to_upload_filename);
    //writeToSDCard(sensor.getData(), datalog_filename);
    
  }
  else if (connectToNetwork(ssid, password)){
    uploadData();
  }
  
  delay(1000);
}

void uploadData() {
  SPI.setDataMode(SPI_MODE0);
    
    File data_file = SD.open(to_upload_filename, FILE_READ);
    String to_upload = "";
    
    for (int i = 0; data_file.available(); ++i){
      to_upload = readFromSDCard(data_file);
      Serial.print("Entry ");
      Serial.print(i);
      Serial.println(":");
      
      Serial.print('\t');
      Serial.println(to_upload);
      
      int rand = random(5);
      Serial.println(rand);
      if (!sendToPushingBox(to_upload)){// || rand != 3){
        writeToSDCard(to_upload, failed_upload_filename);
      }
    }
    
    data_file.close();
    appendFailedUploads();
    Serial.println("-----------------------");
}

void appendFailedUploads() {
  SD.remove(to_upload_filename);
  
  File failed_file = SD.open(failed_upload_filename, FILE_READ);
  File to_upload_file = SD.open(to_upload_filename, FILE_WRITE);
  
  if (failed_file && to_upload_file){
    while (failed_file.available()){
      to_upload_file.print((char)failed_file.read());
    }
  }   
  
  failed_file.close();
  to_upload_file.close();
  
  SD.remove(failed_upload_filename);
}
String readFromSDCard(File& read_file){
  String data_string = "";
  char data = '/0';
  while (read_file.available()){
    data = read_file.read();
    if ((int)data == 10){
      break;
    }
    else if ((int)data != 13){
      data_string += data;
    }
  }

  return data_string;
}

boolean writeToSDCard(String dataString, char filename[]){
  SPI.setDataMode(SPI_MODE1);
  
  Serial.print(F("I'm going to write to "));
  Serial.println(filename);

  SPI.setDataMode(SPI_MODE0);
  // open the file. note that only one file can be open at a time, 
  // so you have to close this one before opening another.
  File dataFile = SD.open(filename, FILE_WRITE);

  delay(500);

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
    Serial.print(F("error opening "));
    Serial.println(filename);
    dataFile.close();
    return false;
  }
}

boolean sendToPushingBox(String data) {
  
  Serial.println(F("I'm going to upload to the spreadsheet"));

  boolean success = false;

  Serial.println(F("Starting connection to server..."));

  if (client.connect("api.pushingbox.com", 80)) {
    Serial.println(data);
    //client.print("GET /pushingbox?devid=v38EB7DD137364CD");
    client.print("GET /pushingbox?devid=");
    client.print(device_id);
    client.print(data);
    //client.print("&t=100&p=29.48&c=1.38&tp=22.20");
    client.println(" HTTP/1.1");
    client.println("Host:api.pushingbox.com");
    client.println("Connection: close");
    client.println();

    success = true;
    Serial.println(F("Upload Successful"));
  }
  else {
    success = false;
    Serial.println(F("Upload Failed"));
  }

  client.flush();
  client.stop();

  delay(2500);

  return success;
}

boolean SD_init(){
  SPI.setDataMode(SPI_MODE0);
  Serial.println(F("Initializing SD card..."));
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(SD_CHIP_SELECT)) {
    Serial.println(F("Card failed, or not present"));
    // don't do anything more:
    return false;
  }
  Serial.println(F("card initialized."));
  return true;
}

boolean connectToNetwork(char ssid[], char password[]){
  status = WiFi.status();

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi shield not present"));
    // don't continue:
    return false;
  }
  int failures = 0;
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED && failures < 2) {
    //Serial.println(failures);
    Serial.print(F("Attempting to connect to WPA SSID: "));
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, password);
    ++failures;
  }
  if (status) {
    Serial.print(F("Connected to "));
    Serial.println(WiFi.SSID());
  }
  return status;
}

