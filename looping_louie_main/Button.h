/**************************************************************/
/* This class is used to manage the buttons that will be used */
/* while drawing the several screens for the display          */
/* @author Kevin Keßler & Reimund König                       */
/* Date: 01.01.2013                                           */
/**************************************************************/

#ifndef BUTTON_H
#define BUTTON_H

#include <UTFT.h>
#include <UTouch.h>

static const int COLOR_ARRAY_LENGTH = 3;
static int count = 0;

class Button
{
  private:
    //Member declarations
    int mButton_id;
    int mButtoncolor[COLOR_ARRAY_LENGTH];
    int mBordercolor[COLOR_ARRAY_LENGTH];
    int mFontcolor[COLOR_ARRAY_LENGTH];
    int mTextbgcolor[COLOR_ARRAY_LENGTH];
    int mX1, mY1, mX2, mY2; //rectangle coords
    char* mStr1; //button string 1
    int mStr1x, mStr1y; //button string 1 coords
    char* mStr2; //button string 2
    int mStr2x, mStr2y; //button string 2 coords
    char* mStatus_str; //button status string (on/off)
    int mStatusx, mStatusy; //button status string coords
    bool mFat; //if true use bigfont, else smallfont for the strings
    bool mState; //true = button is on, false = off

  public:
    //Constructors
    Button();
    Button(int butclr[],  int bordclr[], int fontclr[], int textbgclr[], int x1, int y1, int x2, int y2, char* str1, int str1x, int str1y, bool fat);
    Button(int butclr[],  int bordclr[], int fontclr[], int textbgclr[], int x1, int y1, int x2, int y2, char* str1, int str1x, int str1y, char* str2, int str2x, int str2y, bool fat, int statusx, int statusy, bool state);
    
    //Copy constructor
    Button(const Button& other);
    
    //Copyassign operator
    const Button& operator=(const Button&);

    //Destructor
    ~Button();
    
    //Methods
    void draw(UTFT myLCD);
    void drawOnOff(UTFT myLCD);
    void hold(UTFT myLCD, UTouch myTouch);
    void holdOnOff(UTFT myLCD, UTouch myTouch);

    //Getters
    int* getButtoncolor();
    int* getBordercolor();
    int* getFontcolor();
    int* getTextbgcolor();
    int getX1();
    int getY1();
    int getX2();
    int getY2();
    char* getStr1();
    char* getStr2();
    int getStr1x();
    int getStr2x();
    int getStr1y();
    int getStr2y();
    char* getStatus_str();
    int getStatusx();
    int getStatusy();
    bool getFat();
    bool getState();
    int getID();

    //Setters
    void setButtoncolor(int clr[]);
    void setBordercolor(int clr[]);
    void setFontcolor(int clr[]);
    void setTextbgcolor(int clr[]);
    void setX1(int i);
    void setY1(int i);
    void setX2(int i);
    void setY2(int i);
    void setStr1(char* str);
    void setStr2(char* str);
    void setStr1x(int i);
    void setStr2x(int i);
    void setStr1y(int i);
    void setStr2y(int i);
    void setStatus_str();
    void setStatusx(int i);
    void setStatusy(int i);
    void setFat(bool b);
    void setState(bool b);


};

#endif // BUTTON_H
