int touch;
bool led;
long previousMillis;
long newMillis;
void setup()
{
  pinMode(13,OUTPUT);
  Serial.begin(9600);
   while (!Serial) {
    if (millis()>2000) break; // wait for serial port to connect. Needed for native USB port only
  }
  // pinMode( 4, INPUT_PULLUP); pinMode( 5, INPUT_PULLUP); pinMode( 6, INPUT_PULLUP); pinMode( 7, INPUT_PULLUP); pinMode( 8, INPUT_PULLUP);
 Serial.print("touch");
}

void loop()
{
  previousMillis=micros();

  touch = touchRead(23);
//  Serial.print(touchRead(1));  Serial.print("_");
//  Serial.print(touchRead(3));  Serial.print("_");
//  Serial.print(touchRead(4));  Serial.print("_");

newMillis=micros();
 //Serial.print(newMillis- previousMillis);  Serial.print("_");
  Serial.println(touch);
  delay(10);
  led =!led;
  digitalWrite(13, led);
}
