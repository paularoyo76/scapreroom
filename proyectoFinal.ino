//interrupcionPIR
#include "driver/gpio.h"
#define timeSeconds 10
#define pista2 10
#define pista3 10

const int motionSensor = 32;

long now; 
long lastTrigger = 0;
int startTimer = 0;
int desbloqueo = 0;
int state = 1;
int C[5];

//MiniReproductor MP3
#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include "HardwareSerial.h"

HardwareSerial mySoftwareSerial(1); 
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);


//Bluetooth
#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;


//PULSADOR
int P1 = 0; int P2 = 0; int P3 = 0; int P4 = 0;
int C1 = 0; int C2 = 0; int C3 = 0; int C4 = 0;

void setup() {
    Serial.begin(115200);
    now = millis();
    pinMode(17, INPUT_PULLUP);
    pinMode(18, INPUT_PULLUP);
    pinMode(19, INPUT_PULLUP);
    pinMode(5,  INPUT_PULLUP);
      
    //Bluetooth
    SerialBT.begin("ESP32test"); //Bluetooth device name
    Serial.println(("The device started, now you can pair it with bluetooth!"));

    //PIR Motion Sensor mode
    pinMode (motionSensor, INPUT_PULLUP);
    attachInterrupt (digitalPinToInterrupt(motionSensor), detectsMovement, RISING);  //indica que se ha detectado movimiento
    Serial.println(("Sensor PIR active"));

    //MP3
    mySoftwareSerial.begin(9600, SERIAL_8N1, 26,25);  // speed, type, RX, TX
  
    Serial.println();
    Serial.println(("DFRobot DFPlayer Mini Demo"));
    myDFPlayer.begin(mySoftwareSerial);
    Serial.println(("DFPlayer Mini online."));
    //----Set volume----
    myDFPlayer.volume(25);
    Serial.println("Standby");
    SerialBT.println("Standby");  //control desde el movil del juego
}

void loop() {
  
  now = millis();
//CONTROL BLUETOOTH -------------------------------------------------------------------------------
   if (SerialBT.available()) {  //Doy posibilidad de controlar el volumen, iniciar el audio y cambiar al siguente
    String inData = " ";
    inData = SerialBT.readStringUntil('\n');
    if(inData.startsWith("+")){
      SerialBT.println(F("up..."));
      myDFPlayer.volumeUp();
    }
    else if (inData.startsWith("-")){
      SerialBT.println(F("down..."));
      myDFPlayer.volumeDown();
    }
    else if (inData.startsWith("n")){
      SerialBT.println(F("next..."));
      myDFPlayer.next();
    }
    if (inData.startsWith("C")){

      for (int i = 0; i < 5 ; i++)
      {
         int index = inData.indexOf(',');
         C[i] = inData.substring(0, index).toInt();
         inData = inData.substring(index + 1);
      }
      C1 = ((int)C[1]);C2 = ((int)C[2]);C3 = ((int)C[3]);C4 = ((int)C[4]);
      
      SerialBT.println(F("Nuevo código: "));
      SerialBT.println(C1, DEC);SerialBT.println(C2, DEC);SerialBT.println(C3, DEC);SerialBT.println(C4, DEC);
   }
   } //if(SerialBT.available())


//MAQUINA DE ESTADOS ------------------------------------------------------------------------------
  
  switch (state){
    case 1:
         //PIR SENSOR
        if(startTimer == 1){
          state = 2;
           SerialBT.println("Detectado");
        }
    break;
    
    case 2: 
        myDFPlayer.play(1);
       
        P1 = digitalRead(17);
        P2 = digitalRead(5);
        P3 = digitalRead(18);
        P4 = digitalRead(19);

        
        if(P1 == C1 && P2 == C2 && P3 == C3 && P4 == C4){
          desbloqueo = 1; 
        }
        
        if(startTimer==1 && (now - lastTrigger > (timeSeconds*1000))){    
         SerialBT.print("CUENTA ATRAS ACABADA, la contraseña es: ");
         SerialBT.print(C1, DEC);SerialBT.print(C2, DEC);SerialBT.print(C3, DEC);SerialBT.print(C4, DEC);
         SerialBT.print(", pero se ha introducido :  ");
         SerialBT.print(P1, DEC);SerialBT.print(P2, DEC);SerialBT.print(P3, DEC);SerialBT.println(P4, DEC);
            
         myDFPlayer.play(3);
          startTimer = 0;
          state = 4;
          
         }
         if(startTimer==1 && desbloqueo==1){    
            SerialBT.println("Se ha adivinado la contraseña :");
            myDFPlayer.play(2);
            SerialBT.print(P1, DEC);SerialBT.print(P2, DEC);SerialBT.print(P3, DEC);SerialBT.println(P4, DEC);
            
            startTimer = 0;
            state = 3;
         }
        
    break;
    
    case 3: 
    
           //Suponemos que la pista 2 dura 10 segundos
          delay (pista2*1000);
          SerialBT.println("Se ha reproducido la siguiente instrucción");
          startTimer = 0;
          state = 1;
          
         
         
    break; 

    case 4: 
    
           //Suponemos que la pista 2 dura 10 segundos
          delay (pista3*1000);
          SerialBT.println("fallo");
          startTimer = 0;
          state = 1;
          
         
         
    break;
    default: break;
  }
}

void detectsMovement() {
  Serial.println("Motion detected");
  //linea para activar mp3 
  startTimer = 1; //comienza cuenta atras, debo decidir de cuanto la hago
  lastTrigger = millis();
}
