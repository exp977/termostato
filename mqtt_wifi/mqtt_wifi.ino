/*
    Basic ESP8266 MQTT example
  
    This sketch demonstrates the capabilities of the pubsub library in combination
    with the ESP8266 board/library.
  
    It connects to an MQTT server then:
    - publishes "hello world" to the topic "outTopic" every two seconds
    - subscribes to the topic "inTopic", printing out any messages
      it receives. NB - it assumes the received payloads are strings not binary
    - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
      else switch it off
  
    It will reconnect to the server if the connection is lost using a blocking
    reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
    achieve the same result without blocking the main loop.
  
    To install the ESP8266 board, (using Arduino 1.6.4+):
    - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
         http://arduino.esp8266.com/stable/package_esp8266com_index.json
    - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
    - Select your ESP8266 in "Tools -> Board"
  
  */
  
  #include <ESP8266WiFi.h>
  #include <PubSubClient.h>
  #include <SimpleDHT.h>
  #include <Wire.h>
  #include <SFE_MicroOLED.h>
  
  // Update these with values suitable for your network.
  
  const char* ssid = "MOVISTAR_B72D";
  const char* password = "xm5NW4EmpscJnNoAGwTX";
  const char* mqtt_server = "192.168.1.78";
  
  WiFiClient espClient;
  PubSubClient client(espClient);
  long lastMsg = 0;
  char msg[50];
  
  #define LED_ON    LOW
  #define LED_OFF   HIGH
  #define RELAY_ON  LOW
  #define RELAY_OFF HIGH
  
  int pinDHT22 = D7;
  int pinLED = D4; 
  int pinRelay = D5;
  int heater = RELAY_OFF;
  
  
  SimpleDHT22 dht22(pinDHT22);
  
  // Configurar OLED
  #define PIN_RESET 255
  #define DC_JUMPER 0
  MicroOLED oled(PIN_RESET, DC_JUMPER); 
  
  void setup() {
    // Inicializar OLED
    oled.begin();
    oled.clear(ALL);
    oled.contrast(0);
  
    digitalWrite(pinLED, LED_OFF); 
    digitalWrite(pinRelay, RELAY_OFF);
    pinMode(pinLED, OUTPUT);
    pinMode(pinRelay, OUTPUT);
  ////  Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    
    oled.clear(PAGE);
    oled.setFontType(0);
    oled.setCursor(0,0);
    oled.println("Conectando Wifi...");
    oled.display();
  }
  
  void setup_wifi() {
  
    delay(10);
    // We start by connecting to a WiFi network
  ////  Serial.println();
  ////  Serial.print("Connecting to ");
  ////  Serial.println(ssid);
  
    WiFi.begin(ssid, password);
  
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
  ////    Serial.print(".");
    }
  
  ////  Serial.println("");
  ////  Serial.println("WiFi connected");
  ////  Serial.println("IP address: ");
  ////  Serial.println(WiFi.localIP());
  
    oled.setCursor(0,16);
    oled.println("Wifi connected!");
    oled.display();
  }
  
  void callback(char* topic, byte* payload, unsigned int length) {
  ////  Serial.print("Message arrived [");
  ////  Serial.print(topic);
  ////  Serial.print("] ");
    for (int i = 0; i < length; i++) {
  ////    Serial.print((char)payload[i]);
    }
  ////  Serial.println();
  
    // Switch on the LED if an 1 was received as first character
    oled.setCursor(0,32);
    if ((char)payload[0] == '1') {
      digitalWrite(pinLED, LED_ON); 
      digitalWrite(pinRelay, RELAY_ON);
      heater = RELAY_ON;
      oled.println("Heater ON ");
    } else {
      digitalWrite(pinLED, LED_OFF);
      digitalWrite(pinRelay, RELAY_OFF);
      heater = RELAY_OFF;
      oled.println("Heater OFF");
    }
    oled.display();
    
  }
  
  void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
  ////    Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (client.connect("ESP8266Client")) {
  ////      Serial.println("connected");
        // Once connected, publish an announcement...
        client.publish("outTopic", "hello world");
        // ... and resubscribe
        client.subscribe("inTopic");
      } else {
  ////      Serial.print("failed, rc=");
  ////      Serial.print(client.state());
  ////      Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
  }
  
  void loop() {
    float temperature = 0;
    float humidity = 0;
    int err = SimpleDHTErrSuccess;
  
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
  
    long now = millis();
    if (now - lastMsg > 30000) {
      lastMsg = now;
      
  ////    Serial.println("Lectura temperatura");  
  
      if ((err = dht22.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
  ////      Serial.print("Read DHT22 failed, err="); Serial.println(err);delay(2000);
        
        oled.clear(PAGE);
        oled.setFontType(0);
        oled.setCursor(0,8);
        oled.println("Temp: err");
        oled.setCursor(0,16);
        oled.println("Humi: err");
        oled.setCursor(0,32);
        if ( heater == RELAY_ON ) {
          oled.println("Heater ON ");
        } else {
          oled.println("Heater OFF");
        }            
        oled.display();
        //return;
      } else {
        oled.clear(PAGE);
        oled.setFontType(0);
        oled.setCursor(0,0);
  
        snprintf (msg, 75, "%.1f", temperature);
  ////      Serial.print("Publish message: ");
  ////      Serial.println(msg);
        client.publish("outTopic/temp", msg);
  
        snprintf (msg, 75, "Temp: %.1fC", temperature);
        oled.setCursor(0,8);
        oled.println(msg);
      
        snprintf (msg, 75, "%f", humidity);
  ////      Serial.print("Publish message: ");
  ////      Serial.println(msg);
        client.publish("outTopic/humi", msg);
  
        snprintf (msg, 75, "Humi: %.1f", humidity);
        oled.setCursor(0,16);
        oled.println(msg);
  
        oled.setCursor(0,32);
        if ( heater == RELAY_ON ) {
          oled.println("Heater ON ");
        } else {
          oled.println("Heater OFF");
        }
              
        oled.display();
      }
    }
  }
