#define FASTLED_ESP8266_RAW_PIN_ORDER
#include "FastLED.h"
#define PIN D8                                                          // к какой ноге подключена лента
#define led 18                                                          // сколько светодиодов в ленте
CRGB strip[led];                                                        // создаем массив светодиодов
#define COLOR_ORDER RGB
#include <ESP8266WiFi.h>                                                
#include <ESP8266WebServer.h>                                          
#include <FS.h>                                                        
#include <ESP8266FtpServer.h>                                          
#include <ArduinoJson.h>


const byte relay = 4;                                                   
const char *ssid = "MyLEDSTRIP";                                        // название wifi точки                                            

ESP8266WebServer HTTP(80);                                              // Определяем объект и порт сервера для работы с HTTP
FtpServer ftpSrv;                                                       // Определяем объект для работы с модулем по FTP (для отладки HTML)

void setup() {
  pinMode(relay,OUTPUT);                                                // Определяем пин реле как исходящий
  Serial.begin(9600);                                                   // Инициализируем вывод данных на серийный порт со скоростью 9600 бод
  WiFi.softAP(ssid);                                                    // Создаём точку доступа
  SPIFFS.begin();                                                       // Инициализируем работу с файловой системой                          
  HTTP.begin();                                                         // Инициализируем Web-сервер
  ftpSrv.begin("relay","relay");                                        // Поднимаем FTP-сервер для удобства отладки работы HTML (логин: relay, пароль: relay)

  Serial.print("\nMy IP to connect via Web-Browser or FTP: ");          // Выводим на монитор серийного порта сообщение о том, что сейчас будем выводить локальный IP
  Serial.println(WiFi.softAPIP());                                      // Выводим локальный IP-адрес ESP8266
  Serial.println("\n");
  FastLED.addLeds<NEOPIXEL,PIN>(strip, led);

// Обработка HTTP-запросов
 HTTP.on("/MyLedstripSingle", my_ledstrip_single);                                 
 HTTP.on("/MyLedstripRainbow", my_ledstrip_rainbow);
 HTTP.on("/MyLedstripGradient", my_ledstrip_gradient);
 HTTP.on("/MyLedstripSolid", my_ledstrip_solid);
 HTTP.on("/off", my_ledstrip_off);        
  HTTP.onNotFound([](){                                                 // Описываем действия при событии "Не найдено"
  if(!handleFileRead(HTTP.uri()))                                       
      HTTP.send(404, "text/plain", "Not Found");                        
  });
}

void loop() {
    HTTP.handleClient();                                                // Обработчик HTTP-событий (отлавливает HTTP-запросы к устройству и обрабатывает их в соответствии с выше описанным алгоритмом)
    ftpSrv.handleFTP();                                                 // Обработчик FTP-соединений  
}


void my_ledstrip_single() {                                              //функция для включения одного светодиода
 Serial.println("данные\n");                                             
 const size_t capacity = 100;            
 DynamicJsonBuffer jsonBuffer(capacity);                                 
 JsonObject& root = jsonBuffer.parseObject(HTTP.arg("plain"));
 uint8_t Single_num =  root["num_sing"].as<uint8_t>();                // извлекаем номер светодиода
 uint8_t s_c_r=  root["col_sing_r"].as<uint8_t>();                    // извлекаем красный
 uint8_t s_c_g=  root["col_sing_g"].as<uint8_t>();                    // извлекаем зеленый
 uint8_t s_c_b=  root["col_sing_b"].as<uint8_t>();                    // извлекаем синий
 strip[Single_num-1] = CRGB(s_c_r,s_c_g,s_c_b);                    // включаем светодиод
 FastLED.show();
 jsonBuffer.clear();                                               // очищаем буфер 
}

void my_ledstrip_rainbow() {                                              //функция для включения радуги
 Serial.println("данные\n");                                             
 const size_t capacity = 100;            
 DynamicJsonBuffer jsonBuffer(capacity);                                 // Инициализируем буфер под JSON
 JsonObject& root = jsonBuffer.parseObject(HTTP.arg("plain"));
 uint8_t number_to =  root["num_to"].as<uint8_t>();
 uint8_t number_move =  root["num_move"].as<uint8_t>();
 uint8_t number_quantity =  root["num_quantity"].as<uint8_t>();
 Serial.println(number_to); 
 Serial.println(number_move);
 Serial.println(number_quantity);
 fill_rainbow (strip,number_to,number_move, number_quantity);
 FastLED.show();
 jsonBuffer.clear(); 
 HTTP.send(200, "text/plain", "done!");                    
}


void my_ledstrip_gradient() {                                              //функция для включения градиента
                                            
 const size_t capacity = 200;            
 DynamicJsonBuffer jsonBuffer(capacity);                                 // Инициализируем буфер под JSON
 JsonObject& root = jsonBuffer.parseObject(HTTP.arg("plain"));
 uint8_t number_start_gradient =  root["num_start_grad"].as<uint8_t>();
 uint8_t number_end_gradient =  root["num_end_grad"].as<uint8_t>();
 uint8_t start_color_r = root["color_start_grad_r"].as<uint8_t>();
 uint8_t start_color_g = root["color_start_grad_g"].as<uint8_t>();
 uint8_t start_color_b = root["color_start_grad_b"].as<uint8_t>();
 uint8_t end_color_r = root["color_end_grad_r"].as<uint8_t>();
 uint8_t end_color_g = root["color_end_grad_g"].as<uint8_t>();
 uint8_t end_color_b = root["color_end_grad_b"].as<uint8_t>();  
 Serial.println(number_start_gradient);
 Serial.println(number_end_gradient);
 Serial.println(start_color_r);
 Serial.println(start_color_g);
 Serial.println(start_color_b);
 Serial.println(end_color_r);
 Serial.println(end_color_g);
 Serial.println(end_color_b);
 jsonBuffer.clear();
 fill_gradient_RGB(strip,number_start_gradient-1,CRGB(start_color_r,start_color_g,start_color_b),number_end_gradient-1,CRGB(end_color_r,end_color_g,end_color_b));                     
 FastLED.show();
 HTTP.send(200, "text/plain", "done!");    
}




void my_ledstrip_solid() {                                              //функция для включения заливки
                                
 const size_t capacity = 250;            
 DynamicJsonBuffer jsonBuffer(capacity);                                 // Инициализируем буфер под JSON
 JsonObject& root = jsonBuffer.parseObject(HTTP.arg("plain"));
 uint8_t number_start_solid =  root["number_start_solid"].as<uint8_t>();
 uint8_t color_solid_r = root["color_solid_r"].as<uint8_t>();
 uint8_t color_solid_g = root["color_solid_g"].as<uint8_t>();
 uint8_t color_solid_b = root["color_solid_b"].as<uint8_t>();
 uint8_t num_end_solid = root["number_end_solid"].as<uint8_t>();
 Serial.println(number_start_solid);
 Serial.println(num_end_solid);
 Serial.println(color_solid_r);
 Serial.println(color_solid_g);
 Serial.println(color_solid_b); 
 for(number_start_solid;number_start_solid<=num_end_solid;number_start_solid++)
 {
  strip[number_start_solid-1] = CRGB(color_solid_r,color_solid_g,color_solid_b);
 }
 FastLED.show();
 jsonBuffer.clear();
  HTTP.send(200, "text/plain", "done!");                       
}

void my_ledstrip_off()                                              //функция для выключения всей ленты
{
  for(uint8_t i=0;i < led ;i++)
  {
  strip[i] = CRGB(0,0,0);
  }
  FastLED.show();
  HTTP.send(200, "text/plain", "done!");    
}

bool handleFileRead(String path){                                       // Функция работы с файловой системой
  if(path.endsWith("/")) path += "index.html";                          // Если устройство вызывается по корневому адресу, то должен вызываться файл index.html (добавляем его в конец адреса)
  String contentType = getContentType(path);                            // С помощью функции getContentType (описана ниже) определяем по типу файла (в адресе обращения) какой заголовок необходимо возвращать по его вызову
  if(SPIFFS.exists(path)){                                              // Если в файловой системе существует файл по адресу обращения
    File file = SPIFFS.open(path, "r");                                 //  Открываем файл для чтения
    size_t sent = HTTP.streamFile(file, contentType);                   //  Выводим содержимое файла по HTTP, указывая заголовок типа содержимого contentType
    file.close();                                                       //  Закрываем файл
    return true;                                                        //  Завершаем выполнение функции, возвращая результатом ее исполнения true (истина)
  }
  return false;                                                         // Завершаем выполнение функции, возвращая результатом ее исполнения false (если не обработалось предыдущее условие)
}

String getContentType(String filename){                                 // Функция, возвращающая необходимый заголовок типа содержимого в зависимости от расширения файла
  if (filename.endsWith(".html")) return "text/html";                   // Если файл заканчивается на ".html", то возвращаем заголовок "text/html" и завершаем выполнение функции
  else if (filename.endsWith(".css")) return "text/css";                // Если файл заканчивается на ".css", то возвращаем заголовок "text/css" и завершаем выполнение функции
  else if (filename.endsWith(".js")) return "application/javascript";   // Если файл заканчивается на ".js", то возвращаем заголовок "application/javascript" и завершаем выполнение функции
  else if (filename.endsWith(".png")) return "image/png";               // Если файл заканчивается на ".png", то возвращаем заголовок "image/png" и завершаем выполнение функции
  else if (filename.endsWith(".jpg")) return "image/jpeg";              // Если файл заканчивается на ".jpg", то возвращаем заголовок "image/jpg" и завершаем выполнение функции
  else if (filename.endsWith(".gif")) return "image/gif";               // Если файл заканчивается на ".gif", то возвращаем заголовок "image/gif" и завершаем выполнение функции
  else if (filename.endsWith(".ico")) return "image/x-icon";            // Если файл заканчивается на ".ico", то возвращаем заголовок "image/x-icon" и завершаем выполнение функции
  return "text/plain";                                                  // Если ни один из типов файла не совпал, то считаем что содержимое файла текстовое, отдаем соответствующий заголовок и завершаем выполнение функции
}
