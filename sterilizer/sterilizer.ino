
// Attach a rotary encoder with output pins to A2 and A3.
// The common contact should be attached to ground.

/*
  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
*/

// include the library code:
#include <LiquidCrystal.h>
#include <RotaryEncoder.h>

#define ozone_pin 6
#define ultraviolet_pin 8
#define fan_pwm_pin 9
#define key_pressed 3
#define key_holded 4
#define backlight_lcd 10
uint8_t key_data;
uint32_t ozone_millis = 0;
uint32_t work_millis = 0;
uint32_t screen_millis = 0;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
//const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
//LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Setup a RoraryEncoder for pins A2 and A3:
RotaryEncoder encoder(A2, A3);

void setup()
{
pinMode(ozone_pin, OUTPUT); // озон (релейный модуль, активный низкий)
pinMode(ultraviolet_pin, OUTPUT); // ультрафиолет (on/off) 
pinMode(fan_pwm_pin, OUTPUT); // вентилятор (ШИМ)
pinMode(backlight_lcd, OUTPUT);
pinMode(LED_BUILTIN, OUTPUT); // светодиод
digitalWrite(ozone_pin,HIGH);
digitalWrite(backlight_lcd, HIGH);
digitalWrite(ultraviolet_pin,LOW);

//digitalWrite(fan_pwm_pin, LOW);
digitalWrite(LED_BUILTIN,LOW);
// set up the LCD's number of columns and rows:
lcd.begin(16, 2);
lcd.clear(); // Чистим экран
// Print a message to the LCD.
//lcd.print("hello, world!");
}

// Read the current position of the encoder and print out when changed.
void loop()
{
  static int pos = 0;
  encoder.tick();
  key_data = get_key();  // вызываем функцию определения нажатия кнопок, присваивая возвращаемое ней значение переменной, которую далее будем использовать в коде
  int newPos = encoder.getPosition();
  if (pos != newPos) 
  {
//    Serial.print(newPos);
//    Serial.println();
    pos = newPos;
  } 
  if (key_data == key_holded) // если значение, возвращённое функцией, отлично от нуля
{
digitalWrite(ozone_pin,LOW);
digitalWrite(ultraviolet_pin,HIGH);
//digitalWrite(fan_pwm_pin, LOW);
digitalWrite(LED_BUILTIN,HIGH);
analogWrite(9, 75); //0-255
work_millis = millis();
ozone_millis = millis();
key_data = 0; //обнуление
}

  if (key_data == key_pressed) // если значение, возвращённое функцией, отлично от нуля
{
digitalWrite(ozone_pin,HIGH);
digitalWrite(ultraviolet_pin,LOW);
//digitalWrite(fan_pwm_pin, LOW);
digitalWrite(LED_BUILTIN,LOW);
analogWrite(9, 0); //0-255
work_millis = 0;
ozone_millis = 0;
key_data = 0; //обнуление
}


if ((millis() - screen_millis) > 1000)
{
if (digitalRead(ultraviolet_pin))
{
  lcd.setCursor(4, 0);
  lcd.print(" worked ");  
  lcd.setCursor(0, 1);
  lcd.print(millis() / 60000);  
  if ((!digitalRead(ozone_pin)) && ((millis() - ozone_millis) > 40000))
  {
  digitalWrite(ozone_pin, HIGH);
  ozone_millis = millis();
  }
  else if (digitalRead(ozone_pin) && ((millis() - ozone_millis) > 180000))
  {
  digitalWrite(ozone_pin, LOW);
  ozone_millis = millis();  
  }
 }
 else
 {
  lcd.setCursor(4, 0);
  lcd.print("  wait  "); 
  lcd.setCursor(0, 1);
  lcd.print("                ");
  }
  screen_millis = millis();
}
if ((!digitalRead(ultraviolet_pin)) && (!digitalRead(ozone_pin)))
{
digitalWrite(ozone_pin, HIGH);
}
key_data = 0; //обнуление



  } // loop ()

byte get_key()
{
// версия 1 - для кратковременного нажатия значение возвращается при отпускании кнопки, для длительного - пока кнопка остаётся нажатой, с заданным интервалом
uint8_t trigger_push_hold_counter = 10; // задержка триггера кратковременного/длительного нажатия (проходов функции, умноженных на задержку "milliseconds_between_increment")  
uint8_t milliseconds_between_increment = 50; // интервал в миллисекундах между инкрементом счётчика нажатой кнопки 
static uint8_t  val_ke;
static uint32_t key_delay_millis;
static uint32_t key_delay_after_hold_millis;
if ((millis() - key_delay_millis) > milliseconds_between_increment) //обрабатываем нажатия инкрементом переменной только если после предыдущей обработки прошло не менее "milliseconds_between_increment" миллисекунд
{
  if (!(PIND & (1 << PIND7)))  //нажатие <<<
    {
    val_ke++;  // инкрементируем счётчик
    if (val_ke > trigger_push_hold_counter) // если значение счётчика больше порога детектирования удержания клавиши
        {
         val_ke = 0; // сбрасываем счётчик
         key_delay_after_hold_millis = millis(); // запоминаем время 
         return key_holded; // возвращаем значение
//return 0;        
        }   
    }
  key_delay_millis = millis(); // запоминаем время 
}
if (val_ke > 0) //если клавиша перед этим была нажата 
{
  if ((PIND & (1 << PIND7)) && ((millis() - key_delay_after_hold_millis) > (trigger_push_hold_counter * milliseconds_between_increment))) // если клавиша на данный момент отпущена и с момента последнего удержания любой клавиши прошёл интервал больше, чем один интервал удержания клавиши
  {
  val_ke = 0;  // сбрасываем счётчик  
  return key_pressed; // возвращаем значение
//  return 0;
  }
}
if (PIND & (1 << PIND7)) {val_ke = 0;} // если добрались до этой точки и кнопка не нажата - обнуляем счётчик (защита от появления "pressed" после "holded")
return 0; // если ни одна из кнопок не была нажата - возвращаем 0
}
