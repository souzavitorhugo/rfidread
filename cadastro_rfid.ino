/*
 * Desenvolvido por Vitor Hugo de Souza Antonio
 * github: https://github.com/souzavitorhugo
 * -- Código para o leitor de cadastro --
 * Sistema de cadastro e controle de alimentação 
 * de criação de animais via RFID.
  */
/**--------------------------------------------------------
  Hardware: WeMos D1 ESP8266
  Framework: Arduino IDE 1.8.19
  Curso: Engenharia de Computação
  Disciplina: Comunicação de dados
  -------------------------------------------------------- */
/*  Relação entre pinos da WeMos D1 R2 e GPIOs do ESP8266
  Pinos-WeMos   Função      Pino-ESP-8266
    TX      TXD             TXD/GPIO1
    RX      RXD             RXD/GPIO3
    D0      IO              GPIO16
    D1      IO, SCL         GPIO5
    D2      IO, SDA         GPIO4
    D3      IO, 10k PU      GPIO0
    D4      IO, 10k PU,     LED GPIO2   //Utilizado na gravação. Sug: Usar como INPUT.
    D5      IO, SCK         GPIO14    //Utilizado na gravação. Sug: Usar como INPUT.
    D6      IO, MISO        GPIO12
    D7      IO, MOSI        GPIO13
    D8      IO, 10k PD, SS  GPIO15
    A0      Inp. AN 3,3Vmax A0
*/
//Libraries
#include <SPI.h>//https://www.arduino.cc/en/reference/SPI
#include <ESP8266WiFi.h>
#include <MFRC522.h>//https://github.com/miguelbalboa/rfid
#include <ESP8266HTTPClient.h>

#define SS_PIN D8
#define RST_PIN D3
//#define SERVER_IP "http://devcontroleveterinario.azurewebsites.net/api/RFID/InserirNovoRFID"

//API QUE INICIA ALIMENTACAO
//#define SERVER_IP "http://devcontroleveterinario.azurewebsites.net/api/Alimentacao/FoiAlimentar"

//API QUE PARA ALIMENTACAO
#define SERVER_IP "http://devcontroleveterinario.azurewebsites.net/api/Alimentacao/ParoAlimentar"

//Parameters
const int ipaddress[4] = {103, 97, 67, 25};
String RFID_ID;

//char* ssid = "Minney";
//char* password = "trialis102";

char* ssid = "POCO F3";
char* password = "vitor123";

//Variables
byte nuidPICC[4] = {0, 0, 0, 0};
//Constants

MFRC522::MIFARE_Key key;

MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);

void setup() {
  //Init Serial USBA
  Serial.begin(115200);

  wifiConnect();

  Serial.println("configurando serial");
  Serial.println(F("Initialize System"));
  //init rfid D8,D5,D6,D7
  SPI.begin();
  rfid.PCD_Init();
  Serial.print(F("Reader :"));
  rfid.PCD_DumpVersionToSerial();
}

void loop() {
  readRFID();
}
void readRFID(void ) { /* function readRFID */
  ////Read RFID card
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  // Look for new 1 cards
  if ( ! rfid.PICC_IsNewCardPresent())
    return;
  // Verify if the NUID has been readed
  if (  !rfid.PICC_ReadCardSerial())
    return;
  // Store NUID into nuidPICC array
  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }
  RFID_ID = printDec(rfid.uid.uidByte, rfid.uid.size);
  Serial.println("CODIGO RFID = ");
  Serial.println(RFID_ID);
  if (WiFi.status() == WL_CONNECTED && RFID_ID != "") {
    sendPostRequisition();
    // Halt PICC
    rfid.PICC_HaltA();
    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();
  } else {
    Serial.println("O Wifi foi desconectado...conectando novamente");
    wifiConnect();
    // Halt PICC
    rfid.PICC_HaltA();
    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();
  }
}
/**
    Helper routine to dump a byte array as hex values to Serial.
*/
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
/**
    Helper routine to dump a byte array as dec values to Serial.
*/
const String printDec(byte *buffer, byte bufferSize) {
  String conteudo = "";
  byte letra;
  for (byte i = 0; i < bufferSize; i++) {
    conteudo.concat(String(buffer[i] < 0x10 ? " 0" : " "));
    conteudo.concat(String(buffer[i], DEC));
  }
  return conteudo;
}

void wifiConnect() {
  Serial.print("Conectando na rede ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi conectado. Endereço de IP: ");
  Serial.println(WiFi.localIP());
}

void sendPostRequisition() {
  WiFiClient client;
  HTTPClient http;
  String RFID_STRING = '"'+RFID_ID+'"';
  Serial.print("[HTTP] begin...\n");
  // configure traged server and url
  http.begin(client, SERVER_IP); //HTTP
  http.addHeader("Content-Type", "application/json");

  Serial.print("[HTTP] POST...\n");
  // start connection and send HTTP header and body
  Serial.println(RFID_STRING);
  int httpCode = http.POST(RFID_STRING);
  Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      const String& payload = http.getString();
      Serial.println("received payload:\n<<");
      Serial.println(payload);
      Serial.println(">>");
    }
  } else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  RFID_ID = "";
  http.end();

}
