/**************************************************************/
/* This is the heart of the Looping Louie Application         */
/* It contains:                                               */
/* - setup() and loop() functions for the Arduino             */
/* - PIN management                                           */
/* - display management  (Visual and Touch)                   */
/* - engine management                                        */
/* - LED management                                           */
/* @author Kevin Keßler & Reimund König                       */
/* @IDE: Arduino 0.0.23                                       */
/**************************************************************/

// include necessary files
#include <UTFT.h>   //for drawing (therefore the extern library UTFT is needed)
#include <UTouch.h> //for touching (therefore the extern library UTOUCH is needed)
#include <I2C.h>    // for the I²C BUS
#include <avr/pgmspace.h>
#include "Button.h"

/**************************************************************/
/*CONSTANTS BEGIN*/
/**************************************************************/

// declare the fonts that will be used
extern uint8_t SmallFont[]; // one line = 12px
extern uint8_t BigFont[];

// declare the images that will be used
extern unsigned int louie_splash_image[0x3840];
extern unsigned int header_image[0x3840];

// I²C BUS
#define BUS_INPUT B00100001      // INPUT-ADDRESS for the I²C BUS
#define BUS_OUTPUT B00100000     // OUTPUT-ADDRESS for the I²C BUS
#define LED_BUS_OUTPUT B00100010 // LED-Output-ADDRESS for the I²C BUS

// init the tft and the touch object
static UTFT myLCD(ITDB32S, 38, 39, 40, 41);
static UTouch myTouch(6, 5, 4, 3, 2);

// define the colors that will be used for the display
int PURPLE[] = {110, 84, 151};
int BACKGROUND[] = {96, 159, 78};
int WHITE[] = {255, 255, 255};
int BLACK[] = {0, 0, 0};
int YELLOW[] = {255, 227, 39};
int RED[] = {255, 0, 0};
int GREEN[] = {0, 255, 0};
int GRAY[] = {204, 204, 204};

// Digital Pins
static const int DP_PWM_Engine = 13;
static const int DP_FORWARDS = 11;
static const int DP_BACKWARDS = 9;

// Engine Speed Levels
const int TURBO_SPEED = 255;
const int SPEED_LEVEL_0 = 0;
const int SPEED_LEVEL_1 = 53;
const int SPEED_LEVEL_2 = 56;
const int SPEED_LEVEL_3 = 60;
const int SPEED_LEVEL_4 = 65;
const int SPEED_LEVEL_5 = 70;
const int SPEED_LEVEL_6 = 80;
const int SPEED_LEVEL_7 = 90;
const int SPEED_LEVEL_8 = 100;
const int SPEED_LEVEL_9 = 110;
const int AMOUNT_OF_SPEED_LEVELS = 10;
int speed_levels[AMOUNT_OF_SPEED_LEVELS] = {
    SPEED_LEVEL_0,
    SPEED_LEVEL_1, SPEED_LEVEL_2, SPEED_LEVEL_3,
    SPEED_LEVEL_4, SPEED_LEVEL_5, SPEED_LEVEL_6,
    SPEED_LEVEL_7, SPEED_LEVEL_8, SPEED_LEVEL_9};

// engine speed tracking
static const int MIN_SPEED = 0;
static const int DEFAULT_SPEED = 4;
static const int MAX_SPEED = 9;
static int speed_level = DEFAULT_SPEED;   // the level for manualy adjusting speed
static int current_speed = DEFAULT_SPEED; // here the current speed will be always stored, even after random changes

// TIMERS in ms
static const int LOOP_DELAY = 500;   // time that has to pass until the next loop will enter the game mode functions
static unsigned long timer_loop = 0; // will be set to the current millis() when the loop enters game mode functions

static const int RANDOM_SPEED_TIME = 3000;   // time that has to pass until the next random value will be assigned
static unsigned long timer_random_speed = 0; // will be set to the current millis() when random value is assigned

static const int BREAKING_TIME = 250;        // time that has to pass until the engine will move again after a stop
static unsigned long timer_engine_break = 0; // will be set to the current millis() when break occurs

static const int BACKWARDS_TIME = 1000;   // time that has to pass until the engine will move again after a stop
static unsigned long timer_backwards = 0; // will be set to the current millis() when break occurs

static const int REFRESH_LOG_TIME = 500;    // time that has to pass until the log will refresh again after last refresh
static unsigned long timer_refresh_log = 0; // will be set to the current millis() when log is refreshed

// blink times
static const int BLINK_SLOW_TIME = 1000;    // time that has to pass until the led status will change in slow blink mode
static const int BLINK_FAST_TIME = 250;     // time that has to pass until the led status will change in fast blink mode
static const int BLINK_CIRCLE_TIME = 350;   // time that has to pass until the led colors move forward in circle mode
static const int BLINK_CROSSING_TIME = 750; // time that has to pass until the led colors move forward in crossing mode
static const int BLINK_INNER_TIME = 500;    // time that has to pass until the led colors switch in inner blink mode
static const int BLINK_RANDOM_TIME = 420;   // time that has to pass until the led colors switch in random blink mode
static unsigned long timer_blink = 0;       // used to time the blink events

// probability management
static const int MAX_PROB = 30;
static const int MIN_PROB = 5;
static int stopping_probability = 10;  // probability in % to pause engine while in stopping mode. between MINPROB and MAXPROB
static int backwards_probability = 10; // probability in % to fly backwards while in backwards mode. between MINPROB and MAXPROB

// flags
static bool direction_forwards = true; // false if backwards
static bool engine_running = true;     // false if stopped

// screen tracking
static const String SPLASH_SCREEN = "SPLASH";
static const String MENU_SCREEN = "MENU";
static const String GAME_SETTINGS_SCREEN = "GAME_SETTINGS";
static const String LED_SETTINGS_SCREEN = "LED_SETTINGS";
static const String LOG_SCREEN = "LOG";
static String current_screen;

// led managing
static const int BLINK_MODE_COUNT = 9;
static char *blink_mode[BLINK_MODE_COUNT] = {"Blue Only", "Inner Only", "Outer & Blue", "Slow Blink", "Fast Blink", "Circle", "Crossing", "Inner Blink", "Random Blink"};
static const int MIN_BLINK = 0;
static const int MAX_BLINK = BLINK_MODE_COUNT - 1;
static int blink_value = MIN_BLINK;
static const byte LED_OFF = B11111111;      // all leds off
static const byte LED_ON = B00000000;       // all leds on
static const byte LED_YELLOW = B11111101;   // outer yellow leds
static const byte LED_PURPLE = B11111011;   // outer purple leds
static const byte LED_GREEN = B11110111;    // outer green leds
static const byte LED_INNER_GP = B11101111; // inner green and purple
static const byte LED_INNER_RY = B11011111; // inner red and yellow
static const byte LED_MIDDLE = B10111111;   // middle blue led
static const byte LED_RED = B11111110;      // outer red leds
byte ledOutputStream = LED_OFF;             // Output Stream for the colored LED's

// turbo managing
static byte TurboOutputStream = B11111111;              // Output Stream for the Turbo LED's
static const unsigned int TURBO_BOOST_COOLDOWN = 42000; // cooldown for each player until boost is ready again
static const unsigned int TURBO_DURATION = 400;         // boost duration in ms
static unsigned long turbo_time_player1 = 0;
static unsigned long turbo_time_player2 = 0;
static unsigned long turbo_time_player3 = 0;
static unsigned long turbo_time_player4 = 0;
static unsigned long turbo_time_player5 = 0;
static unsigned long turbo_time_player6 = 0;
static unsigned long turbo_time_player7 = 0;
static unsigned long turbo_time_player8 = 0;
boolean input_received = false; // Boolean to see when an input arrives

// log managing
static const String EMPTY_LINE = "                              ";
static const int LOG_LINE_WIDTH = 30;                                          // 30 signs
static const int LOG_LINE_HEIGHT = 12;                                         // 12 px
static const int LOG_Y_MIN = 84;                                               // the first position on the screen where entries are allowed
static const int LOG_Y_MAX = 252;                                              // the last position on the screen where entries are allowed
static const int LOG_ARRAY_LENGTH = (LOG_Y_MAX - LOG_Y_MIN) / LOG_LINE_HEIGHT; // calc the max. amount of possible log entrys
static String log_array[LOG_ARRAY_LENGTH];                                     // the array that stores the log strings
static int log_entry_count = 0;                                                // how much log entries are currently in the array
static int log_y_current = LOG_Y_MIN;                                          // the current y value, where the logentry will be drawn
/**************************************************************/
/*CONSTANTS END*/
/**************************************************************/

/**************************************************************/
/*BUTTONS BEGIN*/
/**************************************************************/
// init the simple buttons
static Button b_startstop(PURPLE, WHITE, WHITE, PURPLE, 10, 96, 230, 136, "Start Engine", CENTER, 108, true);
static Button b_gamesettings(PURPLE, WHITE, WHITE, PURPLE, 10, 146, 230, 186, "Game Settings", CENTER, 158, true);
static Button b_ledsettings(PURPLE, WHITE, WHITE, PURPLE, 10, 196, 230, 236, "LED Settings", CENTER, 208, true);
static Button b_log(PURPLE, WHITE, WHITE, PURPLE, 10, 246, 230, 286, "Log", CENTER, 258, true);

static Button b_minus(PURPLE, WHITE, WHITE, PURPLE, 10, 90, 50, 130, "-", 23, 102, true);
static Button b_speedval(PURPLE, WHITE, WHITE, PURPLE, 55, 90, 105, 130, IntToChar(speed_level), 73, 102, true);
static Button b_plus(PURPLE, WHITE, WHITE, PURPLE, 110, 90, 150, 130, "+", 123, 102, true);

static Button b_minus_backwards(GRAY, WHITE, WHITE, GRAY, 10, 166, 50, 206, "-", 23, 178, true);
static Button b_probability_backwards(GRAY, WHITE, WHITE, GRAY, 55, 166, 105, 206, IntToPercent(backwards_probability), 57, 178, true);
static Button b_plus_backwards(GRAY, WHITE, WHITE, GRAY, 110, 166, 150, 206, "+", 123, 178, true);

static Button b_minus_stopping(GRAY, WHITE, WHITE, GRAY, 10, 218, 50, 258, "-", 23, 230, true);
static Button b_probability_stopping(GRAY, WHITE, WHITE, GRAY, 55, 218, 105, 258, IntToPercent(stopping_probability), 57, 230, true);
static Button b_plus_stopping(GRAY, WHITE, WHITE, GRAY, 110, 218, 150, 258, "+", 123, 230, true);

static Button b_ledminus(GRAY, WHITE, WHITE, GRAY, 10, 196, 50, 236, "<", 23, 208, true);
static Button b_ledspeedval(GRAY, WHITE, WHITE, GRAY, 60, 196, 180, 236, blink_mode[MIN_BLINK], CENTER, 210, false);
static Button b_ledplus(GRAY, WHITE, WHITE, GRAY, 190, 196, 230, 236, ">", 203, 208, true);

static Button b_back(PURPLE, WHITE, WHITE, PURPLE, 10, 270, 100, 310, "Back", 22, 282, true);
static Button b_clear(PURPLE, WHITE, WHITE, PURPLE, 140, 270, 230, 310, "Clear", 146, 282, true);

// init the on/off buttons
static Button boo_randomspeed(RED, WHITE, WHITE, RED, 160, 90, 230, 130, "Random", 165, 92, "Speed", 165, 104, false, 205, 118, false);
static Button boo_backwards(RED, WHITE, WHITE, RED, 160, 166, 230, 206, "Random", 165, 168, "Backw.", 165, 180, false, 205, 192, false);
static Button boo_stopping(RED, WHITE, WHITE, RED, 160, 218, 230, 258, "Random", 165, 220, "Stops.", 165, 232, false, 205, 244, false);
static Button boo_turbo(RED, WHITE, WHITE, RED, 160, 270, 230, 310, "Turbo", 165, 272, "Mode", 167, 284, false, 205, 296, false);
static Button boo_ledonoff(RED, WHITE, WHITE, RED, 10, 96, 230, 136, "LEDs", CENTER, 108, " ", 30, 108, true, 180, 115, false);
static Button boo_ledblink(GRAY, WHITE, WHITE, GRAY, 10, 146, 230, 186, "Modes", CENTER, 158, " ", 30, 158, true, 180, 165, false);

/*
 *create button arrays for each screen. these arrays contain all buttons, that are relevant for the specified screen
 */
static const int MenuButtonArrayLength = 4;
static Button MenuButtonArray[MenuButtonArrayLength] = {b_startstop, b_gamesettings, b_ledsettings, b_log};

static const int GameSettingsButtonArrayLength = 11;
static Button GameSettingsButtonArray[GameSettingsButtonArrayLength] = {b_minus, b_plus, b_back, boo_randomspeed, boo_backwards, b_minus_backwards, b_plus_backwards, boo_stopping, b_minus_stopping, b_plus_stopping, boo_turbo};

static const int LedSettingsButtonArrayLength = 5;
static Button LedSettingsButtonArray[LedSettingsButtonArrayLength] = {b_ledminus, b_ledplus, b_back, boo_ledonoff, boo_ledblink};

static const int LogButtonArrayLength = 2;
static Button LogButtonArray[LogButtonArrayLength] = {b_back, b_clear};
/**************************************************************/
/*BUTTONS END*/
/**************************************************************/

/**************************************************************/
/*SETUP() and LOOP() BEGIN*/
/**************************************************************/
/**
 *the setup() function is only called once
 *when the arduino starts. it is used to initialise some
 *components and to show the splash screen
 **/
void setup()
{
  // initialize monitoring
  Serial.begin(9600);

  // init the i2c bus
  I2c.begin();
  I2c.write(BUS_OUTPUT, 0x02, 0x00);     // configure device for continuous mode
  I2c.write(LED_BUS_OUTPUT, 0x02, 0x00); // configure device for continuous mode
  I2c.timeOut(500);                      // maximum waiting time while hanging in the bus in ms

  // define Output Pins
  pinMode(DP_PWM_Engine, OUTPUT);
  pinMode(DP_BACKWARDS, OUTPUT);
  pinMode(DP_FORWARDS, OUTPUT);

  // init the display
  myLCD.InitLCD(PORTRAIT);
  myLCD.clrScr();

  // init the touch function
  myTouch.InitTouch(PORTRAIT);
  myTouch.setPrecision(PREC_MEDIUM);

  // set all LEDs to off
  setLight(LED_OFF);
  setTurboAvailability(LED_OFF);

  // init log array
  initLogArray();

  // show splash screen
  drawSplash();

  // draw Main Menu
  drawMenu();
} // END SETUP()

/**
 *the loop() function is acting like a endless loop.
 *it runs again and again until the arduino is turned off
 *from here any interaction will be managed
 **/
void loop()
{
  // create own "loop" because its faster and more reliable than the loop() function.
  while (true)
  {

    // check if screen is touched and forward touch coords
    if (myTouch.dataAvailable())
    {
      myTouch.read();
      int x = myTouch.getX();
      int y = y = myTouch.getY();

      forwardTouch(x, y);
    }

    // if LEDs are activated, check if a blinking mode was selected
    if (boo_ledonoff.getState())
    {
      // if blink mode is off, just light the leds
      if (!boo_ledblink.getState())
      {
        ledOutputStream = LED_ON; // set all LEDs on
      }
      else // else do the selected blinking mode
      {
        Serial.println((String) "Blink Mode Case =" + blink_value);
        switch (blink_value)
        {
        // Blue Only Mode
        case 0:
          ledOutputStream = LED_MIDDLE;
          break;

        // Inner Only Mode
        case 1:
          ledOutputStream = LED_MIDDLE & LED_INNER_RY & LED_INNER_GP;
          break;

        // Outer and Blue Only Mode
        case 2:
          ledOutputStream = LED_RED & LED_GREEN & LED_PURPLE & LED_YELLOW & LED_MIDDLE;
          break;

        // Slow Blink Mode
        case 3:
          blinkModeSlow();
          break;

        // Fast Blink Mode
        case 4:
          blinkModeFast();
          break;

        // Circle Blink Mode
        case 5:
          blinkModeCircle();
          break;

        // Crossing Blink Mode
        case 6:
          blinkModeCrossing();
          break;

        // Inner Blink Mode
        case 7:
          blinkModeInner();
          break;

        // Random Blink Mode
        case 8:
          blinkModeRandom();
          break;

        default:
          Serial.println("Blink Mode Case = default");
        } // END SWITCH
      }   // END ELSE

      setLight(ledOutputStream); // set the LEDs to the given Bitpattern
    }                            // END LED

    /***************GAME MODE HANDLING BEGIN*************************/
    unsigned long last_loop = millis() - timer_loop; // calc the time when the last loop entered the game mode functions

    // only care about the game modes if Engine started && loop delay has expired
    if (b_startstop.getState() && (last_loop > LOOP_DELAY))
    {
      timer_loop = millis();

      if (boo_randomspeed.getState()) // Random speed mode: ON
      {
        unsigned long last_change = millis() - timer_random_speed; // calc the time when the last speed change was

        // change speed level randomly if the last change ran over the random_speed_time value
        if (last_change > RANDOM_SPEED_TIME)
        {
          int random_level = random(MIN_SPEED + 1, MAX_SPEED);
          set_engine_speed_level(random_level);
          timer_random_speed = millis();
        }
      }

      if (boo_stopping.getState()) // stopping mode: ON
      {

        unsigned long last_break = millis() - timer_engine_break; // calc the time when the last break was

        // pause engine with a chance of stopping_probability (in %) and only if the engine is running
        if ((random(1, 100) > 100 - stopping_probability) && engine_running)
        {
          engine_stop();
          addLogEntry((String) "Engine paused (" + BREAKING_TIME + "ms)"); // make log entry
          engine_running = false;
          timer_engine_break = millis();
        }
        else
        {
          // if last_break ran over the breaking time and engine is still stopped, let him fly again
          if ((last_break > BREAKING_TIME) && !engine_running)
          {
            // let him fly in the same direction like before the stop
            if (direction_forwards)
              engine_forward();
            else
              engine_backward();

            engine_running = true;
          }
        }
      }

      if (boo_backwards.getState()) // backwards mode: ON
      {
        // calc the time when the last backwards flight started
        unsigned long last_backw = millis() - timer_backwards;

        // fly backwards with a chance of backwards_probability in %
        if ((random(1, 100) > 100 - backwards_probability) && direction_forwards)
        {
          engine_backward();
          addLogEntry("Engine backwards"); // make log entry
          timer_backwards = millis();
        }
        else
        {
          // let him fly forward when the backwards timer has expired and only if flying backwards
          if (last_backw > BACKWARDS_TIME && !direction_forwards)
          {
            engine_forward();
            addLogEntry("Engine forwards"); // make log entry
          }
        }
      }

      if (boo_turbo.getState()) // turbo mode: ON
      {
        // check if the turbo cooldown for the specified player is ready
        checkTurboCooldown();

        // check if turbo is pressed
        checkTurboPressed();
      }
    }
    /***************GAME MODE HANDLING END*************************/
  }
} // END LOOP()
/**************************************************************/
/*SETUP() and LOOP() END*/
/**************************************************************/

/**************************************************************/
/*Engine Functions BEGIN*/
/**************************************************************/
/*
 * this function stops the engine instant
 */
void engine_stop()
{
  analogWrite(DP_FORWARDS, 255);
  analogWrite(DP_BACKWARDS, 255);
}

/*
 * this function does start the engine
 */
void engine_start()
{
  engine_running = true;
  analogWrite(DP_FORWARDS, 255);
  analogWrite(DP_BACKWARDS, 0);
  set_engine_speed_level(current_speed);
}

/*
 * this function lets the engine move forwards
 */
void engine_forward()
{
  analogWrite(DP_FORWARDS, 255);
  analogWrite(DP_BACKWARDS, 0);
  direction_forwards = true; // set direction state
}

/*
 * this function lets the engine move backwards
 */
void engine_backward()
{
  analogWrite(DP_FORWARDS, 0);
  analogWrite(DP_BACKWARDS, 255);
  direction_forwards = false; // set direction state
}

/*
 * this function sets the engine_speed_level to the given value
 */
void set_engine_speed_level(int value)
{
  analogWrite(DP_PWM_Engine, speed_levels[value]);
  addLogEntry((String) "Engine Speed Level: " + IntToChar(value)); // make log entry
  current_speed = value;
}
/**************************************************************/
/*Engine Functions END*/
/**************************************************************/

/**************************************************************/
/*Turbo Functions END*/
/**************************************************************/
/*
 *this function activates the turbo boost
 */
void activate_turbo()
{
  bool dir = direction_forwards;
  if (!dir)
    engine_forward();

  analogWrite(DP_PWM_Engine, TURBO_SPEED);
  delay(TURBO_DURATION); // the time how long boost will be active.
  analogWrite(DP_PWM_Engine, speed_levels[current_speed]);

  if (!dir)
    engine_backward();
}

/*
 *this function checks if the turbo cooldown for each player is ready or not.
 *if so, it sets his led to on
 */
void checkTurboCooldown()
{
  if (turbo_time_player1 <= millis())
    TurboOutputStream = TurboOutputStream & B11111110;
  if (turbo_time_player2 <= millis())
    TurboOutputStream = TurboOutputStream & B11111101;
  if (turbo_time_player3 <= millis())
    TurboOutputStream = TurboOutputStream & B11111011;
  if (turbo_time_player4 <= millis())
    TurboOutputStream = TurboOutputStream & B11110111;
  if (turbo_time_player5 <= millis())
    TurboOutputStream = TurboOutputStream & B11101111;
  if (turbo_time_player6 <= millis())
    TurboOutputStream = TurboOutputStream & B11011111;
  if (turbo_time_player7 <= millis())
    TurboOutputStream = TurboOutputStream & B10111111;
  if (turbo_time_player8 <= millis())
    TurboOutputStream = TurboOutputStream & B01111111;

  setTurboAvailability(TurboOutputStream);
}

/*
 *checks if the turbo button of a player is pressed.
 *if so, and if available, it activates turbo
 */
void checkTurboPressed()
{
  byte data = turboButtonRead(BUS_INPUT);
  if (!input_received)
  {
    if (bitRead(data, 0))
    {
      if (turbo_time_player1 <= millis())
      {
        addLogEntry("Turbo Boost by Green 1"); // make log entry
        TurboOutputStream = TurboOutputStream | B00000001;
        setTurboAvailability(TurboOutputStream);
        activate_turbo();
        turbo_time_player1 = millis() + TURBO_BOOST_COOLDOWN;

        return;
      }
    }

    if (bitRead(data, 1))
    {
      if (turbo_time_player2 <= millis())
      {
        addLogEntry("Turbo Boost by Red 2"); // make log entry
        TurboOutputStream = TurboOutputStream | B00000010;
        setTurboAvailability(TurboOutputStream);
        activate_turbo();
        turbo_time_player2 = millis() + TURBO_BOOST_COOLDOWN;

        return;
      }
    }

    if (bitRead(data, 2))
    {
      if (turbo_time_player3 <= millis())
      {
        addLogEntry("Turbo Boost by Purple 1"); // make log entry
        TurboOutputStream = TurboOutputStream | B00000100;
        setTurboAvailability(TurboOutputStream);
        activate_turbo();
        turbo_time_player3 = millis() + TURBO_BOOST_COOLDOWN;

        return;
      }
    }

    if (bitRead(data, 3))
    {
      if (turbo_time_player4 <= millis())
      {
        addLogEntry("Turbo Boost by Yellow 2");
        TurboOutputStream = TurboOutputStream | B00001000;
        setTurboAvailability(TurboOutputStream);
        activate_turbo();
        turbo_time_player4 = millis() + TURBO_BOOST_COOLDOWN;

        return;
      }
    }

    if (bitRead(data, 4))
    {
      if (turbo_time_player5 <= millis())
      {
        addLogEntry("Turbo Boost by Yellow 1"); // make log entry
        TurboOutputStream = TurboOutputStream | B00010000;
        setTurboAvailability(TurboOutputStream);
        activate_turbo();
        turbo_time_player5 = millis() + TURBO_BOOST_COOLDOWN;

        return;
      }
    }

    if (bitRead(data, 5))
    {
      if (turbo_time_player6 <= millis())
      {
        addLogEntry("Turbo Boost by Purple 2"); // make log entry
        TurboOutputStream = TurboOutputStream | B00100000;
        setTurboAvailability(TurboOutputStream);
        activate_turbo();
        turbo_time_player6 = millis() + TURBO_BOOST_COOLDOWN;

        return;
      }
    }

    if (bitRead(data, 6))
    {
      if (turbo_time_player7 <= millis())
      {
        addLogEntry("Turbo Boost by Red 1"); // make log entry
        TurboOutputStream = TurboOutputStream | B01000000;
        setTurboAvailability(TurboOutputStream);
        activate_turbo();
        turbo_time_player7 = millis() + TURBO_BOOST_COOLDOWN;

        return;
      }
    }

    if (bitRead(data, 7))
    {
      if (turbo_time_player8 <= millis())
      {
        addLogEntry("Turbo Boost by Green 2"); // make log entry
        TurboOutputStream = TurboOutputStream | B10000000;
        setTurboAvailability(TurboOutputStream);
        activate_turbo();
        turbo_time_player8 = millis() + TURBO_BOOST_COOLDOWN;

        return;
      }
    }
  }
}

// Function for the Availability of the Turbo Boost for each player
void setTurboAvailability(byte os)
{
  I2c.beginTransmission(BUS_OUTPUT);
  I2c.send(os);
  I2c.endTransmission(); // closing the connection
}
// Function for reading the BUS_INPUT
byte turboButtonRead(int adresse)
{
  byte datenByte = 0xff;
  I2c.requestFrom(adresse, 1);
  if (I2c.available())
  {
    datenByte = I2c.receive();
    input_received = false;
  }
  else
  {
    input_received = true;
  }
  return datenByte;
}

/*
 *resets the player turbo timers
 */
void resetTurboTimers()
{
  turbo_time_player1 = 0;
  turbo_time_player2 = 0;
  turbo_time_player3 = 0;
  turbo_time_player4 = 0;
  turbo_time_player5 = 0;
  turbo_time_player6 = 0;
  turbo_time_player7 = 0;
  turbo_time_player8 = 0;
}
/**************************************************************/
/*Turbo Functions END*/
/**************************************************************/

/**************************************************************/
/*SCREEN DRAWING FUNCTIONS BEGIN*/
/**************************************************************/
/*
 *draws the first screen of the application = the splash screen
 *shows a splash image and some text
 */
void drawSplash()
{
  current_screen = SPLASH_SCREEN;

  myLCD.setFont(SmallFont);

  myLCD.fillScr(96, 159, 78);      // Screen Background Color
  myLCD.setColor(255, 255, 255);   // for all draw print and fill commands (e.G. Fontcolor)
  myLCD.setBackColor(96, 159, 78); // background for print commands

  // draw splash image (start x, start y (left upper), img width, img height, data array, scale factor)
  myLCD.drawBitmap(0, 0, 120, 120, louie_splash_image, 2);

  // print(str, x coord, y coord) left=0, center= 9998, right = 9999
  myLCD.print("*** Looping Louie Extended ***", CENTER, 252);
  myLCD.print("MMP Phase II Projekt", CENTER, 264);
  myLCD.print("Reimund Koenig 863751", CENTER, 276);
  myLCD.print("Kevin Kessler 863076", CENTER, 288);
  myLCD.print("Betreuung: Dr. Hubert Zitt", CENTER, 300);

  delay(3000); // wait 3 seconds before proceeding
}

/*
 *draws the menu screen containing 3 buttons
 *"gamesettings", "led settings", "log"
 */
void drawMenu()
{
  // set current screen
  current_screen = MENU_SCREEN;

  // clear the screen
  myLCD.clrScr();

  // set Screen Background Color
  myLCD.fillScr(96, 159, 78);

  // draw header image
  myLCD.drawBitmap(0, 0, 240, 60, header_image);

  // title "menu"
  myLCD.setFont(SmallFont);
  myLCD.setColor(255, 255, 255);    // set border and fontcolor to white
  myLCD.setBackColor(110, 84, 151); // set font background to purple
  myLCD.print("           Main Menu          ", CENTER, 72);

  //"Start / Stop Engine" Button
  b_startstop.draw(myLCD);

  //"Game Settings" Button
  b_gamesettings.draw(myLCD);

  //"LED Settings" Button
  b_ledsettings.draw(myLCD);

  //"Log" Button
  b_log.draw(myLCD);
}

/*
 *draws the game settings screen containing buttons
 *to change the game preferences
 */
void drawGameSettings()
{
  // set current screen
  current_screen = GAME_SETTINGS_SCREEN;

  // clear the screen
  myLCD.clrScr();

  // set Screen Background Color
  myLCD.fillScr(96, 159, 78);

  // draw header image
  myLCD.drawBitmap(0, 0, 240, 60, header_image);

  // title "speed"
  myLCD.setFont(SmallFont);
  myLCD.setColor(255, 255, 255);    // set border and fontcolor to white
  myLCD.setBackColor(110, 84, 151); // set font background to purple
  myLCD.print("        Speed Settings        ", CENTER, 72);

  // draw speed concerning Buttons
  b_minus.draw(myLCD);
  b_speedval.setStr1(IntToChar(speed_level));
  b_speedval.draw(myLCD);
  b_plus.draw(myLCD);
  boo_randomspeed.drawOnOff(myLCD);

  // title "modes"
  myLCD.setFont(SmallFont);
  myLCD.setColor(255, 255, 255);    // set border and fontcolor to white
  myLCD.setBackColor(110, 84, 151); // set font background to purple
  myLCD.print("       Gamemode Settings      ", CENTER, 148);

  // draw the on/off buttons for the game modes
  boo_backwards.drawOnOff(myLCD);
  boo_stopping.drawOnOff(myLCD);
  boo_turbo.drawOnOff(myLCD);

  // draw the backwards probability changing buttons
  b_minus_backwards.draw(myLCD);
  b_probability_backwards.setStr1(IntToPercent(backwards_probability));
  b_probability_backwards.draw(myLCD);
  b_plus_backwards.draw(myLCD);

  // draw the stopping probability changing buttons
  b_minus_stopping.draw(myLCD);
  b_probability_stopping.setStr1(IntToPercent(stopping_probability));
  b_probability_stopping.draw(myLCD);
  b_plus_stopping.draw(myLCD);

  // draw "back" Button
  b_back.draw(myLCD);
}

/*
 *draws the led settings screen containing buttons
 *to change the led preferences
 */
void drawLEDSettings()
{
  // set current screen
  current_screen = LED_SETTINGS_SCREEN;

  // clear the screen
  myLCD.clrScr();

  // set Screen Background Color
  myLCD.fillScr(96, 159, 78);

  // draw header image
  myLCD.drawBitmap(0, 0, 240, 60, header_image);

  // title "LED Settings"
  myLCD.setFont(SmallFont);
  myLCD.setColor(255, 255, 255);    // set border and fontcolor to white
  myLCD.setBackColor(110, 84, 151); // set font background to purple
  myLCD.print("         LED Settings         ", CENTER, 72);

  //"LED On/Off" On/Off Button
  boo_ledonoff.drawOnOff(myLCD);

  //"LED Blink" On/Off Button
  boo_ledblink.drawOnOff(myLCD);

  // LED Blink Speed Buttons
  b_ledminus.draw(myLCD);
  b_ledspeedval.draw(myLCD);
  b_ledplus.draw(myLCD);

  //"back" Button
  b_back.draw(myLCD);
}

/*
 *draws the log screen where the actions are logged and listed as text
 */
void drawLog()
{
  // set current screen
  current_screen = LOG_SCREEN;

  // clear the screen
  myLCD.clrScr();

  // set Screen Background Color
  myLCD.fillScr(96, 159, 78);

  // draw header image
  myLCD.drawBitmap(0, 0, 240, 60, header_image);

  // title "Log"
  myLCD.setFont(SmallFont);
  myLCD.setColor(255, 255, 255);    // set border and fontcolor to white
  myLCD.setBackColor(110, 84, 151); // set font background to purple
  myLCD.print("              Log             ", CENTER, 72);

  //"back" Button
  b_back.draw(myLCD);

  //"clear" Button
  b_clear.draw(myLCD);
}
/**************************************************************/
/*SCREEN DRAWING FUNCTIONS END*/
/**************************************************************/

/**************************************************************/
/*TOUCH HANDLING BEGIN*/
/**************************************************************/

/* forwardTouch(int, int)
 *manages the touches by it's coordinates.
 *checks if the given coordinates match the coordinates of a button.
 *if so, it calls the onTouch() func. of this button.
 */
void forwardTouch(int x, int y)
{

  Button *ptr = 0; // pointer to the first element of a buttonarray
  int length = 0;

  if (current_screen.equals(MENU_SCREEN))
  {
    length = MenuButtonArrayLength;
    ptr = MenuButtonArray;
  }
  else if (current_screen.equals(GAME_SETTINGS_SCREEN))
  {
    length = GameSettingsButtonArrayLength;
    ptr = GameSettingsButtonArray;
  }
  else if (current_screen.equals(LED_SETTINGS_SCREEN))
  {
    length = LedSettingsButtonArrayLength;
    ptr = LedSettingsButtonArray;
  }
  else if (current_screen.equals(LOG_SCREEN))
  {
    length = LogButtonArrayLength;
    ptr = LogButtonArray;
  }

  // iterate the array and check if the touch coords match button coords
  for (int i = 0; i < length; i++)
  {
    int x1 = ptr[i].getX1();
    int y1 = ptr[i].getY1();
    int x2 = ptr[i].getX2();
    int y2 = ptr[i].getY2();

    if ((x >= x1) && (x <= x2) && (y >= y1) && (y <= y2))
    {
      onTouch(ptr[i].getID());
      break;
    }
  }
}

/* onTouch()
 *Handles the specified touch event for the button with the given id
 */
void onTouch(int id)
{
  // the back button appears at 3 screens: game settings, led settings, log
  // it always will redirect to the menu screen
  if (id == b_back.getID())
  {
    b_back.hold(myLCD, myTouch); // visual feedback
    drawMenu();                  // draw the Menu screen
    return;
  }

  // menu screen touch
  if (current_screen.equals(MENU_SCREEN))
  {
    onTouchMenu(id);
  }

  // game settings screen touch
  else if (current_screen.equals(GAME_SETTINGS_SCREEN))
  {
    onTouchGameSettings(id);
  }

  // led settings screen touch
  else if (current_screen.equals(LED_SETTINGS_SCREEN))
  {
    onTouchLedSettings(id);
  }

  // log screen touch
  else if (current_screen.equals(LOG_SCREEN))
  {
    onTouchLog(id);
  }
}

void onTouchMenu(int id)
{
  /***Start/Stop Engine Button***/
  if (id == b_startstop.getID())
  {
    // engine start
    if (!b_startstop.getState())
    {
      // switch buttontext and state
      b_startstop.setState(true);
      b_startstop.setStr1("Stop Engine");

      // start the engine
      engine_start();

      // make log entry
      addLogEntry("Engine started");
    }
    else // engine stop
    {
      // switch buttontext and state
      b_startstop.setState(false);
      b_startstop.setStr1("Start Engine");

      // stop the engine
      engine_stop();

      // make log entry
      addLogEntry("Engine stopped");
    }

    b_startstop.hold(myLCD, myTouch); // visual feedback

    // if turbo mode is active, reset the player timers
    if (boo_turbo.getState())
    {
      resetTurboTimers();

      if (b_startstop.getState())
      {
        // if turbo mode on and engine starts, turn the turbo leds on
        TurboOutputStream = B00000000;
        setTurboAvailability(TurboOutputStream);
      }
      else
      {
        // if turbo mode off and engine stops, turn the turbo leds off
        TurboOutputStream = B11111111;
        setTurboAvailability(TurboOutputStream);
      }
    }

    return;
  }

  /***Gamesettings Button***/
  if (id == b_gamesettings.getID())
  {
    b_gamesettings.hold(myLCD, myTouch); // visual feedback
    drawGameSettings();                  // draw the game settings screen
    return;
  }

  /***Ledsettings Button***/
  if (id == b_ledsettings.getID())
  {
    b_ledsettings.hold(myLCD, myTouch); // visual feedback
    drawLEDSettings();                  // draw the led settings screen
    return;
  }

  /***Log Button***/
  if (id == b_log.getID())
  {
    b_log.hold(myLCD, myTouch);                                      // visual feedback
    drawLog();                                                       // draw the Log screen
    refreshLog();                                                    // print all the entries of the log array
    log_y_current = LOG_Y_MIN + (log_entry_count * LOG_LINE_HEIGHT); // set the position of the current log entry
    return;
  }
}

void onTouchGameSettings(int id)
{
  /***Speed "-" Button***/
  if (id == b_minus.getID())
  {
    // only active if not min speed and if random speed = off
    if ((speed_level > MIN_SPEED) && !boo_randomspeed.getState())
    {
      b_minus.hold(myLCD, myTouch); // visual feedback
      speed_level--;
      b_speedval.setStr1(IntToChar(speed_level)); // set the value text of the speed button
      myLCD.setFont(BigFont);
      myLCD.printNumI(speed_level, 73, 102); // refresh the value text of the speed button

      // set engine speed
      set_engine_speed_level(speed_level);
    }
    return;
  }

  /***Speed "+" Button***/
  if (id == b_plus.getID())
  {
    // only active if not max speed and if random speed = off
    if ((speed_level < MAX_SPEED) && !boo_randomspeed.getState())
    {
      b_plus.hold(myLCD, myTouch); // visual feedback
      speed_level++;
      b_speedval.setStr1(IntToChar(speed_level)); // set the value text of the speed button
      myLCD.setFont(BigFont);
      myLCD.printNumI(speed_level, 73, 102); // refresh the value text of the speed button

      // set engine speed
      set_engine_speed_level(speed_level);
    }
    return;
  }

  /***Randomspeed On/Off Button***/
  if (id == boo_randomspeed.getID())
  {
    // switch color and state of the button
    boo_randomspeed.holdOnOff(myLCD, myTouch);

    if (boo_randomspeed.getState())
    {
      // gray speed buttons to show they are inactive when randomspeed = on
      // set colors
      b_minus.setButtoncolor(GRAY);
      b_minus.setTextbgcolor(GRAY);
      b_speedval.setButtoncolor(GRAY);
      b_speedval.setTextbgcolor(GRAY);
      b_speedval.setStr1("?");
      b_plus.setButtoncolor(GRAY);
      b_plus.setTextbgcolor(GRAY);

      // draw
      b_minus.draw(myLCD);
      b_speedval.draw(myLCD);
      b_plus.draw(myLCD);
    }
    else
    {
      // ungray speed buttons to show they are active when randomspeed = off
      // set colors
      b_minus.setButtoncolor(PURPLE);
      b_minus.setTextbgcolor(PURPLE);
      b_speedval.setButtoncolor(PURPLE);
      b_speedval.setTextbgcolor(PURPLE);
      b_speedval.setStr1(IntToChar(speed_level));
      b_plus.setButtoncolor(PURPLE);
      b_plus.setTextbgcolor(PURPLE);

      // draw
      b_minus.draw(myLCD);
      b_speedval.draw(myLCD);
      b_plus.draw(myLCD);

      // when random speed is turned off, switch engine speed back to the selected speed_level
      set_engine_speed_level(speed_level);
    }

    addLogEntry((String) "Random Engine Speed: " + boo_randomspeed.getStatus_str()); // make log entry

    return;
  }

  /***Backwards On/Off Button***/
  if (id == boo_backwards.getID())
  {
    // switch color and state of the button
    boo_backwards.holdOnOff(myLCD, myTouch);

    if (!boo_backwards.getState())
    {
      // if backwards mode is turned off, let him fly forwards again
      if (b_startstop.getState())
        engine_forward();

      // gray probability buttons to show they are inactive when backwardsmode = off
      // set colors
      b_minus_backwards.setButtoncolor(GRAY);
      b_minus_backwards.setTextbgcolor(GRAY);
      b_probability_backwards.setButtoncolor(GRAY);
      b_probability_backwards.setTextbgcolor(GRAY);
      b_probability_backwards.setStr1(IntToPercent(backwards_probability));
      b_plus_backwards.setButtoncolor(GRAY);
      b_plus_backwards.setTextbgcolor(GRAY);

      // draw
      b_minus_backwards.draw(myLCD);
      b_probability_backwards.draw(myLCD);
      b_plus_backwards.draw(myLCD);
    }
    else
    {
      // ungray probability buttons to show they are active when backwardsmode = on
      // set colors
      b_minus_backwards.setButtoncolor(PURPLE);
      b_minus_backwards.setTextbgcolor(PURPLE);
      b_probability_backwards.setButtoncolor(PURPLE);
      b_probability_backwards.setTextbgcolor(PURPLE);
      b_probability_backwards.setStr1(IntToPercent(backwards_probability));
      b_plus_backwards.setButtoncolor(PURPLE);
      b_plus_backwards.setTextbgcolor(PURPLE);

      // draw
      b_minus_backwards.draw(myLCD);
      b_probability_backwards.draw(myLCD);
      b_plus_backwards.draw(myLCD);
    }

    addLogEntry((String) "Backwards Mode: " + boo_backwards.getStatus_str()); // make log entry
    return;
  }

  /***Backwards Probability "-" Button***/
  if (id == b_minus_backwards.getID())
  {
    // only active if backwards mode is on and probability is >MIN_PROB
    if (boo_backwards.getState() && (backwards_probability > MIN_PROB))
    {
      b_minus_backwards.hold(myLCD, myTouch); // visual feedback
      backwards_probability -= 5;
      b_probability_backwards.setStr1(IntToPercent(backwards_probability));
      b_probability_backwards.draw(myLCD);
    }
  }

  /***Backwards Probability "+" Button***/
  if (id == b_plus_backwards.getID())
  {
    // only active if backwards mode is on and probability is <MAX_PROB
    if (boo_backwards.getState() && (backwards_probability < MAX_PROB))
    {
      b_plus_backwards.hold(myLCD, myTouch); // visual feedback
      backwards_probability += 5;
      b_probability_backwards.setStr1(IntToPercent(backwards_probability));
      b_probability_backwards.draw(myLCD);
    }
  }

  /***Stopping On/Off Button***/
  if (id == boo_stopping.getID())
  {
    // switch color and state of the button
    boo_stopping.holdOnOff(myLCD, myTouch);

    if (boo_stopping.getState())
    {
      // ungray probability buttons to show they are active when stoppingmode = on
      // set colors
      b_minus_stopping.setButtoncolor(PURPLE);
      b_minus_stopping.setTextbgcolor(PURPLE);
      b_probability_stopping.setButtoncolor(PURPLE);
      b_probability_stopping.setTextbgcolor(PURPLE);
      b_probability_stopping.setStr1(IntToPercent(stopping_probability));
      b_plus_stopping.setButtoncolor(PURPLE);
      b_plus_stopping.setTextbgcolor(PURPLE);

      // draw
      b_minus_stopping.draw(myLCD);
      b_probability_stopping.draw(myLCD);
      b_plus_stopping.draw(myLCD);
    }
    else
    {
      // gray probability buttons to show they are inactive when stoppingmode = off
      // set colors
      b_minus_stopping.setButtoncolor(GRAY);
      b_minus_stopping.setTextbgcolor(GRAY);
      b_probability_stopping.setButtoncolor(GRAY);
      b_probability_stopping.setTextbgcolor(GRAY);
      b_probability_stopping.setStr1(IntToPercent(stopping_probability));
      b_plus_stopping.setButtoncolor(GRAY);
      b_plus_stopping.setTextbgcolor(GRAY);

      // draw
      b_minus_stopping.draw(myLCD);
      b_probability_stopping.draw(myLCD);
      b_plus_stopping.draw(myLCD);
    }

    addLogEntry((String) "Stopping Mode: " + boo_stopping.getStatus_str()); // make log entry
    return;
  }

  /***Stopping Probability "-" Button***/
  if (id == b_minus_stopping.getID())
  {
    // only active if stopping mode is on and probability is >MIN_PROB
    if (boo_stopping.getState() && (stopping_probability > MIN_PROB))
    {
      b_minus_stopping.hold(myLCD, myTouch); // visual feedback
      stopping_probability -= 5;
      b_probability_stopping.setStr1(IntToPercent(stopping_probability));
      b_probability_stopping.draw(myLCD);
    }
  }

  /***Stopping Probability "+" Button***/
  if (id == b_plus_stopping.getID())
  {
    // only active if stopping mode is on and probability is <MAX_PROB
    if (boo_stopping.getState() && (stopping_probability < MAX_PROB))
    {
      b_plus_stopping.hold(myLCD, myTouch); // visual feedback
      stopping_probability += 5;
      b_probability_stopping.setStr1(IntToPercent(stopping_probability));
      b_probability_stopping.draw(myLCD);
    }
  }

  /***Turbo On/Off Button***/
  if (id == boo_turbo.getID())
  {
    // switch color and state of the button
    boo_turbo.holdOnOff(myLCD, myTouch);

    // reset the playertimers to 0
    resetTurboTimers();

    if (boo_turbo.getState())
    {
      // if turbo mode on, turn the turbo leds on
      TurboOutputStream = B00000000;
      setTurboAvailability(TurboOutputStream);
    }
    else
    {
      // if turbo mode off, turn the turbo leds off
      TurboOutputStream = B11111111;
      setTurboAvailability(TurboOutputStream);
    }

    addLogEntry((String) "Turbo Mode: " + boo_turbo.getStatus_str()); // make log entry
    return;
  }
}

void onTouchLedSettings(int id)
{
  /***Led On/Off Button***/
  if (id == boo_ledonoff.getID())
  {
    // switch color and state of the button
    boo_ledonoff.holdOnOff(myLCD, myTouch);

    if (!boo_ledonoff.getState())
    {
      // if leds off -> gray the blink button to show it is inactive
      boo_ledblink.setButtoncolor(GRAY);
      boo_ledblink.setTextbgcolor(GRAY);
      boo_ledblink.drawOnOff(myLCD);

      // and turn all leds off
      setLight(B11111111);
    }
    else
    {
      // if leds on -> ungray the blink button to show it is active
      boo_ledblink.setButtoncolor(PURPLE);
      boo_ledblink.setTextbgcolor(PURPLE);
      boo_ledblink.drawOnOff(myLCD);

      // and turn all the leds on
      setLight(B00000000);
    }

    addLogEntry((String) "LEDs: " + boo_ledonoff.getStatus_str()); // make log entry
    return;
  }

  /***Led Blink On/Off Button***/
  if (id == boo_ledblink.getID())
  {
    // only activate if led button is "on"
    if (boo_ledonoff.getState())
    {
      // switch color and state of the button
      boo_ledblink.holdOnOff(myLCD, myTouch);

      // if blink = off -> gray the blink speed buttons to show they are inactive
      if (!boo_ledblink.getState())
      {
        // set colors
        b_ledminus.setButtoncolor(GRAY);
        b_ledminus.setTextbgcolor(GRAY);
        b_ledspeedval.setButtoncolor(GRAY);
        b_ledspeedval.setTextbgcolor(GRAY);
        b_ledplus.setButtoncolor(GRAY);
        b_ledplus.setTextbgcolor(GRAY);

        // draw
        b_ledminus.draw(myLCD);
        b_ledspeedval.draw(myLCD);
        b_ledplus.draw(myLCD);
      }
      else // if blink = on -> ungray the buttons
      {
        // set colors
        b_ledminus.setButtoncolor(PURPLE);
        b_ledminus.setTextbgcolor(PURPLE);
        b_ledspeedval.setButtoncolor(PURPLE);
        b_ledspeedval.setTextbgcolor(PURPLE);
        b_ledplus.setButtoncolor(PURPLE);
        b_ledplus.setTextbgcolor(PURPLE);

        // draw
        b_ledminus.draw(myLCD);
        b_ledspeedval.draw(myLCD);
        b_ledplus.draw(myLCD);
      }

      addLogEntry((String) "LED Blink: " + boo_ledblink.getStatus_str()); // make log entry
    }
    return;
  }

  /***Led "-" Button***/
  if (id == b_ledminus.getID())
  {
    b_ledminus.hold(myLCD, myTouch); // visual feedback

    // if the first blink mode is selected an the user presses "-"
    // switch to the last blink mode to simulate a queue
    if (blink_value <= MIN_BLINK)
      blink_value = MAX_BLINK;
    else
      blink_value--;

    b_ledspeedval.setStr1(blink_mode[blink_value]); // set the value text of the blink mode button
    b_ledspeedval.draw(myLCD);                      // refresh the value text of the speed button

    addLogEntry((String) "Led Blink Mode: " + blink_mode[blink_value]); // make log entry
    return;
  }

  /***Led "+" Button***/
  if (id == b_ledplus.getID())
  {
    b_ledplus.hold(myLCD, myTouch); // visual feedback

    // if the last blink mode is selected an the user presses "+"
    // switch to the first blink mode to simulate a queue
    if (blink_value >= MAX_BLINK)
      blink_value = MIN_BLINK;
    else
      blink_value++;

    b_ledspeedval.setStr1(blink_mode[blink_value]); // set the value text of the blink mode button
    b_ledspeedval.draw(myLCD);                      // refresh the value text of the speed button

    addLogEntry((String) "Led Blink Mode: " + blink_mode[blink_value]); // make log entry
    return;
  }
}

void onTouchLog(int id)
{
  /***Clear Button***/
  if (id == b_clear.getID())
  {
    b_clear.hold(myLCD, myTouch); // visual feedback

    clearLog();                 // clear the log
    addLogEntry("Log cleared"); // make log entry
  }
}

/**************************************************************/
/*TOUCH HANDLING END*/
/**************************************************************/

/**************************************************************/
/*LOG FUNCTIONS BEGIN*/
/**************************************************************/
/*
 *initialises the LogArray with "empty" strings
 */
void initLogArray()
{
  for (int i = 0; i < LOG_ARRAY_LENGTH; i++)
  {
    log_array[i] = EMPTY_LINE;
  }
}

/*
 *Redraws all the log strings on the log screen
 */
void refreshLog()
{
  // draw entries from top
  log_y_current = LOG_Y_MIN;

  // print all the entries of the log array
  for (int i = 0; i < LOG_ARRAY_LENGTH; i++)
  {
    printEntry(log_array[i]);
  }
}

/*clears the log screen*/
void clearLog()
{
  log_entry_count = 0;

  // set empty strings
  for (int i = 0; i < LOG_ARRAY_LENGTH; i++)
  {
    log_array[i] = EMPTY_LINE;
  }

  // draw the empty strings
  refreshLog();

  // reset log_y for the following entries
  log_y_current = LOG_Y_MIN;
}

/*
 *checks the given string and adds it to the log_array.
 *if the log array is full, the array will be cleared
 *and a new log page will be started
 */
void addLogEntry(String str)
{
  int strlen = str.length();

  // check if string length fits to line width
  if (strlen > LOG_LINE_WIDTH)
  {
    str = "ERROR - String too long";
  }

  // if smaller than width, fill it up with spaces, for redrawing purpose
  while (strlen < LOG_LINE_WIDTH)
  {
    str += " ";
    strlen++;
  }

  // if log is full, clear it and start from top again
  if (log_entry_count > LOG_ARRAY_LENGTH)
  {
    clearLog();
  }

  // add the log entry to the array
  log_array[log_entry_count] = str;
  log_entry_count++;

  // print the entry
  printEntry(str);
}

/*prints only the added line to the log screen*/
void printEntry(String str)
{
  // if we are in the log screen, print that line
  if (current_screen.equals(LOG_SCREEN))
  {
    myLCD.setFont(SmallFont);
    myLCD.setColor(255, 255, 255);   // set border and fontcolor to white
    myLCD.setBackColor(96, 159, 78); // set font background to background green

    if ((log_y_current >= LOG_Y_MIN) && (log_y_current < LOG_Y_MAX))
    {
      myLCD.print(str, LEFT, log_y_current);
      log_y_current += LOG_LINE_HEIGHT;
    }
  }
}
/**************************************************************/
/*LOG FUNCTIONS END*/
/**************************************************************/

/**************************************************************/
/*LED MANAGEMENT BEGIN*/
/**************************************************************/
/*
 *  Sets the colored LEDs on/off, depending on the given bit pattern
 */
void setLight(byte os)
{
  I2c.beginTransmission(LED_BUS_OUTPUT);
  I2c.send(os);
  I2c.endTransmission(); // Schließen der Verbindung
}

/*
 * This Blink mode simply switches the outer leds on and off
 * everytime the timer runs over the BLINK_SLOW_TIME
 */
void blinkModeSlow()
{
  unsigned long mode_timer = millis() - timer_blink;

  if ((mode_timer > 0) && (mode_timer < BLINK_SLOW_TIME))
    ledOutputStream = LED_MIDDLE & LED_INNER_RY & LED_INNER_GP; // all outer leds off

  else if ((mode_timer > BLINK_SLOW_TIME) && (mode_timer < 2 * BLINK_SLOW_TIME))
    ledOutputStream = LED_ON; // all leds on

  else
    timer_blink = millis();
}

/*
 * This Blink mode simply switches all outer leds on and off
 * everytime the timer runs over the BLINK_FAST_TIME
 */
void blinkModeFast()
{
  unsigned long mode_timer = millis() - timer_blink;

  if ((mode_timer > 0) && (mode_timer < BLINK_FAST_TIME))
    ledOutputStream = LED_MIDDLE & LED_INNER_RY & LED_INNER_GP; // all outer leds off

  else if ((mode_timer > BLINK_FAST_TIME) && (mode_timer < 2 * BLINK_FAST_TIME))
    ledOutputStream = LED_ON; // all leds on

  else
    timer_blink = millis();
}

/*
 * This blinkmode lets the leds blink in a circle row
 */
void blinkModeCircle()
{
  unsigned long mode_timer = millis() - timer_blink;

  if ((mode_timer > 0) && (mode_timer < BLINK_CIRCLE_TIME))
    ledOutputStream = LED_GREEN & LED_MIDDLE & LED_INNER_RY & LED_INNER_GP;

  else if ((mode_timer > BLINK_CIRCLE_TIME) && (mode_timer < 2 * BLINK_CIRCLE_TIME))
    ledOutputStream = LED_PURPLE & LED_MIDDLE & LED_INNER_RY & LED_INNER_GP;

  else if ((mode_timer > 2 * BLINK_CIRCLE_TIME) && (mode_timer < 3 * BLINK_CIRCLE_TIME))
    ledOutputStream = LED_RED & LED_MIDDLE & LED_INNER_RY & LED_INNER_GP;

  else if ((mode_timer > 3 * BLINK_CIRCLE_TIME) && (mode_timer < 4 * BLINK_CIRCLE_TIME))
    ledOutputStream = LED_YELLOW & LED_MIDDLE & LED_INNER_RY & LED_INNER_GP;

  else
    timer_blink = millis();
}

/*
 * This Blink Mode alternates the blinking colors in a 45° degree distance
 */
void blinkModeCrossing()
{
  unsigned long mode_timer = millis() - timer_blink;

  if ((mode_timer > 0) && (mode_timer < BLINK_CROSSING_TIME))
    ledOutputStream = LED_GREEN & LED_RED & LED_INNER_GP & LED_MIDDLE;

  else if ((mode_timer > BLINK_CROSSING_TIME) && (mode_timer < 2 * BLINK_CROSSING_TIME))
    ledOutputStream = LED_PURPLE & LED_YELLOW & LED_INNER_RY & LED_MIDDLE;

  else
    timer_blink = millis();
}

/*
 * This Blink Mode alternates the inner leds
 */
void blinkModeInner()
{
  unsigned long mode_timer = millis() - timer_blink;

  if ((mode_timer > 0) && (mode_timer < BLINK_INNER_TIME))
    ledOutputStream = LED_GREEN & LED_RED & LED_PURPLE & LED_YELLOW & LED_INNER_GP;

  else if ((mode_timer > BLINK_INNER_TIME) && (mode_timer < 2 * BLINK_INNER_TIME))
    ledOutputStream = LED_GREEN & LED_RED & LED_PURPLE & LED_YELLOW & LED_INNER_RY;

  else if ((mode_timer > 2 * BLINK_INNER_TIME) && (mode_timer < 3 * BLINK_INNER_TIME))
    ledOutputStream = LED_GREEN & LED_RED & LED_PURPLE & LED_YELLOW & LED_MIDDLE;

  else
    timer_blink = millis();
}

/*
 * Let the LEDs blink in a random order
 */
void blinkModeRandom()
{
  unsigned long mode_timer = millis() - timer_blink;

  // get a random number between 0-255 and set it to the outputstream
  if (!((mode_timer > 0) && (mode_timer < BLINK_RANDOM_TIME)))
  {
    ledOutputStream = B11111111 & random(0, 255);
    timer_blink = millis();
  }
}
/**************************************************************/
/*LED MANAGEMENT END*/
/**************************************************************/

/**************************************************************/
/*HELPER FUNCTIONS BEGIN*/
/**************************************************************/
/*
 *Helper function to convert int into char*
 */
char *IntToChar(int i)
{
  char *stra = "";
  stra = itoa(i, stra, 10); // int to array. in this case the char*. 10=basis
  return stra;
}

/*
 *Helper function to convert long into char*
 */
char *LongToChar(long i)
{
  char *strb = "";
  strb = ltoa(i, strb, 10); // int to array. in this case the char*. 10=basis
  return strb;
}

/*
 *Helper function to convert int into char* and add a "%" as last char
 */
char *IntToPercent(int i)
{
  char *strc = "";
  strc = ltoa(i, strc, 10); // int to array. in this case the char*. 10=basis

  // add a percent sign at the end of the string
  int len = strlen(strc);
  strc[len] = '%';
  strc[len + 1] = '\0';

  return strc;
}
/**************************************************************/
/*HELPER FUNCTIONS END*/
/**************************************************************/
