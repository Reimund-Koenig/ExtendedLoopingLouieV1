/**************************************************************/
/* This class is used to manage the buttons that will be used */
/* while drawing the several screens for the display          */
/* @author Kevin Keßler & Reimund König                       */
/* Date: 01.01.2013                                           */
/**************************************************************/

#include "Button.h"
#include <stdlib.h>

extern uint8_t SmallFont[];
extern uint8_t BigFont[];

//colors
extern int PURPLE[];
extern int BACKGROUND[];
extern int WHITE[];
extern int BLACK[];
extern int YELLOW[];
extern int RED[];
extern int GREEN[];

/**************************************************************/
/*CONSTRUCTORS, COPYASSIGN OPERATOR and DESTRUCTOR BEGIN*/
/**************************************************************/
//Standard Constructor
Button::Button()
       :mButton_id(count++), mX1(0), mY1(0), mX2(0), mY2(0), mStr1(0), mStr1x(0), mStr1y(0), mStr2(0), mStr2x(0), mStr2y(0), mFat(0),
         mStatusx(0), mStatusy(0), mState(0), mStatus_str(0)
        {
          setButtoncolor(BLACK);
          setBordercolor(BLACK);
          setFontcolor(BLACK);
          setTextbgcolor(BLACK);
        }

//Constructor for the simple buttons
Button::Button(int butclr[],  int bordclr[], int fontclr[], int textbgclr[], int x1, int y1, int x2, int y2, char* str1, int str1x, int str1y, bool fat)
       :mButton_id(count++), mX1(x1), mY1(y1), mX2(x2), mY2(y2), mStr1(0), mStr1x(str1x), mStr1y(str1y), mStr2(0), mStr2x(0), mStr2y(0), mFat(fat),
        mStatusx(0), mStatusy(0), mState(false), mStatus_str(0)
        {
          setButtoncolor(butclr);
          setBordercolor(bordclr);
          setFontcolor(fontclr);
          setTextbgcolor(textbgclr);

          if(str1)
          {
              mStr1=new char[strlen(str1)+1];
              strcpy(mStr1, str1);
          }
        }

//Constructor for the on/off buttons
Button::Button(int butclr[],  int bordclr[], int fontclr[], int textbgclr[], int x1, int y1, int x2, int y2, char* str1, int str1x, int str1y, char* str2,
               int str2x, int str2y, bool fat, int statusx, int statusy, bool state)
       :mButton_id(count++), mX1(x1), mY1(y1), mX2(x2), mY2(y2), mStr1(0), mStr1x(str1x), mStr1y(str1y), mStr2(0), mStr2x(str2x), mStr2y(str2y), mFat(fat),
        mStatusx(statusx), mStatusy(statusy), mState(state), mStatus_str(0)
        {
          setButtoncolor(butclr);
          setBordercolor(bordclr);
          setFontcolor(fontclr);
          setTextbgcolor(textbgclr);

          if(str1)
          {
              mStr1=new char[strlen(str1)+1];
              strcpy(mStr1, str1);
          }

          if(str2)
          {
              mStr2=new char[strlen(str2)+1];
              strcpy(mStr2, str2);
          }
          
          setStatus_str();
        }
        
        
//Copyconstructor
Button::Button(const Button& other)
       :mButton_id(other.mButton_id), mX1(other.mX1), mY1(other.mY1), mX2(other.mX2), mY2(other.mY2), mStr1(0), mStr1x(other.mStr1x), mStr1y(other.mStr1y), mStr2(0), 
        mStr2x(other.mStr2x), mStr2y(other.mStr2y), mStatus_str(0), mStatusx(other.mStatusx), mStatusy(other.mStatusy), mFat(other.mFat), mState(other.mState)
        {
          if(other.mButtoncolor)
          {
            mButtoncolor[0] = other.mButtoncolor[0];
            mButtoncolor[1] = other.mButtoncolor[1];
            mButtoncolor[2] = other.mButtoncolor[2];
          }
          
          if(other.mBordercolor)
          {
            mBordercolor[0] = other.mBordercolor[0];
            mBordercolor[1] = other.mBordercolor[1];
            mBordercolor[2] = other.mBordercolor[2];
          }  
          
          if(other.mFontcolor)
          {
            mFontcolor[0] = other.mFontcolor[0];
            mFontcolor[1] = other.mFontcolor[1];
            mFontcolor[2] = other.mFontcolor[2];
          }  
          
          if(other.mTextbgcolor)
          {
            mTextbgcolor[0] = other.mTextbgcolor[0];
            mTextbgcolor[1] = other.mTextbgcolor[1];
            mTextbgcolor[2] = other.mTextbgcolor[2];
          }  
          
          if(other.mStr1)
          {
            mStr1 = new char[strlen(other.mStr1)+1];
            strcpy(mStr1, other.mStr1);
          }
          
          if(other.mStr2)
          {
            mStr2 = new char[strlen(other.mStr2)+1];
            strcpy(mStr2, other.mStr2);
          }
          
          if(other.mStatus_str)
          {
            mStatus_str = new char[strlen(other.mStatus_str)+1];
            strcpy(mStatus_str, other.mStatus_str);
          }
        }
        
//Kopierzuweisungsoperator
const Button& Button::operator=(const Button& other)
{
  if (this == &other) return *this;
  
  mButton_id = other.mButton_id;
  
  if(other.mButtoncolor)
  {
    mButtoncolor[0] = other.mButtoncolor[0];
    mButtoncolor[1] = other.mButtoncolor[1];
    mButtoncolor[2] = other.mButtoncolor[2];
  }
  
  if(other.mBordercolor)
  {
    mBordercolor[0] = other.mBordercolor[0];
    mBordercolor[1] = other.mBordercolor[1];
    mBordercolor[2] = other.mBordercolor[2];
  }  
  
  if(other.mFontcolor)
  {
    mFontcolor[0] = other.mFontcolor[0];
    mFontcolor[1] = other.mFontcolor[1];
    mFontcolor[2] = other.mFontcolor[2];
  }  
  
  if(other.mTextbgcolor)
  {
    mTextbgcolor[0] = other.mTextbgcolor[0];
    mTextbgcolor[1] = other.mTextbgcolor[1];
    mTextbgcolor[2] = other.mTextbgcolor[2];
  }  
  
  mX1 = other.mX1;
  mY1 = other.mY1;
  mX2 = other.mX2;
  mY2 = other.mY2;
  
  if(mStr1)
  {
    delete[] mStr1;
    mStr1 = 0;
  }
  if(other.mStr1)
  {
    mStr1 = new char[strlen(other.mStr1)+1];
    strcpy(mStr1, other.mStr1);
  }
  
  mStr1x = other.mStr1x;
  mStr1y = other.mStr1y;
  
  if(mStr2)
  {
    delete[] mStr2;
    mStr2 = 0;
  }
  if(other.mStr2)
  {
    mStr2 = new char[strlen(other.mStr2)+1];
    strcpy(mStr2, other.mStr2);
  }
  
  mStr2x = other.mStr2x;
  mStr2y = other.mStr2y;
  
  if(mStatus_str)
  {
    delete[] mStatus_str;
    mStatus_str = 0;
  }
  if(other.mStatus_str)
  {
    mStatus_str = new char[strlen(other.mStatus_str)+1];
    strcpy(mStatus_str, other.mStatus_str);
  }

  mStatusx = other.mStatusx;
  mStatusy = other.mStatusy;
  
  mFat = other.mFat;
  mState = other.mState;
}

//Destructor
Button::~Button()
{
  Serial.println("~Destructor");
  
  if(mStr1);
  {
    delete[] mStr1;
    mStr1 = 0;
  }
  
  if(mStr2);
  {
    delete[] mStr2;
    mStr2 = 0;
  }
  
  if(mStatus_str);
  {
    delete[] mStatus_str;
    mStatus_str = 0;
  }
}
/**************************************************************/
/*CONSTRUCTORS, COPYASSIGN OPERATOR and DESTRUCTOR END*/
/**************************************************************/

/**************************************************************/
/*METHODS BEGIN*/
/**************************************************************/

/*
*draws a button at the given display
*/
void Button::draw(UTFT myLCD)
{
  //set font type
  if(mFat)
    myLCD.setFont(BigFont);
  else
    myLCD.setFont(SmallFont);
    
  myLCD.setBackColor(mTextbgcolor[0], mTextbgcolor[1], mTextbgcolor[2]); //set font backgroundcolor
  myLCD.setColor(mButtoncolor[0], mButtoncolor[1], mButtoncolor[2]); //set rectangle color  
  myLCD.fillRoundRect (mX1, mY1, mX2, mY2); //draw rectangle (fillRoundRect(x1, y1, x2, y2); x1 & y1 = start, x2 & y2 = end)
  myLCD.setColor(mBordercolor[0], mBordercolor[1], mBordercolor[2]); //set border color
  myLCD.drawRoundRect (mX1, mY1, mX2, mY2); //draw the border for the filled rect
  myLCD.setColor(mFontcolor[0], mFontcolor[1], mFontcolor[2]); //set font color
  myLCD.print(mStr1, mStr1x, mStr1y); //set and show the button text
}

/*
*draws a on/off button at the given display
*/
void Button::drawOnOff(UTFT myLCD)
{
  //set font type
  if(mFat)
    myLCD.setFont(BigFont);
  else
    myLCD.setFont(SmallFont);
  
  //if button state true => show "on"
  if(mState)
  {
    mStatus_str = "ON";
  }
  else //else make show "off"
  {
    mStatus_str = "OFF";
  }
  
  myLCD.setBackColor(mTextbgcolor[0], mTextbgcolor[1], mTextbgcolor[2]); //set font backgroundcolor
  myLCD.setColor(mButtoncolor[0], mButtoncolor[1], mButtoncolor[2]); //set rectangle color  
  myLCD.fillRoundRect (mX1, mY1, mX2, mY2); //draw rectangle (fillRoundRect(x1, y1, x2, y2); x1 & y1 = start, x2 & y2 = end)
  myLCD.setColor(mBordercolor[0], mBordercolor[1], mBordercolor[2]); //set border color
  myLCD.drawRoundRect (mX1, mY1, mX2, mY2); //draw the border for the filled rect
  myLCD.setColor(mFontcolor[0], mFontcolor[1], mFontcolor[2]); //set font color
  myLCD.print(mStr1, mStr1x, mStr1y); //set and show the button text
  myLCD.print(mStr2, mStr2x, mStr2y); //set and show the button text
  myLCD.print(mStatus_str, mStatusx, mStatusy); //set and show the state text
}

/*
*show a visual feedback when a button is pressed
*/
void Button::hold(UTFT myLCD, UTouch myTouch)
{
  setTextbgcolor(YELLOW);
  setButtoncolor(YELLOW);
  setBordercolor(PURPLE);
  setFontcolor(PURPLE);
  draw(myLCD);
  
  //hold on while touched
  while (myTouch.dataAvailable())
    myTouch.read();
  
  setTextbgcolor(PURPLE);
  setButtoncolor(PURPLE);
  setBordercolor(WHITE);
  setFontcolor(WHITE);
  draw(myLCD);
}

/*
*show a visual feedback when a button switches on/off
*and switch it's state
*/
void Button::holdOnOff(UTFT myLCD, UTouch myTouch)
{
  setTextbgcolor(YELLOW);
  setButtoncolor(YELLOW);
  setBordercolor(PURPLE);
  setFontcolor(PURPLE);
  drawOnOff(myLCD);
  
  //hold on while pressed
  while (myTouch.dataAvailable())
    myTouch.read();
  
  if(!mState) //if button state is false => we have to turn ON
  {
    mState = true;
    setButtoncolor(GREEN);
    setBordercolor(BLACK);
    setFontcolor(BLACK);
    setTextbgcolor(GREEN);
  }
  else //if button state is true => we have to switch OFF
  {
    mState = false;
    setButtoncolor(RED);
    setBordercolor(WHITE);
    setFontcolor(WHITE);
    setTextbgcolor(RED);
  }
  
  drawOnOff(myLCD);
}

//Getters
int Button::getID() {return this->mButton_id;}
bool Button::getState() {return this->mState;}
char* Button::getStatus_str() {return this->mStatus_str;}
int Button::getX1() {return this->mX1;}
int Button::getY1() {return this->mY1;}
int Button::getX2() {return this->mX2;}
int Button::getY2() {return this->mY2;}


//Setters
void Button::setButtoncolor(int clr[])
{
    for(int i=0; i<COLOR_ARRAY_LENGTH; i++)
    {
        this->mButtoncolor[i] = clr[i];
    }
}

void Button::setBordercolor(int clr[])
{
    for(int i=0; i<COLOR_ARRAY_LENGTH; i++)
    {
        this->mBordercolor[i] = clr[i];
    }
}

void Button::setFontcolor(int clr[])
{
    for(int i=0; i<COLOR_ARRAY_LENGTH; i++)
    {
        this->mFontcolor[i] = clr[i];
    }
}

void Button::setTextbgcolor(int clr[])
{
    for(int i=0; i<COLOR_ARRAY_LENGTH; i++)
    {
        this->mTextbgcolor[i] = clr[i];
    }
}

void Button::setStr1(char* str)
{
  mStr1 = str;
}

void Button::setState(bool b)
{
  mState = b;
}

void Button::setStatus_str()
{
  char* status_str;
  
  if(mState)
    status_str="ON";
  else
    status_str="OFF";
    
   mStatus_str=new char[strlen(status_str)+1];
   strcpy(mStatus_str, status_str);
}

/**************************************************************/
/*METHODS END*/
/**************************************************************/

/**************************************************************/
/*HELPER FUNCTIONS BEGIN*/
/**************************************************************/

//because arduino does not support "new" and "delete" operators, we have to implement them
//IMPORTANT: Arduino 1.0 and later implements "new" and "delete" but no "new[]" and no "delete[]"

//if using Arduino 1.0 or latet, comment on "new" and "delete"
void* operator new(size_t size) 
{ 
  return malloc(size); 
}

void operator delete(void* ptr) 
{ 
  free(ptr); 
}

void * operator new[](size_t size) 
{ 
    return malloc(size); 
} 

void operator delete[](void * ptr) 
{ 
    free(ptr); 
}

/**************************************************************/
/*HELPER FUNCTIONS END*/
/**************************************************************/
