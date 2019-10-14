#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

// define intervalo de leitura dos sensores
#define INTERVALO 30000

// aqui define o ID da balança
#define ID_BALANCA 1

// variavel de tempo de espera
unsigned long tempoEspera = 0;

// variaveis da balança
String inString = "";

//Definir o SSID da rede WiFi
const char* ssid = "SystemSegPlaca1";
const char* password = "systemseg";

SoftwareSerial swSerial(14, 12); // RX, TX

int             ESPServerPort  = 9001;
IPAddress       ESPServer(192,168,4,1);
WiFiClient      ESPClient;


void CheckWiFiConnectivity()
{
  while(WiFi.status() != WL_CONNECTED)
  {
    for(int i=0; i < 10; i++)
    {
      delay(50);
    }
    Serial.print(".");
  }

  Serial.println("OK!");
}

void ESPRequest()
{
  // First Make Sure You Got Disconnected
  ESPClient.stop();

  // Connect to Send Messages
  if (ESPClient.connect(ESPServer, ESPServerPort)) {
    Serial.println("ESP Conectado com sucesso!");
  }
}

void ConnectWiFi() {
  if(WiFi.status() == WL_CONNECTED)
  {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    delay(50);
  }

  WiFi.mode(WIFI_STA);                // To Avoid Broadcasting An SSID
    
  //Inicia o WiFi
  WiFi.begin(ssid, password);
}
  
  
void setup() 
{
  //Configuração da UART
  Serial.begin(9600);
  swSerial.begin(9600);

  // Starting To Connect --------------------------------------------------
  ConnectWiFi();

  Serial.print("Conectando no ESP principal..");

  CheckWiFiConnectivity();

  // Conecting The Device As A Client -------------------------------------
  ESPRequest();

  // iniciando tempo de espera do envio da leitura
  tempoEspera = millis() + INTERVALO;

  Serial.println("Pronto!");
}

void loop() 
{
  // Faz leitura da balança
  if (swSerial.available()) {
    int inChar = swSerial.read();
    if (inChar != '\n') {
      inString += (char)inChar;
    }
    else {

      Serial.println("Recebi uma nova linha");
      
      // Espera para fazer o envio da leitura
      if (tempoEspera < millis()) {
        
        // adiciona o ID da balança na string
        inString += ",bl=";
        inString += String(ID_BALANCA);

        if (WiFi.status() != WL_CONNECTED) {
          Serial.println("Perdeu a conexão, reconectando...");
          ConnectWiFi();
          CheckWiFiConnectivity();
        }
        
        // envia leitura da balança para o ESP01 da placa principal
        ESPClient.println(inString);

        Serial.print("Enviando para o ESP01: ");
        Serial.println(inString);

        // reinicia o tempo de espera
        tempoEspera = millis() + INTERVALO;
      }

      // Reinicia inString para nova leitura
      inString = "";
    }
  }
}
