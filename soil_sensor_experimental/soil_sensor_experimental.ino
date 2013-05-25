#include <WiFi.h>
#include <Decagon5TE.h>
#include <SD.h>
#include <SPI.h>

Decagon5TE sensor(2, 3, 15000);
const byte SD_CHIP_SELECT = 4;
const byte RTC_CHIP_SELECT = 8;

char TO_UPLOAD_FILENAME[] = "toUpload.txt";
char FAILED_UPLOAD_FILENAME[] = "fUpload.txt";
char ARCHIVE_FILENAME[] = "datalog.csv";

int status = WL_IDLE_STATUS;

char device_id[] = "vFB218A4E2233052";

WiFiClient client;

void setup(){
  Serial.begin(1200); // opens serial port, sets data rate to 1200 bps as per the 5TE's spec.
  
  RTC_init();
  SD_init();
  
  //connectToNetwork("Apartment", "tenretni");
  
  SD.remove(TO_UPLOAD_FILENAME);
  SD.remove(FAILED_UPLOAD_FILENAME);
  SD.remove(ARCHIVE_FILENAME);

  Serial.println(F("-------------------------------"));
}

void loop() {
  if (sensor.isReadyForReading()){
    //sensor.setReadingTime(ReadTimeDate());
    sensor.readData(ReadTimeDate());

    writeToSDCard(sensor.getData(), TO_UPLOAD_FILENAME);
    writeToSDCard(sensor.getData(), ARCHIVE_FILENAME);
    delay(500);

    if (connectToNetwork("Apartment", "tenretni")){
      readToUpload();
      
      //uploadData();
    }

    Serial.println(F("-------------------------------"));
  }

  Serial.println(ReadTimeDate());
  delay(1000);
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
    Serial.println(TO_UPLOAD_FILENAME);
    dataFile.close();
    return false;
  }

}

void appendFailedUploads(){
  Serial.print("Deleting ");
  Serial.println(TO_UPLOAD_FILENAME);
  SD.remove(TO_UPLOAD_FILENAME);
  
  File failed_upload_file = SD.open(FAILED_UPLOAD_FILENAME, FILE_READ);
  File to_upload_file = SD.open(TO_UPLOAD_FILENAME, FILE_WRITE);

  delay(500);

  if (to_upload_file && failed_upload_file){
    while (failed_upload_file.available()){
      to_upload_file.print((char)failed_upload_file.read());
    }
  }
  else {
    Serial.println("Failed to open the files.");
  }

  to_upload_file.close();
  failed_upload_file.close();
  SD.remove(FAILED_UPLOAD_FILENAME);
}

void readToUpload() {
  File to_upload_file = SD.open(TO_UPLOAD_FILENAME, FILE_READ);
  if (to_upload_file){
    Serial.print("Reading from ");
    Serial.println(TO_UPLOAD_FILENAME);
    while (to_upload_file.available()){
      Serial.print((char)to_upload_file.read());
    }
  }
  else {
    Serial.print("Can't open ");
    Serial.println(TO_UPLOAD_FILENAME);
  }
  to_upload_file.close();
  SD.remove(TO_UPLOAD_FILENAME);
}

double parseDouble(){
}

void uploadData(){
  File to_upload_file = SD.open(TO_UPLOAD_FILENAME, FILE_READ);
  
  delay(500);
  if (to_upload_file) {
    while (to_upload_file.available()){
      
      Serial.print("Reading from ");
      Serial.println(TO_UPLOAD_FILENAME);
      
      String timestamp = "";
      String permittivity = "";
      String conductivity = "";
      String temperature = "";
      
      while(char data = to_upload_file.read() != ',' && to_upload_file.available()){
        if (data == ',' || !to_upload_file.available()){
          break;
        }
        Serial.print((int)data);
        timestamp += data;
      }
      
      if (!to_upload_file.available()){
        Serial.println("Reached end of file.");
        break;
      }
      
      Serial.print(F("Timestamp: "));
      Serial.println(timestamp);
      
      while(char data = to_upload_file.read()){
        if (data == ','){
          break;
        }
        permittivity += data;
      }
      Serial.print(F("Permittivity: "));
      Serial.println(permittivity);
      
      while(char data = to_upload_file.read()){
        if (data == ','){
          break;
        }
        conductivity += data;
      }
  
      Serial.print(F("Conductivity: "));
      Serial.println(conductivity);
      
      while(char data = to_upload_file.read()){
        if (data == ','){
          break;
        }
      temperature += data;
      }
      Serial.print(F("Temperature: "));
      Serial.println(temperature);
  
      String data = "&permittivity=";
      data += permittivity;
      data += "&conductivity=";
      data += conductivity;
      data += "&temperature=";
      data += temperature;

      if (false){
        String failedUploadString = permittivity + "," + conductivity + "," + temperature;
        writeToSDCard(failedUploadString, FAILED_UPLOAD_FILENAME);
      }
    }
  }
  to_upload_file.close();
  
  appendFailedUploads();
}

boolean sendToPushingBox(String data) {

  Serial.println(F("I'm going to upload to the spreadsheet"));

  boolean success = false;

  Serial.println(F("Starting connection to server..."));

  if (client.connect("api.pushingbox.com", 80)) {
    Serial.println(F("connected to server"));
    // Make a HTTP request of the form:
    //client.println("GET /pushingbox?devid=vFB218A4E2233052&permittivity=1.52&conductivity=23.02 HTTP/1.1");
    client.print("GET /pushingbox?devid=");
    client.print(device_id);
    client.print(data);
    client.println(" HTTP/1.1");
    client.println("Host: api.pushingbox.com");
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

  return success;
}

boolean connectToNetwork(char ssid[], char password[]){
  status = WiFi.status();

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi shield not present"));
    // don't continue:
    while(true);
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

String doubleToString(double input){
  char input_buffer[10] = {'/0'};
  return dtostrf(input, 2, 2, input_buffer);
}

int RTC_init(){
  pinMode(RTC_CHIP_SELECT,OUTPUT); // chip select
  // start the SPI library:
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE1); // both mode 1 & 3 should work
  //set control register
  digitalWrite(RTC_CHIP_SELECT, LOW);
  SPI.transfer(0x8E);
  SPI.transfer(0x60); //60= disable Osciallator and Battery SQ wave @1hz, temp compensation, Alarms disabled
  digitalWrite(RTC_CHIP_SELECT, HIGH);
  delay(10);
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

String ReadTimeDate(){
  SPI.setDataMode(SPI_MODE1);
  String temp;
  int TimeDate [7]; //second,minute,hour,null,day,month,year
  for(int i=0; i<=6;i++){
    if(i==3)
    i++;
    digitalWrite(RTC_CHIP_SELECT, LOW);
    SPI.transfer(i+0x00);
    unsigned int n = SPI.transfer(0x00);
    digitalWrite(RTC_CHIP_SELECT, HIGH);
    int a=n & B00001111;
    if(i==2){
      int b=(n & B00110000)>>4; //24 hour mode
      if(b==B00000010)
        b=20;
      else if(b==B00000001)
        b=10;
      TimeDate[i]=a+b;
    }
    else if(i==4){
      int b=(n & B00110000)>>4;
      TimeDate[i]=a+b*10;
    }
    else if(i==5){
      int b=(n & B00010000)>>4;
      TimeDate[i]=a+b*10;
    }
    else if(i==6){
      int b=(n & B11110000)>>4;
      TimeDate[i]=a+b*10;
    }
    else{
      int b=(n & B01110000)>>4;
      TimeDate[i]=a+b*10;
    }
  }
  temp.concat(TimeDate[4]);
  temp.concat("/") ;
  temp.concat(TimeDate[5]);
  temp.concat("/") ;
  temp.concat(TimeDate[6]);
  temp.concat(" ") ;
  temp.concat(TimeDate[2]);
  temp.concat(":") ;
  temp.concat(TimeDate[1]);
  temp.concat(":") ;
  temp.concat(TimeDate[0]);
  return(temp);
}
