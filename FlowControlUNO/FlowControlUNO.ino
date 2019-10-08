#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#define  Buzzer  11

float    Freq,Pulse;
unsigned int TimeS = 0;
unsigned long Rate,SetRate;

//---------------------------------------------------
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {{'1','2','3','A'},
                         {'4','5','6','B'},
                         {'7','8','9','C'},
                         {'*','0','#','D'}
                        };
byte rowPins[ROWS] = {10,9,8,7}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6,5,4,3 }; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//------------ LCD ---------------------------
LiquidCrystal_I2C lcd(0x3F,20,4);// PCF8574A =  0x3F

void Ext_Int0() 
{ 
  Pulse = TCNT1;
  TCNT1 = 0;  
  TimeS = 0;
}
void Init_Timer1(void)
{ 
    TCCR1A=0x00;  // Timer  62500 Hz
    TCCR1B=0x04; 
}    
//-------------------------------------------
void setup(void)
{        
    lcd.begin();
    lcd.setCursor(0,0);lcd.print(" Flow Meter Control "); 
    lcd.setCursor(0,1);lcd.print("                    ");
    lcd.setCursor(0,2);lcd.print("                    ");
    lcd.setCursor(0,3);lcd.print("Rate  =          l/m"); 
    
    pinMode(Buzzer,OUTPUT); digitalWrite(Buzzer,HIGH);
         
    noInterrupts();           // disable all interrupts
    Init_Timer1();   
    attachInterrupt(digitalPinToInterrupt(2), Ext_Int0, FALLING);
    interrupts();             // enable all interrupts
    Beep();  
}
//-----------------------------------------------------              
//-----------------------------------------------------              
//-----------------------------------------------------
void loop(void)
{    delay(100);
     TimeS++;
     if(TimeS < 10)
     { 
       Freq  = 62500/Pulse;
       Freq  = Freq / 7.5;
       Rate  = Freq*10; 
     }else{Rate = 0; }  
     lcd.setCursor(11,3);ShowRate(Rate);            
}
//-----------------------------------------------------------------
void NumToChar(unsigned long Num, char  *Buffer,unsigned char Digit)
{char i;
   for(i=(Digit-1);i>= 0;i--)
    { Buffer[i] =  (Num % 10) + '0';  // 234 , 23 , 2
      Num = Num / 10;
    }  
   for(i=0;i<(Digit-2);i++)
    { if(Buffer[i] == '0'){Buffer[i] =  ' ';}else{i =100;}
                         
    }   
}
/*-------------------------------------------------------------*
*  Show Number On LCD                                           *
*-------------------------------------------------------------*/
void ShowRate(unsigned long Num)
{ char Buf[10];
  NumToChar(Num,Buf,3);  
  lcd.print(Buf[0]);  
  lcd.print(Buf[1]);
  lcd.print(".");
  lcd.print(Buf[2]);   
}
/*-------------------------------------------------------------*
* Read temperature from DS18B20                                *
*-------------------------------------------------------------*/
char BufNum[8];
void  SlideNum(void)
{
  BufNum[6]  = BufNum[5];
  BufNum[5]  = BufNum[4];
  BufNum[4]  = BufNum[3];
  BufNum[3]  = BufNum[2];
  BufNum[2]  = BufNum[1];
  BufNum[1]  = BufNum[0];
}
/*-------------------------------------------------------------*
* Get Number From Keypad                                      *
*-------------------------------------------------------------*/
unsigned long GetNum(int Count,int X,int Y)
{ char Key,i,N;
  int Sum; 
  lcd.blink();
  lcd.setCursor(X,Y); 
  N = 0;
  for(i=0;i<Count;i++){BufNum[i] = ' ';}
  i = 0;
  while(Key != '#')
  { Key = NO_KEY;
    while(Key == NO_KEY){Key = keypad.getKey();}    
    Beep(); delay(300);
    
   if(Key == '*'){for(i=0;i<Count;i++){BufNum[i] = ' ';}
                  lcd.setCursor(X,Y);lcd.print("   ");
                  lcd.setCursor(X,Y);
                  lcd.blink();
                  N = 0;
                 }                  
   if((N < Count)&&(Key >= '0')&&(Key <= '9'))
   { 
     lcd.print(Key);
     BufNum[N] = Key;
     N++;     
     if(N==1){lcd.print(".");}
     if(N==Count){lcd.noBlink();}  
   } 
   
  }
Sum = 0;
for(i=0;i<Count;i++){Sum = (Sum*10) + (BufNum[i]-'0');}
lcd.noCursor();
lcd.noBlink();
return(Sum);  
}
/*-------------------------------------------------------------*
*  Beep  Sound                                                 *
*-------------------------------------------------------------*/
void Beep()
{
  digitalWrite(Buzzer,LOW);delay(50);
  digitalWrite(Buzzer,HIGH);
}

