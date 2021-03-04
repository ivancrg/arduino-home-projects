#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#ifndef STASSID
#define STASSID ""
#define STAPSK ""
#endif

const char *ssid = STASSID;
const char *password = STAPSK;
ESP8266WebServer server(80);

#define SOUND_METER_PIN A0 //A0 na esp8266

int ledMode = 0;
int spectrumMode = 0;

//led_pin[0] = ;////D1, D2, D3, D4, D5, D6 na esp8266
#define NUM_LED_PARTS 2
#define NUM_LEDS_IN_PART 44
CRGB leds[NUM_LED_PARTS][NUM_LEDS_IN_PART];

#define MSGEQ_SIZE 7
int spectrumIntensity[MSGEQ_SIZE];

typedef struct colorStruct
{
  int r;
  int g;
  int b;
} Color;

Color spectrum_color[3], fader_color[2];
Color loudness_color, on_color, running_color;

float multiplierLoudnessAverages[5] = {0, 0, 0, 0, 0};
int multiplierLoudnessAveragesPos = 0;

int faderDelay = 25;
float multiplierFader;
int directionFader;

int runningDelay = 75;
int positionRunning;

void handleRoot()
{
  String message = "<!DOCTYPE html><html><head><meta charset=\"utf-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  message += "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jscolor/2.4.0/jscolor.min.js\"></script></head>";
  message += "<body style=\"background-color: #575151; font-family: Helvetica, sans-serif;\"><a href=\"/home\"><h1>Idi na početnu stranicu.</h1></a></body></html>";
  message += "\n";

  server.send(200, "text/html", message);
}

void handleHome()
{
  String message = "<!DOCTYPE html><html><head><meta charset=\"utf-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  message += "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jscolor/2.4.0/jscolor.min.js\"></script>";
  message += "<script>function navigateTo(path){window.location.href = window.location.href.substring(0, window.location.href.length - 4) + path;}";
  message += "</script></head> <body onload=\"setHref();\" style=\"background-color: #575151; font-family: Helvetica, sans-serif;\">";
  message += "<h1>Početna stranica</h1>";
  message += "<a href=\"../spectrumConfig\"><h4 style=\"margin-bottom: 2px;\">Odaberi boje za prikaz frekvencija</h4></a>";
  message += "<a href=\"../loudnessConfig\"><h4 style=\"margin-bottom: 2px;\">Odaberi boju za prikaz vizualizacije zvuka</h4></a>";
  message += "<a href=\"../faderConfig\"><h4 style=\"margin-bottom: 2px;\">Odaberi boju za prikaz prigušenja</h4></a>";
  message += "<a href=\"../onConfig\"><h4 style=\"margin-bottom: 2px;\">Odaberi boju za stalni prikaz</h4></a>";
  message += "<a href=\"../runningConfig\"><h4 style=\"margin-bottom: 2px;\">Odaberi boju za prikaz trčećeg svjetla</h4></a>";
  message += "<button onclick=\"navigateTo('spectrumShow')\" style=\"margin-top: 20px; font-weight: bold; font-size: 15px; background-color: #4CAF50; color: white; border: none; text-align: center; padding: 6px 12px;\">Pokreni prikaz po frekvenciji</button><br/>";
  message += "<button onclick=\"navigateTo('loudnessShow')\" style=\"margin-top: 20px; font-weight: bold; font-size: 15px; background-color: #4CAF50; color: white; border: none; text-align: center; padding: 6px 12px;\">Pokreni prikaz po glasnoći</button><br/>";
  message += "<button onclick=\"navigateTo('faderShow')\" style=\"margin-top: 20px; font-weight: bold; font-size: 15px; background-color: #4CAF50; color: white; border: none; text-align: center; padding: 6px 12px;\">Pokreni prikaz prigušenja</button><br/>";
  message += "<button onclick=\"navigateTo('onShow')\" style=\"margin-top: 20px; font-weight: bold; font-size: 15px; background-color: #4CAF50; color: white; border: none; text-align: center; padding: 6px 12px;\">Pokreni prikaz stalni prikaz</button><br/>";
  message += "<button onclick=\"navigateTo('runningShow')\" style=\"margin-top: 20px; font-weight: bold; font-size: 15px; background-color: #4CAF50; color: white; border: none; text-align: center; padding: 6px 12px;\">Pokreni prikaz trčećeg svjetla</button><br/>";
  message += "</body> </html>";
  message += "\n";

  server.send(200, "text/html", message);
}

void handleSpectrumConfig()
{
  String message = "<!DOCTYPE html><html>";
  message += "<head><meta charset=\"utf-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  message += "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jscolor/2.4.0/jscolor.min.js\"></script>";
  message += "</head><body style=\"background-color: #575151; font-family: Helvetica, sans-serif;\" onload=\"initP();\">";
  message += "<h1>Odabir postavki prikaza po frekvencijama</h1>";
  message += "<h4 style=\"margin-bottom: 2px;\">Odaberi boju za prikaz niskih frekvencija:</h4>";
  message += "<input data-jscolor=\"{preset: 'dark'}\" id=\"p1\"> <br/>";
  message += "<h4 style=\"margin-bottom: 2px;\">Odaberi boju za prikaz srednjih frekvencija:</h4>";
  message += "<input data-jscolor=\"{preset: 'dark'}\" id=\"p2\"> <br/>";
  message += "<h4 style=\"margin-bottom: 2px;\">Odaberi boju za prikaz visokih frekvencija:</h4>";
  message += "<input data-jscolor=\"{preset: 'dark'}\" id=\"p3\"> <br/>";
  message += "<h4 style=\"margin-bottom: 2px\">Odaberi način prikaza frekvencija:</h4>";
  message += "<select name=\"mode\" id=\"mode\"><option value=\"0\">Od sredine</option><option value=\"1\">Svaki posebno</option></select><br /><br />";
  message += "<button onclick=\"pOut();\" style=\"font-weight: bold; font-size: 15px; background-color: #4CAF50; color: white; border: none; text-align: center; padding: 6px 12px;\">Spremi odabrane postavke</button>";
  message += "<script> function initP(){";
  message += "document.querySelector('#p1').jscolor.fromString(\"#575151\");";
  message += "document.querySelector('#p2').jscolor.fromString(\"#575151\");";
  message += "document.querySelector('#p3').jscolor.fromString(\"#575151\");}";
  message += "function pOut(){";
  message += "message = \"rgb1=\";";
  message += "message += \"r\" + Math.round(document.querySelector('#p1').jscolor.channel('R'));";
  message += "message += \"g\" + Math.round(document.querySelector('#p1').jscolor.channel('G'));";
  message += "message += \"b\" + Math.round(document.querySelector('#p1').jscolor.channel('B')) + \"%3B\";";
  message += "message += \"&rgb2=\";";
  message += "message += \"r\" + Math.round(document.querySelector('#p2').jscolor.channel('R'));";
  message += "message += \"g\" + Math.round(document.querySelector('#p2').jscolor.channel('G'));";
  message += "message += \"b\" + Math.round(document.querySelector('#p2').jscolor.channel('B')) + \"%3B\";";
  message += "message += \"&rgb3=\";";
  message += "message += \"r\" + Math.round(document.querySelector('#p3').jscolor.channel('R'));";
  message += "message += \"g\" + Math.round(document.querySelector('#p3').jscolor.channel('G'));";
  message += "message += \"b\" + Math.round(document.querySelector('#p3').jscolor.channel('B')) + \"%3B\";";
  message += "message += \"&mode=\" + document.getElementById(\"mode\").value;";
  //message += "window.location.href = window.location.href + \"/spectrum?\" + message;}";
  message += "window.location.href = window.location.href.substring(0, window.location.href.length - 14) + \"spectrumSave?\" + message;}";
  message += "</script>";
  message += "</body></html>";
  message += "\n";

  server.send(200, "text/html", message);
}

void handleSpectrumSave()
{
  if (server.args() > 4)
  {
    handleNotFound();
    return;
  }

  for (uint8_t i = 0; i < server.args() - 1; i++)
  {
    //argument tipa r87g81b81;
    int posR = server.arg(i).indexOf('r');
    int posG = server.arg(i).indexOf('g');
    int posB = server.arg(i).indexOf('b');
    int posEnd = server.arg(i).indexOf(';');

    spectrum_color[i].r = server.arg(i).substring(posR + 1, posG).toInt();
    spectrum_color[i].g = server.arg(i).substring(posG + 1, posB).toInt();
    spectrum_color[i].b = server.arg(i).substring(posB + 1, posEnd).toInt();
  }

  spectrumMode = server.arg(server.args() - 1).toInt();

  //check
  for (uint8_t i = 0; i < 3; ++i)
  {
    Serial.print(spectrum_color[i].r);
    Serial.print(" ");
    Serial.print(spectrum_color[i].g);
    Serial.print(" ");
    Serial.println(spectrum_color[i].b);
  }

  Serial.println(spectrumMode);

  String message = "<!DOCTYPE html><html>";
  message += "<head><meta charset=\"utf-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  message += "</head><body style=\"background-color: #575151; font-family: Helvetica, sans-serif;\">";
  message += "<h1>Postavke spremljene</h1>";
  message += "<h4 style=\"color: rgb(" + String(spectrum_color[0].r) + ", " + String(spectrum_color[0].g) + ", " + String(spectrum_color[0].b) + ");\">Niske frekvencije</h4>";
  message += "<h4 style=\"color: rgb(" + String(spectrum_color[1].r) + ", " + String(spectrum_color[1].g) + ", " + String(spectrum_color[1].b) + ");\">Srednje frekvencije</h4>";
  message += "<h4 style=\"color: rgb(" + String(spectrum_color[2].r) + ", " + String(spectrum_color[2].g) + ", " + String(spectrum_color[2].b) + ");\">Visoke frekvencije</h4>";
  message += "<br/><br/><a href=\"/home\"><h4>Idi na početnu stranicu.</h4></a></body></html></body></html>";
  message += "\n";

  server.send(200, "text/html", message);
}

void handleOneColorConfig(String path)
{
  //path = loudness ili fader ili on ili running
  //loudnessConfig, faderConfig, onConfig, runningConfig

  String message = "<!DOCTYPE html> <html> <head>";
  message += "<meta charset=\"utf-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  message += "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jscolor/2.4.0/jscolor.min.js\"></script>";
  message += "</head> <body style=\"background-color: #575151; font-family: Helvetica, sans-serif;\" onload=\"initP()\">";

  if (!strcmp(path.c_str(), "loudness"))
  {
    message += "<h1>Odabir boje prikaza vizualizacije zvuka</h1>";
    message += "<h4 style=\"margin-bottom: 2px;\">Odaberi boju za prikaz vizualizacije zvuka:</h4>";
  }
  else if (!strcmp(path.c_str(), "fader"))
  {
    message += "<h1>Odabir postavki za prikaz prigušenja</h1>";
    message += "<h4 style=\"margin-bottom: 2px;\">Odaberi prvu boju za prigušenje:</h4>";
  }
  else if (!strcmp(path.c_str(), "on"))
  {
    message += "<h1>Odabir boje stalnog prikaza</h1>";
    message += "<h4 style=\"margin-bottom: 2px;\">Odaberi boju za stalni prikaz:</h4>";
  }
  else if (!strcmp(path.c_str(), "running"))
  {
    message += "<h1>Odabir postavki prikaza trčećeg svjetla</h1>";
    message += "<h4 style=\"margin-bottom: 2px;\">Odaberi boju za prikaz trčećeg svjetla:</h4>";
  }

  message += "<input data-jscolor=\"{preset: 'dark'}\" id=\"p1\"> <br/>";

  if (!strcmp(path.c_str(), "fader"))
  {
    message += "<h4 style=\"margin-bottom: 2px;\">Odaberi drugu boju za prigušenje:</h4>";
    message += "<input data-jscolor=\"{preset: 'dark'}\" id=\"p2\"> <br/>";
  }

  if (!strcmp(path.c_str(), "fader") || !strcmp(path.c_str(), "running"))
  {
    message += "<h4 style=\"margin-bottom: 2px\">Odaberi kašnjenje u milisekundama:</h4> <input type=\"number\" id=\"delay\" name=\"quantity\" min=\"0\" max=\"999999\" /> <br />";
  }

  message += "<br/> <button onclick=\"pOut();\" style=\"font-weight: bold; font-size: 15px; background-color: #4CAF50; color: white; border: none; text-align: center; padding: 6px 12px;\">Spremi odabranu boju</button>";
  message += "<script> function initP(){";
  message += "document.querySelector('#p1').jscolor.fromString(\"#575151\");";
  if (!strcmp(path.c_str(), "fader"))
  {
    message += "document.querySelector('#p2').jscolor.fromString(\"#575151\");";
  }
  message += "} function pOut(){ message = \"rgb1=\";";
  message += "message += \"r\" + Math.round(document.querySelector('#p1').jscolor.channel('R'));";
  message += "message += \"g\" + Math.round(document.querySelector('#p1').jscolor.channel('G'));";
  message += "message += \"b\" + Math.round(document.querySelector('#p1').jscolor.channel('B')) + \";\";";

  if (!strcmp(path.c_str(), "fader"))
  {
    message += "message += \"&rgb2=\";";
    message += "message += \"r\" + Math.round(document.querySelector('#p2').jscolor.channel('R'));";
    message += "message += \"g\" + Math.round(document.querySelector('#p2').jscolor.channel('G'));";
    message += "message += \"b\" + Math.round(document.querySelector('#p2').jscolor.channel('B')) + \";\";";
  }

  if (!strcmp(path.c_str(), "fader") || !strcmp(path.c_str(), "running"))
  {
    message += "message += \"&delay=\" + document.getElementById(\"delay\").value;";
  }

  if (!strcmp(path.c_str(), "loudness"))
  {
    message += "window.location.href = window.location.href.substring(0, window.location.href.length - 14) + \"loudnessSave?\" + message;}; ";
  }
  else if (!strcmp(path.c_str(), "fader"))
  {
    message += "window.location.href = window.location.href.substring(0, window.location.href.length - 11) + \"faderSave?\" + message;}; ";
  }
  else if (!strcmp(path.c_str(), "on"))
  {
    message += "window.location.href = window.location.href.substring(0, window.location.href.length - 8) + \"onSave?\" + message;}; ";
  }
  else if (!strcmp(path.c_str(), "running"))
  {
    message += "window.location.href = window.location.href.substring(0, window.location.href.length - 13) + \"runningSave?\" + message;}; ";
  }

  message += "</script> </body> </html>";
  message += "\n";

  server.send(200, "text/html", message);
}

void handleOneColorSave(String path)
{
  if (server.args() > 3 || server.args() < 1)
  {
    handleNotFound();
    return;
  }
  
  //koristimo za ispis (i postavljanje osim ako je u pitanju fader)
  int posR = server.arg(0).indexOf('r');
  int posG = server.arg(0).indexOf('g');
  int posB = server.arg(0).indexOf('b');
  int posEnd = server.arg(0).indexOf(';');
  int r = server.arg(0).substring(posR + 1, posG).toInt();
  int g = server.arg(0).substring(posG + 1, posB).toInt();
  int b = server.arg(0).substring(posB + 1, posEnd).toInt();

  if (!strcmp(path.c_str(), "fader"))
  {
    for (uint8_t i = 0; i < server.args() - 1; i++)
    {
      //argument tipa r87g81b81;
      int posR = server.arg(i).indexOf('r');
      int posG = server.arg(i).indexOf('g');
      int posB = server.arg(i).indexOf('b');
      int posEnd = server.arg(i).indexOf(';');

      fader_color[i].r = server.arg(i).substring(posR + 1, posG).toInt();
      fader_color[i].g = server.arg(i).substring(posG + 1, posB).toInt();
      fader_color[i].b = server.arg(i).substring(posB + 1, posEnd).toInt();
    }

    faderDelay = server.arg(server.args() - 1).toInt();
  }
  else
  {    
    if (!strcmp(path.c_str(), "loudness"))
    {
      loudness_color.r = r;
      loudness_color.g = g;
      loudness_color.b = b;
    }
    else if (!strcmp(path.c_str(), "on"))
    {
      on_color.r = r;
      on_color.g = g;
      on_color.b = b;
    }
    else if (!strcmp(path.c_str(), "running"))
    {
      running_color.r = r;
      running_color.g = g;
      running_color.b = b;
      runningDelay = server.arg(server.args() - 1).toInt();
    }
  }

  //check
  Serial.print(loudness_color.r);
  Serial.print(" ");
  Serial.print(loudness_color.g);
  Serial.print(" ");
  Serial.println(loudness_color.b);
  Serial.print(fader_color[0].r);
  Serial.print(" ");
  Serial.print(fader_color[0].g);
  Serial.print(" ");
  Serial.println(fader_color[0].b);
  Serial.print(fader_color[1].r);
  Serial.print(" ");
  Serial.print(fader_color[1].g);
  Serial.print(" ");
  Serial.println(fader_color[1].b);
  Serial.print(on_color.r);
  Serial.print(" ");
  Serial.print(on_color.g);
  Serial.print(" ");
  Serial.println(on_color.b);
  Serial.print(running_color.r);
  Serial.print(" ");
  Serial.print(running_color.g);
  Serial.print(" ");
  Serial.println(running_color.b);

  String message = "<!DOCTYPE html><html>";
  message += "<head><meta charset=\"utf-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  message += "</head><body style=\"background-color: #575151; font-family: Helvetica, sans-serif;\">";

  if (!strcmp(path.c_str(), "fader"))
  {
    message += "<h1>Odabir boja spremljen</h1>";
    message += "<h4 style=\"color: rgb(" + String(fader_color[0].r) + ", " + String(fader_color[0].g) + ", " + String(fader_color[0].b) + ");\">Prva boja</h4>";
    message += "<h4 style=\"color: rgb(" + String(fader_color[1].r) + ", " + String(fader_color[1].g) + ", " + String(fader_color[1].b) + ");\">Druga boja</h4>";
  }
  else
  {
    message += "<h1>Odabir boje spremljen</h1>";
    message += "<h4 style=\"color: rgb(" + String(r) + ", " + String(g) + ", " + String(b) + ");\">Odabrana boja</h4>";
  }

  message += "<br/><br/><a href=\"/home\"><h4>Idi na početnu stranicu.</h4></a></body></html>";
  message += "\n";

  server.send(200, "text/html", message);
}

void handleShow(String path)
{
  if (!strcmp(path.c_str(), "spectrum"))
  {
    ledMode = 0;
  }
  else if (!strcmp(path.c_str(), "loudness"))
  {
    ledMode = 1;
    for (int i = 0; i < 5; ++i)
      multiplierLoudnessAverages[i] = 0.0;
  }
  else if (!strcmp(path.c_str(), "fader"))
  {
    multiplierFader = 0.0;
    directionFader = 0;
    ledMode = 2;
  }
  else if (!strcmp(path.c_str(), "on"))
  {
    ledMode = 3;
    onInColor();
  }
  else if (!strcmp(path.c_str(), "running"))
  {
    positionRunning = 0;
    ledMode = 4;
  }

  Serial.println(ledMode);

  String message = "<!DOCTYPE html><html><head><meta charset=\"utf-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  message += "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jscolor/2.4.0/jscolor.min.js\"></script></head>";
  message += "<body onload=\"setHref();\" style=\"background-color: #575151; font-family: Helvetica, sans-serif;\"><h1>Pokrenuto.</h1><br/><br/><a href=\"/home\"><h4>Idi na početnu stranicu.</h4></a></body></body></html>";
  message += "\n";

  server.send(200, "text/html", message);
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

//DUMMY
void readMSGEQ7()
{
  //Serial.print("DUMMY MSGEQ7: ");
  for (int i = 0; i < 7; ++i)
  {
    spectrumIntensity[i] = random(1024);
    //Serial.print(spectrumIntensity[i]); Serial.print(", ");
  }
 // Serial.println();
}

//NIJE TESTIRANO S MSGEQ7
void spectrumInColors()
{
  readMSGEQ7();
     
  if (spectrumMode == 0)
  {
    //44 ledice --> intenzitet treba mapirati na 0-22 (širi se od sredine prema krajevima)
    //intenzitet može biti 0-1023
    int len[3] = {
        map((spectrumIntensity[0] + spectrumIntensity[1]) / 2, 0, 1023, 0, NUM_LEDS_IN_PART / 2),
        map((spectrumIntensity[2] + spectrumIntensity[3] + spectrumIntensity[4]) / 3, 0, 1023, 0, NUM_LEDS_IN_PART / 2),
        map((spectrumIntensity[5] + spectrumIntensity[6]) / 2, 0, 1023, 0, NUM_LEDS_IN_PART / 2)
        };
     
    //Serial.print("Intensity: ");
    //Serial.print(len[0]); Serial.print(", "); Serial.print(len[1]); Serial.print(", "); Serial.print(len[2]);
    //Serial.println();
    
    for (int part = 0; part < NUM_LED_PARTS; ++part)
    {
      //ledice svijetle od pozicije st do en
      int st = (NUM_LEDS_IN_PART / 2) - len[part];
      int en = ((NUM_LEDS_IN_PART / 2) - 1) + len[part];
      // 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 | 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43
      // -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- | -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
      //Serial.print(len[part]); Serial.print(", start: "); Serial.print(st); Serial.print(", end:"); Serial.print(en);
      //Serial.println();
  
      for (int i = 0; i < NUM_LEDS_IN_PART; ++i)
      {
        if (i < st || i > en)
        {
          leds[part][i] = CRGB(0, 0, 0);
        }
        else
        {
          leds[part][i] = CRGB(spectrum_color[part].r, spectrum_color[part].g, spectrum_color[part].b);
        }
      }

      //Serial.print("PART = "); Serial.print(part); Serial.print(", color = "); Serial.print(spectrum_color[part].r); Serial.print(" "); Serial.print(spectrum_color[part].g); Serial.print(" "); Serial.print(spectrum_color[part].b); Serial.println();
    }
  }
  else if (spectrumMode == 1)
  {
    int select = 0; //select se mice od 0 do 6 --> MSGEQ sensitivity po svakoj ledici uz to da su 0, 1 boje niskog, 2, 3, 4 boje srednjeg, 5, 6 boje visokog
    float sensitivity[7] = {
      map(spectrumIntensity[0], 0, 1023, 0, 1),
      map(spectrumIntensity[1], 0, 1023, 0, 1),
      map(spectrumIntensity[2], 0, 1023, 0, 1),
      map(spectrumIntensity[3], 0, 1023, 0, 1),
      map(spectrumIntensity[4], 0, 1023, 0, 1),
      map(spectrumIntensity[5], 0, 1023, 0, 1),
      map(spectrumIntensity[6], 0, 1023, 0, 1)
    };
    
    for (int part = 0; part < NUM_LED_PARTS; ++part)
    {
      for (int i = 0; i < NUM_LEDS_IN_PART; ++i)
      {
        if(select < 2)
        {
          leds[part][i] = CRGB(spectrum_color[0].r * sensitivity[select], spectrum_color[0].g * sensitivity[select], spectrum_color[0].b * sensitivity[select]);
        }
        else if(select < 5)
        {
          leds[part][i] = CRGB(spectrum_color[1].r * sensitivity[select], spectrum_color[1].g * sensitivity[select], spectrum_color[1].b * sensitivity[select]);
        }
        else
        {
          leds[part][i] = CRGB(spectrum_color[2].r * sensitivity[select], spectrum_color[2].g * sensitivity[select], spectrum_color[2].b * sensitivity[select]);
        }

        select = (select + 1) % 7; 
      }
    }
  }

  FastLED.show();
}

void loudnessInColor(float offset = 0.0)
{
  float sum = 0.0;
  float multiplier; //treba pročitat jačinu zvuka

  for (int i = 0; i < 7; ++i)
  {
    sum += analogRead(SOUND_METER_PIN);
  }

  multiplier = sum / 7 / 1024.0;
  if (multiplier < 0.04)
    multiplier = 0.0;
  multiplier += offset;

  multiplierLoudnessAverages[(multiplierLoudnessAveragesPos++) % 5] = multiplier;
  sum = 0.0;

  for (int i = 0; i < 5; ++i)
  {
    Serial.print(multiplierLoudnessAverages[i]);
    Serial.print(" ");
    sum += multiplierLoudnessAverages[i];
  }
  multiplier = sum / 5.0;
  Serial.println(multiplier);

  for (int part = 0; part < NUM_LED_PARTS; ++part)
  {
    for (int i = 0; i < NUM_LEDS_IN_PART; ++i)
    {
      leds[part][i] = CRGB(round(loudness_color.r * multiplier), round(loudness_color.g * multiplier), round(loudness_color.b * multiplier));
    }

    //Serial.print(leds[part][0].r); Serial.print(" "); Serial.print(leds[part][0].g); Serial.print(" "); Serial.print(leds[part][0].b); Serial.print(" ");
  }

  FastLED.show();
}

void fadeInColors(float mStep, int delayMs)
{
  // boje su fader_color[0].r, fader_color[0].g, fader_color[0].b i fader_color[1].r, fader_color[1].g, fader_color[1].b
  float r0 = fader_color[0].r;
  float g0 = fader_color[0].g;
  float b0 = fader_color[0].b;
  float r1 = fader_color[1].r;
  float g1 = fader_color[1].g;
  float b1 = fader_color[1].b;

  for (int part = 0; part < NUM_LED_PARTS; ++part)
  {
    for (int i = 0; i < NUM_LEDS_IN_PART; ++i)
    {
      //linearna interpolacija x(t) = x0 + (x1 − x0) · t
      //linearna interpolacija rout = r0 + (r1 - r0) * multiplierFader

      int rout = round(r0 + (r1 - r0) * multiplierFader);
      int gout = round(g0 + (g1 - g0) * multiplierFader);
      int bout = round(b0 + (b1 - b0) * multiplierFader);

      if (rout > 255)
        rout = 255;
      else if (rout < 0)
        rout = 0;

      if (gout > 255)
        gout = 255;
      else if (gout < 0)
        gout = 0;

      if (bout > 255)
        bout = 255;
      else if (bout < 0)
        bout = 0;

      leds[part][i] = CRGB(rout, gout, bout);
    }
  }

  FastLED.show();

  /*//check
  Serial.print(leds[0].r); Serial.print(" "); Serial.print(leds[0].g); Serial.print(" "); Serial.print(leds[0].b); Serial.print(" ");
  Serial.print(" "); Serial.println(multiplierFader);*/

  if (!directionFader)
  {
    multiplierFader += mStep;
  }
  else
  {
    multiplierFader -= mStep;
  }

  if (multiplierFader >= 1)
  {
    directionFader = 1;
  }
  else if (multiplierFader <= 10e-5)
  {
    directionFader = 0;
  }

  delay(delayMs);
}

void onInColor()
{
  for (int part = 0; part < NUM_LED_PARTS; ++part)
  {
    for (int i = 0; i < NUM_LEDS_IN_PART; ++i)
    {
      leds[part][i] = CRGB(on_color.r, on_color.g, on_color.b);
    }
  }
  FastLED.show();
}

void runningInColor(int delayMs)
{
  for (int part = 0; part < NUM_LED_PARTS; ++part)
  {
    for (int i = 0; i < NUM_LEDS_IN_PART; ++i)
    {
      if (i + part * NUM_LEDS_IN_PART == positionRunning)
      {
        leds[part][i] = CRGB(running_color.r, running_color.g, running_color.b);
      }
      else
      {
        leds[part][i] = CRGB(0, 0, 0);
      }
    }
  }
  FastLED.show();

  Serial.println();

  positionRunning++;

  if (positionRunning >= (NUM_LED_PARTS * NUM_LEDS_IN_PART))
  {
    positionRunning = 0;
  }

  delay(delayMs);
}

void setup(void)
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  // Set your Static IP address
  IPAddress local_IP(192, 168, 5, 77);
  // Set your Gateway IP address
  IPAddress gateway(192, 168, 5, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress primaryDNS(8, 8, 8, 8);   //optional
  IPAddress secondaryDNS(8, 8, 4, 4); //optional

  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
  {
    Serial.println("STA Failed to configure");
  }

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  server.on("/", handleRoot);
  server.on("/home", handleHome);
  server.on("/spectrumConfig", handleSpectrumConfig);
  server.on("/spectrumSave", handleSpectrumSave);
  server.on("/loudnessConfig", []() {
    handleOneColorConfig("loudness");
  });
  server.on("/loudnessSave", []() {
    handleOneColorSave("loudness");
  });
  server.on("/faderConfig", []() {
    handleOneColorConfig("fader");
  });
  server.on("/faderSave", []() {
    handleOneColorSave("fader");
  });
  server.on("/onConfig", []() {
    handleOneColorConfig("on");
  });
  server.on("/onSave", []() {
    handleOneColorSave("on");
  });
  server.on("/runningConfig", []() {
    handleOneColorConfig("running");
  });
  server.on("/runningSave", []() {
    handleOneColorSave("running");
  });
  server.on("/spectrumShow", []() {
    handleShow("spectrum");
  });
  server.on("/loudnessShow", []() {
    handleShow("loudness");
  });
  server.on("/faderShow", []() {
    handleShow("fader");
  });
  server.on("/onShow", []() {
    handleShow("on");
  });
  server.on("/runningShow", []() {
    handleShow("running");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  for (int part = 0; part < NUM_LED_PARTS; ++part)
  {
    if (part == 0)
      FastLED.addLeds<WS2812, 5, GRB>(leds[part], NUM_LEDS_IN_PART);
    else if (part == 1)
      FastLED.addLeds<WS2812, 4, GRB>(leds[part], NUM_LEDS_IN_PART);
    else if (part == 2)
      FastLED.addLeds<WS2812, 0, GRB>(leds[part], NUM_LEDS_IN_PART);
    else if (part == 3)
      FastLED.addLeds<WS2812, 2, GRB>(leds[part], NUM_LEDS_IN_PART);
    else if (part == 4)
      FastLED.addLeds<WS2812, 14, GRB>(leds[part], NUM_LEDS_IN_PART);
    else if (part == 5)
      FastLED.addLeds<WS2812, 12, GRB>(leds[part], NUM_LEDS_IN_PART);
  }
  pinMode(SOUND_METER_PIN, INPUT);
}

void loop(void)
{
  server.handleClient();

  //izvedi izabranu funkciju
  if (ledMode == 0)
  {
    spectrumInColors();
    delay(30);
  }
  else if (ledMode == 1)
  {
    loudnessInColor();
  }
  else if (ledMode == 2)
  {
    fadeInColors(0.0075, faderDelay);
  }
  else if (ledMode == 3)
  {
    //implementirano prije, treba samo jedan loop
  }
  else if (ledMode == 4)
  {
    runningInColor(runningDelay);
  }
}
