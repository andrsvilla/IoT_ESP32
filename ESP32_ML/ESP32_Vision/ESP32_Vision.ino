#include "Arduino.h"
#include "esp_camera.h"
#include <WiFi.h>
#include <base64.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE  // Has PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM

#include "camera_pins.h"

const char* ssid = "MATGIL";
const char* password = "claufulv21*";

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  else {
    Serial.printf("Camera init OK");
  }


#if defined(CAMERA_MODEL_M5STACK_WIDE)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(ssid);
  
  classifyImage();
 
}

void loop(){
}

void classifyImage() {

  String response;
  // Capture picture
   camera_fb_t * fb = NULL;
   fb = esp_camera_fb_get();
   
   if(!fb) {
    Serial.println("Camera capture failed");
    return;
   }
   else {
    Serial.println("Camera capture OK");
  }

  size_t size = fb->len;
  String buffer = base64::encode((uint8_t *) fb->buf, fb->len);

  String payload = "{\"inputs\": [{ \"data\": {\"image\": {\"base64\": \"" + buffer + "\"}}}]}";

  buffer = "";
  // Uncomment this if you want to show the payload
  //Serial.println(payload);

  esp_camera_fb_return(fb);
  
  // Generic model
  String model_id = "aaa03c23b3724a16a56b629203edc62c";

  HTTPClient http;
  http.begin("https://api.clarifai.com/v2/models/" + model_id + "/outputs");
  http.addHeader("Content-Type", "application/json");     
  http.addHeader("Authorization", "Key 88ca43b12048449997a67b0c39214073"); 
  int response_code = http.POST(payload);

  if(response_code>0){
      Serial.print(response_code);
      Serial.print("Returned String: ");
      response = http.getString();
      Serial.println(response);
    } else {
      Serial.print("POST Error: ");
      Serial.print(response_code);
      return;
    }


  // Parse the json response: Arduino assistant
    const int jsonSize = JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(20) + 3*JSON_OBJECT_SIZE(1) + 6*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 20*JSON_OBJECT_SIZE(4) + 2*JSON_OBJECT_SIZE(6);
    DynamicJsonDocument doc(jsonSize);
    deserializeJson(doc, response);

    /*
    StaticJsonDocument doc;
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, response);
    // Test if parsing succeeds.
    if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
    }
    */

  //const int jsonSize = JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(20) + 3*JSON_OBJECT_SIZE(1) + 6*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 20*JSON_OBJECT_SIZE(4) + 2*JSON_OBJECT_SIZE(6);
  //DynamicJsonDocument doc(jsonSize);
  //deserializeJson(doc, response);


  for (int i=0; i < 10; i++) {
      const char* name = doc["outputs"][0]["data"]["concepts"][i]["name"];
      const float p = doc["outputs"][0]["data"]["concepts"][i]["value"];
        
      Serial.println("=====================");
      Serial.print("Name:");
      Serial.println(name[i]);
      Serial.print("Prob:");
      Serial.println(p);
      Serial.print("i:");
      Serial.println(i);
  }

  Serial.println("\nSleep....");
  esp_deep_sleep_start();

}
