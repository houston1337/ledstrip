#define FASTLED_ESP8266_RAW_PIN_ORDER
#include "FastLED.h"
#define PIN1 D5                                                          // к какой ноге подключена лента GPIO14
#define PIN2 D2                                                          // GPIO4
#define led1 12                                                          // сколько светодиодов в ленте
#define led2 49    
CRGB strip1[led1];   
CRGB strip2[led2];// создаем массив светодиодов
#define COLOR_ORDER RGB
#include <ESP8266WiFi.h>                                                
#include <ESP8266WebServer.h>                                          
#include <FS.h>                                                        
#include <ESP8266FtpServer.h>                                          
#include <ArduinoJson.h>


const byte relay = 4;                                                   
const char *ssid = "MyLEDSTRIP";                                        // название wifi точки                                            
uint8_t last_dinamic_mode =0;
uint8_t gHue = 0;
uint8_t speedl=15;
unsigned long timing;
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
  FastLED.addLeds<NEOPIXEL,PIN1>(strip1, led1);
  FastLED.addLeds<NEOPIXEL,PIN2>(strip2, led2);

/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/******************************************************************************************************************/
/*                                                     Обработка HTTP-запросов                                                    */
/******************************************************************************************************************/
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
  
 HTTP.on("/MyLedstripSingle", my_ledstrip_single);                                 
 HTTP.on("/MyLedstripRainbow", my_ledstrip_rainbow);
 HTTP.on("/MyLedstripGradient", my_ledstrip_gradient);
 HTTP.on("/MyLedstripSolid", my_ledstrip_solid);
 HTTP.on("/MyLedstripPolice1", blink_police1);
  HTTP.on("/MyLedstripPolice2", blink_police2);
 HTTP.on("/MyLedstripRainbowWAVE", rainbow_wave);
 
 HTTP.on("/off", my_ledstrip_off);        
  HTTP.onNotFound([](){                                                 // Описываем действия при событии "Не найдено"
  if(!handleFileRead(HTTP.uri()))                                       
      HTTP.send(404, "text/plain", "Not Found");                        
  });
}

void loop() {
    HTTP.handleClient();                                                // Обработчик HTTP-событий (отлавливает HTTP-запросы к устройству и обрабатывает их в соответствии с выше описанным алгоритмом)
    ftpSrv.handleFTP();                                                 // Обработчик FTP-соединений  
 
   if(last_dinamic_mode==1) 
   {
    blink_police1();
   }
   else if (last_dinamic_mode==2)
   {
    rainbow_wave();
   }
    else if (last_dinamic_mode==3)
   {
    blink_police2();
   }
   
   
}
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/******************************************************************************************************************/
/*                                                    SINGLE                                                      */
/******************************************************************************************************************/
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void my_ledstrip_single() {                                              //функция для включения одного светодиода
 Serial.println("данные\n");                                             
 const size_t capacity = 100;            
 DynamicJsonBuffer jsonBuffer(capacity);                                 
 JsonObject& root = jsonBuffer.parseObject(HTTP.arg("plain"));
 uint8_t Single_num =  root["num_sing"].as<uint8_t>();                // извлекаем номер светодиода
 uint8_t s_c_r=  root["col_sing_r"].as<uint8_t>();                    // извлекаем красный
 uint8_t s_c_g=  root["col_sing_g"].as<uint8_t>();                    // извлекаем зеленый
 uint8_t s_c_b=  root["col_sing_b"].as<uint8_t>();                    // извлекаем синий
 uint8_t model= root["mode"].as<uint8_t>();

Serial.println("светодиод: "); 
Serial.println(Single_num);
Serial.println("\n Цвет: ");
Serial.println(s_c_r);
Serial.println(", ");
Serial.println(s_c_g);
Serial.println(", ");
Serial.println(s_c_b);
  
 if(model == 1)
 {
  strip1[Single_num-1] = CRGB(s_c_r,s_c_g,s_c_b);
  FastLED.show();
 }
 else if(model == 2)
 {
  strip2[Single_num-1] = CRGB(s_c_r,s_c_g,s_c_b);
 }                
 FastLED.show();
 jsonBuffer.clear();
 HTTP.send(200, "text/plain", "done!");  
 // очищаем буфер 
 last_dinamic_mode=0; 
}

/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/******************************************************************************************************************/
/*                                                    RAINBOW                                                     */
/******************************************************************************************************************/
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
void my_ledstrip_rainbow() {                                              //функция для включения радуги
 Serial.println("данные\n");                                             
 const size_t capacity = 100;            
 DynamicJsonBuffer jsonBuffer(capacity);                                 // Инициализируем буфер под JSON
 JsonObject& root = jsonBuffer.parseObject(HTTP.arg("plain"));
 uint8_t number_to =  root["num_to"].as<uint8_t>();
 uint8_t number_move =  root["num_move"].as<uint8_t>();
 uint8_t number_quantity =  root["num_quantity"].as<uint8_t>();
 uint8_t model= root["mode"].as<uint8_t>();
 uint8_t brightness= root["brightness"].as<uint8_t>();
 
 Serial.println("Number_to: ");
 Serial.println(number_to); 
 Serial.println("Number_move: ");
 Serial.println(number_move);
 Serial.println("Number_quantity: ");
 Serial.println(number_quantity);


 if(model == 1)
 {
  fill_rainbow (strip1,number_to,number_move, number_quantity);
  
 
 }
 else if(model == 2)
 {
  fill_rainbow (strip2,number_to,number_move, number_quantity);
  
 }    

 
 FastLED.show();
 jsonBuffer.clear(); 
 HTTP.send(200, "text/plain", "done!");   
 last_dinamic_mode=0;                  
}



/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/******************************************************************************************************************/
/*                                                    GRADIENT                                                    */
/******************************************************************************************************************/
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
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

 uint8_t model= root["mode"].as<uint8_t>();
 Serial.println("Start_gradient: ");
 Serial.println(number_start_gradient);
 Serial.println("End_gradient: ");
 Serial.println(number_end_gradient);
 Serial.println("Start_color: ");
 Serial.println(start_color_r);
 Serial.println(start_color_g);
 Serial.println(start_color_b);
 Serial.println("End_color: ");
 Serial.println(end_color_r);
 Serial.println(end_color_g);
 Serial.println(end_color_b);
 jsonBuffer.clear();

 
 if(model == 1)
 {
  
  fill_gradient_RGB(strip1,number_start_gradient-1,CRGB(start_color_r,start_color_g,start_color_b),number_end_gradient-1,CRGB(end_color_r,end_color_g,end_color_b)); 
 
 }
else if(model == 2)
 {
  fill_gradient_RGB(strip2,number_start_gradient-1,CRGB(start_color_r,start_color_g,start_color_b),number_end_gradient-1,CRGB(end_color_r,end_color_g,end_color_b)); 
 }

     

 FastLED.show();
 HTTP.send(200, "text/plain", "done!");  
 last_dinamic_mode=0;  
}

/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/******************************************************************************************************************/
/*                                                    SOLID                                                       */
/******************************************************************************************************************/
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


void my_ledstrip_solid() {                                              //функция для включения заливки
                                
 const size_t capacity = 250;            
 DynamicJsonBuffer jsonBuffer(capacity);                                 // Инициализируем буфер под JSON
 JsonObject& root = jsonBuffer.parseObject(HTTP.arg("plain"));
 uint8_t number_start_solid =  root["number_start_solid"].as<uint8_t>();
 uint8_t color_solid_r = root["color_solid_r"].as<uint8_t>();
 uint8_t color_solid_g = root["color_solid_g"].as<uint8_t>();
 uint8_t color_solid_b = root["color_solid_b"].as<uint8_t>();
 uint8_t num_end_solid = root["number_end_solid"].as<uint8_t>();
 Serial.println("Number_start_solid: ");
 Serial.println(number_start_solid);
 Serial.println("Number_end_solid: ");
 Serial.println(num_end_solid);
 Serial.println(color_solid_r);
 Serial.println(color_solid_g);
 Serial.println(color_solid_b); 
 uint8_t model= root["mode"].as<uint8_t>();
if(model == 1)
 {
  for(number_start_solid;number_start_solid<=num_end_solid;number_start_solid++)
  {
    strip1[number_start_solid-1] = CRGB(color_solid_r,color_solid_g,color_solid_b);
  }
}
else if(model == 2)
 {
   for(number_start_solid;number_start_solid<=num_end_solid;number_start_solid++)
  {
    strip2[number_start_solid-1] = CRGB(color_solid_r,color_solid_g,color_solid_b);
  } 
 }

 FastLED.show();
 jsonBuffer.clear();
  HTTP.send(200, "text/plain", "done!");   
  last_dinamic_mode=0;                     
}
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/******************************************************************************************************************/
/*                                                    OFF                                                       */
/******************************************************************************************************************/
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void my_ledstrip_off()                                              //функция для выключения всей ленты
{

  for(uint8_t i=0;i < led1 ;i++)
  {
  strip1[i] = CRGB(0,0,0);
  }
    for(uint8_t i=0;i < led2 ;i++)
  {
  strip2[i] = CRGB(0,0,0);
  }
  FastLED.show();
  HTTP.send(200, "text/plain", "done!"); 
  last_dinamic_mode=0;    
}
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/******************************************************************************************************************/
/*                                                    POLICE                                                       */
/******************************************************************************************************************/
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
void blink_police1()
{
 
  for(uint8_t i = 0; i<led1/2; i++)
    {
      strip1[i] = CRGB(255,0,0);
    }
    FastLED.show();
   delay(20);
   
   for(uint8_t i = 0; i<led1/2; i++)
    {
      strip1[i] = CRGB(0,0,0);
    }
    FastLED.show();
   delay(20);
   for(uint8_t i = 0; i<led1/2; i++)
    {
      strip1[i] = CRGB(255,0,0);
    }
    FastLED.show();
   delay(20);
   
   for(uint8_t i = 0; i<led1/2; i++)
    {
      strip1[i] = CRGB(0,0,0);
    }
    FastLED.show();
   delay(200);


   for(uint8_t i = led1/2; i<led1; i++)
    {
      strip1[i] = CRGB(0,0,255);
    }
    FastLED.show();
   delay(20);
   
   for(uint8_t i = led1/2; i<led1; i++)
    {
      strip1[i] = CRGB(0,0,255);
    }
    FastLED.show();
   delay(20);
   for(uint8_t i = led1/2; i<led1; i++)
    {
      strip1[i] = CRGB(0,0,255);
    }
    FastLED.show();
   delay(20);
   
   for(uint8_t i = led1/2; i<led1; i++)
    {
      strip1[i] = CRGB(0,0,0);
    }
    FastLED.show();
   HTTP.send(200, "text/plain", "done!"); 
   delay(200);
   last_dinamic_mode=1; 
 }

void blink_police2()
{
 
  for(uint8_t i = 0; i<led2/2; i++)
    {
      strip2[i] = CRGB(255,0,0);
    }
    FastLED.show();
   delay(20);
   
   for(uint8_t i = 0; i<led2/2; i++)
    {
      strip2[i] = CRGB(0,0,0);
    }
    FastLED.show();
   delay(20);
   for(uint8_t i = 0; i<led2/2; i++)
    {
      strip2[i] = CRGB(255,0,0);
    }
    FastLED.show();
   delay(20);
   
   for(uint8_t i = 0; i<led2/2; i++)
    {
      strip2[i] = CRGB(0,0,0);
    }
    FastLED.show();
   delay(200);


   for(uint8_t i = led2/2; i<led2; i++)
    {
      strip2[i] = CRGB(0,0,255);
    }
    FastLED.show();
   delay(20);
   
   for(uint8_t i = led2/2; i<led2; i++)
    {
      strip2[i] = CRGB(0,0,255);
    }
    FastLED.show();
   delay(20);
   for(uint8_t i = led2/2; i<led2; i++)
    {
      strip2[i] = CRGB(0,0,255);
    }
    FastLED.show();
   delay(20);
   
   for(uint8_t i = led2/2; i<led2; i++)
    {
      strip2[i] = CRGB(0,0,0);
    }
    FastLED.show();
   HTTP.send(200, "text/plain", "done!"); 
   delay(200);
   last_dinamic_mode=3; 
 }
 /*////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/******************************************************************************************************************/
/*                                                    RAINBOW WAVE                                                       */
/******************************************************************************************************************/
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
void rainbow_wave_data()
{
 Serial.println("RAINBOW WAVE MODE\n");                                             
 const size_t capacity = 100;            
 DynamicJsonBuffer jsonBuffer(capacity);                                 // Инициализируем буфер под JSON
 JsonObject& root = jsonBuffer.parseObject(HTTP.arg("plain"));
 speedl  =  root["speed1"].as<uint8_t>();
 Serial.println("Speed: ");
 Serial.println(speedl);
 jsonBuffer.clear();
 rainbow_wave();
}
void rainbow_wave()
{ 
  if (millis() - timing > 10){ // Вместо 10000 подставьте нужное вам значение паузы 
  timing = millis(); 
  gHue++; 
  fill_rainbow (strip1,led1,gHue, 8);
  FastLED.show();
  last_dinamic_mode=2;
  HTTP.send(200, "text/plain", "done!");
  }
}


/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/******************************************************************************************************************/
/*                                                    Server                                                       */
/******************************************************************************************************************/
/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

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
