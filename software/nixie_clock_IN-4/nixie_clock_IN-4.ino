#include <iarduino_RTC.h>

#define ON_TIME 2200        //время горения одного индикатора, микросекунды 
#define RESTORE_TIME 220    //время горения индикатора во время обхода в режиме антиотравления, микросекунды
#define PERIOD_TIME 3000    //период одного индикатора

#define NIGHT_START 22      //часы начала ночного режима
#define NIGHT_END 7         //окончание ночного режима

#define SET_TIME_PIN 10     //номер пина кнопки установки времени
#define SET_MINUTES_PIN 11  //номер пина кнопки увеличения минут
#define SET_HOUR_PIN 12     //номер пина кнопки увеличения часов

const byte numberPins[] = {2, 3, 4, 5};
const byte anodePins[] = {6, 7, 8, 9};

iarduino_RTC time(RTC_DS1307);    

int onTime = ON_TIME; 
byte currentHour = 0;
byte currentMinutes = 0;
byte numbersToDisplay[4];   //0 - десятки часов, 1 - единица часов, 2 - десятки минут, 3 - единицы минут
bool setTimeFlag = false, setHourFlag = false, setMinutesFlag = false, restoreFlag = false;

void restoreIndicators();   //функция, реализующая антиотравление катодов индикаторов
void getNumbersToDisplay();
void displayNumbers();

void setup() 
{
  delay(300);
  time.begin();
  currentHour = 0;
  currentMinutes = 0;
  for (byte i = 2; i < 10; i++)
  {
    pinMode(i, OUTPUT);
  }
  for (byte i = 10; i < 13; i++)
  {
    pinMode(i, INPUT);
  }
}

void loop() 
{
  time.gettime();
  currentHour = time.Hours;
  currentMinutes = time.minutes;
  
  if (currentMinutes == 0 && restoreFlag)
  {
    restoreIndicators();
    restoreFlag = !restoreFlag;
  }
  else if (currentMinutes == 30 && !restoreFlag)
  {
    restoreIndicators();
    restoreFlag = !restoreFlag;
  }
  if ( (currentHour >= NIGHT_START && currentHour <= 23) || (currentHour >= 0 && currentHour < NIGHT_END) )
  {
    onTime = ON_TIME / 5;
  }
  else
  {
    onTime = ON_TIME;
  }
  
  
  setTimeFlag = digitalRead(SET_TIME_PIN);
  
  if (setTimeFlag)
  {
    setMinutesFlag = digitalRead(SET_MINUTES_PIN);
    if (setMinutesFlag)
    {
      if (++currentMinutes > 59)
      {
        currentMinutes = 0;
      }
    }
    setHourFlag = digitalRead(SET_HOUR_PIN);
    if (setHourFlag)
    {
      if (++currentHour > 23)
      {
        currentHour = 0;
      }
    }
    time.settime(0, currentMinutes, currentHour);
  }
  
  getNumbersToDisplay();
  displayNumbers();
}

/*----------------ФУНКЦИИ----------------
 ----------------------------------------*/

void restoreIndicators()
{
  for (byte anode : anodePins)
  {
    for (byte i = 0, number; i < 10; i++)
    {
     number = i;
     for (byte numberPin : numberPins)
     {
      digitalWrite(numberPin, number & 1);
      number >>= 1;
     }
     digitalWrite(anode, HIGH);
     delayMicroseconds(RESTORE_TIME);
     digitalWrite(anode, LOW);
    }
  }
}

void getNumbersToDisplay()
{
  numbersToDisplay[0] = currentHour / 10;
  numbersToDisplay[1] = currentHour % 10;
  numbersToDisplay[2] = currentMinutes / 10;
  numbersToDisplay[3] = currentMinutes % 10;
}

void displayNumbers()
{
  for (byte i = 0, number; i < 4; i++)
  {
    number = numbersToDisplay[i];
    for (byte numberPin : numberPins)
    {
      digitalWrite(numberPin, number & 1);
      number >>= 1;
    }
    digitalWrite(anodePins[i], HIGH);
    delayMicroseconds(onTime);
    digitalWrite(anodePins[i], LOW);
    delayMicroseconds(PERIOD_TIME - onTime);
  }
}
