#include <DHT.h>
#include <DHT_U.h>
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

#define REQ_BUF_SZ   90

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(169, 254, 148, 255);

EthernetServer server(80);

char HTTP_req[REQ_BUF_SZ] = {0};
char req_index = 0;


String readstring;
double gas_val;
double humedadS_val;

int tempC;
int tempF;

int c_length = 0;



/*DIMMER VARIABLES*/
int ZC = 7;//sdfsdf
int Z;
int up = 0;
int Data;
int Z_OUT = 8;
int TRIAC_TIME = 100;
int Time = 0;
int dimmer_val = 0;
/*-------------------*/

/*SENSOR PIR*/
const int pir = 7 ;
/*------------------*/

/*SENSOR TEMPERATURA Y HUMEDAD*/
#define DHTPIN 6
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
/*-------------------------*/


void setup() {
  Serial.begin(115200);
  dht.begin();

  //DIMMER
  pinMode(ZC, INPUT);
  pinMode(Z_OUT, OUTPUT);

  pinMode(A1, INPUT); //SENSOR DE GAS
  pinMode(A2, INPUT); //SENSOR HIGROMETRO
  DHT dht(DHTPIN, DHTTYPE); //SENSOR DE TEMPERATURA Y HUMEDAD
  pinMode(10, OUTPUT);

  pinMode(pir, INPUT); //SENSOR DE PRESENCIA

  digitalWrite(10, HIGH);
  Serial.print("Iniciando SD card...");
  if (!SD.begin(4)) {
    Serial.println("Error!");
    return;
  }
  Serial.println("Conectado.");

  Ethernet.begin(mac, ip);
  digitalWrite(10, HIGH);

  server.begin();
  Ethernet.localIP();
}

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        c_length++;
        char c = client.read();
        if (req_index < (REQ_BUF_SZ - 1)) {
          HTTP_req[req_index] = c;
          if (req_index > 42 && req_index < 70) {
            readstring += c;
          }
          req_index++;
        }
        if (c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          if (StrContains(HTTP_req, "ajax_inputs&no")) {
            // send rest of HTTP header
            client.println("Content-Type: text/xml");
            client.println("Connection: keep-alive");
            client.println();
            // send XML file containing input states

            dimmer();

            XML_response(client);
            readstring = "";
          }
          else {
            client.println("Content-Type: text/html");
            client.println("Connection: keep-alive");
            client.println();
            File html = SD.open("index.htm");
            if (html) {
              while (html.available()) {
                client.write(html.read());
              }
              // close the file:
              html.close();
            }
          }
          req_index = 0;
          StrClear(HTTP_req, REQ_BUF_SZ);
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    // close the connection:
    client.stop();
  }

}

void dimmer() {
  int ind1 = readstring.indexOf("dimmer");
  String finalstring = readstring.substring(ind1 + 7, ind1 + 10);
  dimmer_val = finalstring.toInt();
  Serial.write("DIMMER\n");
  Serial.println(dimmer_val);
  Serial.println("");
}

void XML_response(EthernetClient cl)
{
  int dec_pir;

  cl.print("<?xml version = \"1.0\" ?>");
  cl.print("<inputs>");
  // read analog pin A2

  gas_val = analogRead(A1);
  humedadS_val = analogRead(A2);

  int humedad_val = dht.readHumidity();// Lee la humedad
  int temp_val = dht.readTemperature(); //Lee la temperatura

  tempC = temp_val;
  tempF = (tempC * 1.8) + 32;


  dec_pir = digitalRead(pir);

  cl.print("<gas>");
  cl.print((gas_val * 100) / 1023);
  cl.print("</gas>");

  cl.print("<presencia>");
  if (dec_pir == 1) {
    cl.print("ON");
  }
  else if (dec_pir == 0) {
    cl.print("OFF");
  }
  cl.print("</presencia>");

  cl.print("<tempC>");
  cl.print(tempC);
  cl.print("</tempC>");

  cl.print("<tempF>");
  cl.print(tempF);
  cl.print("</tempF>");

  cl.print("<humedad>");
  cl.print(humedad_val);
  cl.print("</humedad>");

  cl.print("<humedadS>");
  cl.print((humedadS_val * 100) / 1023);
  cl.print("</humedadS>");

  cl.print("</inputs>");
}

void StrClear(char *str, char length)
{
  for (int i = 0; i < length; i++) {
    str[i] = 0;
  }
}


char StrContains(char *str, char *sfind)
{
  char found = 0;
  char index = 0;
  char len;

  len = strlen(str);

  if (strlen(sfind) > len) {
    return 0;
  }
  while (index < len) {
    if (str[index] == sfind[found]) {
      found++;
      if (strlen(sfind) == found) {
        return 1;
      }
    }
    else {
      found = 0;
    }
    index++;
  }
  return 0;
}
