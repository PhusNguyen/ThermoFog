#include <Thread.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>

#define Temp_PIN  17 // ESP32 pin GIOP17 connected to DS18B20 sensor's DATA pin
#define PWM_fan_PIN 2 // ESP32 pin GPIO2 as PWM wire in fan
#define Ultrasonic_Signal_PIN 18 // ESP32 pin GPIO18 as signal to relay
#define Peltier_Signal_PIN 19 // ESP32 pin GPIO19 as signal to relay

// LCD control button
#define upButton 33
#define downButton 25
#define selectButton 26
#define runButton 32

OneWire oneWire(Temp_PIN);
DallasTemperature DS18B20(&oneWire);
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// Threads
Thread Thread_read_sensor = Thread();
Thread Thread_display_LCD = Thread();
Thread Thread_fan = Thread();
Thread Thread_button = Thread();
Thread Thread_Ultrasonic = Thread();
Thread Thread_Peltier = Thread();

float tempC;
float input_TempC = 20.0;
float temp_range = 2.0;
unsigned int input_fanSpeed = 3;

int menu = 1; // Cursor flag
bool select_state = false;  // state of select button
bool run_state = false;     // state of run button
bool manual_mode = false; // state of manual mode
bool automatic_mode = true; // state of automatic mode

// callback for myThread
void read_sensor(){
  DS18B20.requestTemperatures();       // send the command to get temperatures
  tempC = DS18B20.getTempCByIndex(0);  // read temperature in °C
}

void LCD_display(){
  lcd.setCursor(0,3);
  lcd.print("Temp:");
  lcd.setCursor(6,3);
  lcd.print(tempC,1);
}

void update_menu(){
  switch (menu){
    case 0:
    menu = 1;
    break;
    case 1:
    if (select_state == false){
      lcd.setCursor(0,0);
      lcd.print("");
      lcd.print("-Mode: Automatic");
      lcd.setCursor(0,1);
      lcd.print("");
      lcd.print(" Temp set: ");
      lcd.setCursor(11,1);
      lcd.print("");
      lcd.print(input_TempC,1);
      lcd.setCursor(0,2);
      lcd.print(" Fan Speed: ");
      lcd.setCursor(12,2);
      lcd.print("");
      lcd.print(input_fanSpeed);
    }
    else {
      lcd.setCursor(0,0);
      lcd.print("");
      lcd.print(">Mode: Manual   ");
      lcd.setCursor(0,1);
      lcd.print("");
      lcd.print(" Temp set: ");
      lcd.setCursor(11,1);
      lcd.print("");
      lcd.print(input_TempC,1);
      lcd.setCursor(0,2);
      lcd.print(" Fan Speed: ");
      lcd.setCursor(12,2);
      lcd.print("");
      lcd.print(input_fanSpeed);
    }
    break;
    case 2:
    if (select_state == false){
      lcd.setCursor(0,0);
      lcd.print("");
      lcd.print(" Mode: Automatic");
      lcd.setCursor(0,1);
      lcd.print("");
      lcd.print("-Temp set: ");
      lcd.setCursor(11,1);
      lcd.print("");
      lcd.print(input_TempC,1);
      lcd.setCursor(0,2);
      lcd.print(" Fan Speed: ");
      lcd.setCursor(12,2);
      lcd.print("");
      lcd.print(input_fanSpeed);
    }
    else {
      lcd.setCursor(0,0);
      lcd.print("");
      lcd.print(" Mode: Automatic");
      lcd.setCursor(0,1);
      lcd.print("");
      lcd.print(">Temp set: ");
      lcd.setCursor(11,1);
      lcd.print("");
      lcd.print(input_TempC,1);
      lcd.setCursor(0,2);
      lcd.print(" Fan Speed: ");
      lcd.setCursor(12,2);
      lcd.print("");
      lcd.print(input_fanSpeed);
    }
    break;
    case 3:
    if (select_state == false){
      lcd.setCursor(0,0);
      lcd.print("");
      lcd.print(" Mode: Automatic");
      lcd.setCursor(0,1);
      lcd.print("");
      lcd.print(" Temp set: ");
      lcd.setCursor(11,1);
      lcd.print("");
      lcd.print(input_TempC,1);
      lcd.setCursor(0,2);
      lcd.print("-Fan Speed: ");
      lcd.setCursor(12,2);
      lcd.print("");
      lcd.print(input_fanSpeed);
    }
    else {
      lcd.setCursor(0,0);
      lcd.print("");
      lcd.print(" Mode: Automatic");
      lcd.setCursor(0,1);
      lcd.print("");
      lcd.print(" Temp set: ");
      lcd.setCursor(11,1);
      lcd.print("");
      lcd.print(input_TempC,1);
      lcd.setCursor(0,2);
      lcd.print(">Fan Speed: ");
      lcd.setCursor(12,2);
      lcd.print("");
      lcd.print(input_fanSpeed);
    }
    break;
    case 4:
    menu = 3;
    break;
  }
}

void Button_control(){
  if (!digitalRead(selectButton)){
    select_state = !select_state;
    while (!digitalRead(selectButton));
  }
  
  if (select_state == false){
    manual_mode = false;
    if (!digitalRead(upButton)){
      menu--;
      while (!digitalRead(upButton));
    }
    else if (!digitalRead(downButton)){
      menu++;
      while (!digitalRead(downButton));
    }
  }

  else if (select_state == true){
    if (menu == 1){
    manual_mode = true;
    }
    else if (menu == 2){
      if (!digitalRead(upButton)){
        input_TempC += 0.1;
        while (!digitalRead(upButton));
      }
      else if (!digitalRead(downButton)){
        input_TempC -= 0.1;
        while(!digitalRead(downButton));
      }
    }
    else if (menu == 3 || menu == 4){
      if (!digitalRead(upButton)){
        input_fanSpeed += 1;
        if (input_fanSpeed > 5)
        input_fanSpeed = 5;
        while (!digitalRead(upButton));
      }
      else if (!digitalRead(downButton)){
        input_fanSpeed -= 1;
        if (input_fanSpeed < 1)
        input_fanSpeed = 1;
        while(!digitalRead(downButton));
      }
    }
  }
}

void temp_control(){
  if (tempC >= input_TempC + temp_range && tempC < input_TempC + temp_range + 0.06)
     automatic_mode = true;
  else if (tempC >= input_TempC - temp_range && tempC < input_TempC - temp_range + 0.06)
     automatic_mode = false;
  
}

void peltier_control(){
  if (manual_mode == true){
    digitalWrite(Peltier_Signal_PIN, HIGH);
  }
  else if (manual_mode == false){
    if (automatic_mode == true)
      digitalWrite(Peltier_Signal_PIN, HIGH);
    else if (automatic_mode == false)
      digitalWrite(Peltier_Signal_PIN, LOW);
  }
}

void ultrasonic_control(){
  if (manual_mode == true){
    digitalWrite(Ultrasonic_Signal_PIN, HIGH);
  }
  else if (manual_mode == false){
    if (automatic_mode == true)
      digitalWrite(Ultrasonic_Signal_PIN, LOW);
    else if (automatic_mode == false)
      digitalWrite(Ultrasonic_Signal_PIN, HIGH);
  }
}

void PWM_fan(){
  if (manual_mode == true){
    analogWrite(PWM_fan_PIN, 100);
  }
  else if (manual_mode == false){
    if (automatic_mode == true)
      analogWrite(PWM_fan_PIN, 10);
    else if (automatic_mode == false){
      if (input_fanSpeed == 1)
      analogWrite(PWM_fan_PIN, 10);
      if (input_fanSpeed == 2)
      analogWrite(PWM_fan_PIN, 40);
      if (input_fanSpeed == 3)
      analogWrite(PWM_fan_PIN, 70);
      if (input_fanSpeed == 4)
      analogWrite(PWM_fan_PIN, 100);
      if (input_fanSpeed == 5)
      analogWrite(PWM_fan_PIN, 150);
    }
  }
}

/* ------------------------------------------------------------------------- */
void setup(){
  DS18B20.begin();    // initialize the DS18B20 sensor
  lcd.init();         // initialize lcd
  lcd.backlight();

  // set input/output
  pinMode(PWM_fan_PIN, OUTPUT);
  pinMode(Peltier_Signal_PIN, OUTPUT);
  pinMode(Ultrasonic_Signal_PIN, OUTPUT);
  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);
  pinMode(selectButton, INPUT_PULLUP);
  pinMode(runButton, INPUT_PULLUP);

  //
  Thread_read_sensor.onRun(read_sensor);
  Thread_read_sensor.setInterval(100);
  //
  Thread_button.onRun(Button_control);
  Thread_button.setInterval(100);
  //
  update_menu();
  Thread_display_LCD.onRun(LCD_display);
  Thread_display_LCD.setInterval(100);
  //
  Thread_Peltier.onRun(peltier_control);
  Thread_Peltier.setInterval(1000);
  //
  Thread_Ultrasonic.onRun(ultrasonic_control);
  Thread_Ultrasonic.setInterval(1000);
  // 
  Thread_fan.onRun(PWM_fan);
  Thread_fan.setInterval(5000);
  
}

void loop(){
  // checks if thread should run
  if(Thread_read_sensor.shouldRun())
    Thread_read_sensor.run();
  if(Thread_button.shouldRun())
    Thread_button.run();
  temp_control();
  update_menu();
  if(Thread_display_LCD.shouldRun())
    Thread_display_LCD.run();
  if(Thread_Peltier.shouldRun())
    Thread_Peltier.run();
  if(Thread_Ultrasonic.shouldRun())
    Thread_Ultrasonic.run();
  if(Thread_fan.shouldRun())
    Thread_fan.run();
}
