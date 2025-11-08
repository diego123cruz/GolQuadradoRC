#include <PS2X_lib.h>  //for v1.6
#include <Servo.h>

// GOL QUADRADO
#define BUZZER_PIN A0
Servo motorTra;
Servo motorDir;
#define farol 3
#define lanterna 4
#define seta_d 7
#define seta_e 8

byte ledStateDir = LOW;
byte ledStateEsq = LOW;
byte seta_state = 0;
byte farol_state = 0;
byte farol_state_last = 0;
unsigned long setaPreviousMillis = 0;
const long setaInterval = 500;

/******************************************************************
 * set pins connected to PS2 controller:
 *   - 1e column: original 
 *   - 2e colmun: Stef?
 * replace pin numbers by the ones you use
 ******************************************************************/
#define PS2_DAT        13
#define PS2_CMD        11
#define PS2_SEL        10
#define PS2_CLK        12

/******************************************************************
 * select modes of PS2 controller:
 *   - pressures = analog reading of push-butttons 
 *   - rumble    = motor rumbling
 * uncomment 1 of the lines for each mode selection
 ******************************************************************/
//#define pressures   true
#define pressures   false
//#define rumble      true
#define rumble      false

PS2X ps2x; // create PS2 Controller Class

//right now, the library does NOT support hot pluggable controllers, meaning 
//you must always either restart your Arduino after you connect the controller, 
//or call config_gamepad(pins) again after connecting the controller.

int error = 0;
byte vibrate = 0;

void setup(){
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  pinMode(farol, OUTPUT);
  pinMode(lanterna, OUTPUT);
  pinMode(seta_d, OUTPUT);
  pinMode(seta_e, OUTPUT);
  motorTra.attach(5); 
  motorDir.attach(6);

  Serial.begin(57600);
  
  delay(300);  //added delay to give wireless ps2 module some time to startup, before configuring it
   
  //CHANGES for v1.6 HERE!!! **************PAY ATTENTION*************
  
  //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
  
  if(error == 0){
    Serial.print("Found Controller, configured successful ");
    Serial.print("pressures = ");
	if (pressures)
	  Serial.println("true ");
	else
	  Serial.println("false");
	Serial.print("rumble = ");
	if (rumble)
	  Serial.println("true)");
	else
	  Serial.println("false");
    Serial.println("Try out all the buttons, X will vibrate the controller, faster as you press harder;");
    Serial.println("holding L1 or R1 will print out the analog stick values.");
    Serial.println("Note: Go to www.billporter.info for updates and to report bugs.");
  }  
  else if(error == 1)
    Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");
   
  else if(error == 2)
    Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");

  else if(error == 3)
    Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
  
//  Serial.print(ps2x.Analog(1), HEX);
}

void loop() {
  /* You must Read Gamepad to get new values and set vibration values
     ps2x.read_gamepad(small motor on/off, larger motor strenght from 0-255)
     if you don't enable the rumble, use ps2x.read_gamepad(); with no values
     You should call this at least once a second
   */  
  if(error == 1) //skip loop if no controller found
    return; 
  
  ps2x.read_gamepad(false, vibrate); //read controller and set large motor to spin at 'vibrate' speed
  
  // BUZINA
  if(ps2x.ButtonPressed(PSB_CROSS)) {
    tone(BUZZER_PIN, 335);
  }
  if(ps2x.ButtonReleased(PSB_CROSS)) {
    noTone(BUZZER_PIN);
  }

  // FAROL ALTO
  if(ps2x.ButtonPressed(PSB_R1)) {
    farol_state_last = farol_state;
    farol_state = 3;
  }
  if(ps2x.ButtonReleased(PSB_R1)) {
    farol_state = farol_state_last;
  }

  // FAROL STATEs
  if(ps2x.ButtonPressed(PSB_START)) {
    if(farol_state == 0) {
      farol_state = 1;
    } else if (farol_state == 1) {
      farol_state = 2;
    } else if (farol_state == 2) {
      farol_state = 3;
    } else if (farol_state == 3) {
      farol_state = 1;
    }
    delay(300);
  }

  if(ps2x.ButtonPressed(PSB_SELECT)) {
    farol_state = 0;
    seta_state = 0;
  }

  switch (farol_state) {
      case 0: // OFF
        digitalWrite(farol, LOW);
        digitalWrite(lanterna, LOW);
        break;
      case 1: // lanterna
        analogWrite(farol, 15);
        digitalWrite(lanterna, HIGH);
        break;
      case 2: // farol
        analogWrite(farol, 80);
        break;
      case 3: // farol alto
        analogWrite(farol, 255);
        break;
      default:
        break;
    }

  // SETAS
  if(ps2x.ButtonPressed(PSB_L1)) {
    seta_state = 1;
  }
  if(ps2x.ButtonPressed(PSB_R2)) {
    seta_state = 2;
  }
  if(ps2x.ButtonPressed(PSB_L2)) {
    seta_state = 3;
  }
  if(ps2x.Button(PSB_L2) && ps2x.Button(PSB_R2)) {
    seta_state = 0;
  }

  // SETA
  unsigned long currentMillis = millis();
  if (currentMillis - setaPreviousMillis >= setaInterval) {
    setaPreviousMillis = currentMillis;

    switch (seta_state) {
      case 0: // OFF
        ledStateDir = LOW;
        ledStateEsq = LOW;
        break;
      case 1: // Pisca
        ledStateDir = !ledStateDir;
        ledStateEsq = ledStateDir;
        break;
      case 2: // Seta Dir
        ledStateDir = !ledStateDir;
        ledStateEsq = LOW;
        break;
      case 3: // Seta Esq
        ledStateDir = LOW;
        ledStateEsq = !ledStateEsq;
        break;
      default:
        break;
    }

    digitalWrite(seta_d, ledStateDir);
    digitalWrite(seta_e, ledStateEsq);
  }

  // DIRECAO
  byte mdir = ps2x.Analog(PSS_RX);
  byte mtra = ps2x.Analog(PSS_LY);
  byte mmdir = map(mdir, 0, 255, 150, 30);
  byte mmtra = map(mtra, 0, 255, 40, 140);
  motorDir.write(mmdir);
  motorTra.write(mmtra);

  //Serial.print("Dir: ");
  //Serial.print(mmdir);
  //Serial.print("  -  Tra: ");
  //Serial.print(mmtra);
  //Serial.println();
  delay(50);  
}
