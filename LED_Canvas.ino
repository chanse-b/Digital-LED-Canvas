/*
 *  LED DISPLAY DRAWING PROGRAM
 *  
 *  -Program allows user to move a cursor around the led
 *  and choose which led's remain on by clicking the switch.
 *  If the led was previously on, the toggle swtich can also
 *  act as an eraser.
 *  -Resources used: many of the functions remain from our lab 2a code.
 *  We also utalizes the Arduino website to help set up the joystick and
 *  the lab2b_pushbutton handout to modify how the joystick is read in relation
 *  to the program.
 */

// Define Anodes and Cathode array, Joystick pins
const byte ANODE_PINS[8] = {13, 12, 11, 10, 9, 8, 7, 6};
const byte CATHODE_PINS[8] = {A3, A2, A1, A0, 5, 4, 3, 2};
const int joyX = A5; 
const int joyY = A4;
const int joyToggle = 1;

/*
 * General setup for the joystick and led board. *IMPORTED FROM LAB 2A*
 */
void setup() {
  pinMode(joyX, INPUT); // joystick inputs
  pinMode(joyY, INPUT);
  pinMode(joyToggle, INPUT_PULLUP); 
  for (byte i = 0; i < 8; i++) { // loop through all output pins, set as outputs, and turn off. Code imported from lab 2a
    pinMode(ANODE_PINS[i], OUTPUT);
    pinMode(CATHODE_PINS[i], OUTPUT);
    digitalWrite(ANODE_PINS[i], HIGH);
    digitalWrite(CATHODE_PINS[i], HIGH);
  }
}

/* Function: display 
 * -----------------
 * Runs through one multiplexing cycle of the LEDs, controlling which LEDs are
 * on.
 *
 * During this function, LEDs that should be on will be turned on momentarily,
 * one row at a time. When this function returns, all the LEDs will be off
 * again, so it needs to be called continuously for LEDs to be on.
 */
void display(byte pattern[8][8]) {
  for (int i = 0; i < 8; i++){
    for (int n = 0; n < 8; n++){
      if (pattern[n][i] > 0){ // if LED state is 1 or 2, turn cathode on
        digitalWrite(CATHODE_PINS[n], LOW);
      }
      else{
        digitalWrite(CATHODE_PINS[n], HIGH);
      }
    }
    digitalWrite(ANODE_PINS[i], LOW); //breifly turn on anode to flash the LED
    delay(1);
    digitalWrite(ANODE_PINS[i], HIGH);
  }
}

/* wrapAroundBoard function uses conditions to send an LED
* to either the horizontal or vertical opposite end of the
* display if it reaches a boundary. A value of 200 is chosen
* to prevent any "overshooting" of the joystick. For example,
* if the joystick moves right, the joystick will read "8"
* at minimum. If the joystick moves left, however, the board
* will read 255. Therefore, 200 is a good number to prevent
* any accidental moves of the joystick
*/
void wrapAroundBoard(byte &x, byte &y) {
  if ((x > 7) && (x < 200)) { //Ex. if LED travels to horizontal max going right, it will then appear on the leftmost pin.
    x = 0;
  } else if (x > 200) {
    x = 7;
  } else if ((y > 7) && (y < 200)) { // vertical wrap around
    y = 0;
  } else if (y > 200 ) {
    y = 7;
  }
}
  
// define joystick input positions and matching mapping value
int xPosition = 0;
int yPosition = 0;
int mapX = 0;
int mapY = 0;
unsigned long switchTime = 0;

/*
 *  stickRead function takes in analog positons from the joystick and
 *  matches them to integers mapX and mapY so that we can set conditions
 *  for LED movement using the joystick on the board. Takes in a coordinate
 *  by reference to modify its value depending on the position of the joystick.
 *  A joystick "debouncing" technique is used to avoid the joystick reading 
 *  the position too many times, possibly causing an "overshoot." The delay is 
 *  set to 150 mS since the joystick movement often takes longer to return to 
 *  equilibrium than a conventional switch. As it relates to the "overshoot",
 *  when the joystick is moved forward, the time is recorded. 
 *  If that time and the current time, using the millis() function
 *  is greater than the delay, the reading will take place once again.
 *  Also the value 400 is used to prevent any accidental movements of the joystick. The 
 *  absolute value of the current position of the joystick to prevent any ambiguity between
 *  intercardinal directions. The function relies on cardinal directions only
 */
void stickRead(byte &x, byte &y) {
  xPosition = analogRead(joyX); // reads analog x,y joystick values
  yPosition = analogRead(joyY);
  mapX = map(xPosition, 0, 1023, -511, 511); // maps the input to a numerical value
  mapY = map(yPosition, 0, 1023, 511, -511);  //511 flipped to account for the joystick orientation on the board
  mapX += 16; // offset for the hardware error
  mapY += 4;
  if ((millis() - switchTime) > 150) { //if current time is 150 ms after the last time the joystick was activated
    if (abs(mapX) > abs(mapY) && mapX > 400) { // if joystick moves right, move x value right by adding 1
      x++;
      switchTime = millis();    //The last time the joystick was activated
    } else if (abs(mapX) > abs(mapY) && mapX < -400) { // if joystick moves left, move x value left by subtracting 1
      x--;
      switchTime = millis();
    } else if (abs(mapX) < abs(mapY) && mapY > 400) { // if joystick moves down, move y value down by subtracting 1
      y--;
      switchTime = millis();
    } else if (abs(mapX) < abs(mapY) && mapY < -400) { // if joystick moves up, move y value up by adding 1
      y++;
      switchTime = millis();
    } 
  }
  wrapAroundBoard(x,y); //helper function used to wrap the cursor to the other respective side of the board.
}

/*
 * The loop function handles the cursor and the toggling of the joystick. 
 * Function utalizes helper functions such as stickread and display to
 * turn the correct led's on. The variables defined below represent the 
 * location of the cursor as x and y. clickX and clickY are initialized
 * as the starting position, but are reassigned in the loop() function as
 * the coordinates marked when the joystick is toggled. The previous_x and
 * previous_y variables are also initlaizes as the starting position, but 
 * are reassigned to record the previous location of the cursor.
 */
int previous_toggleState = 2;
byte x = 3; //represents cursor coordinates. (3,4) is the starting position
byte y = 4;
byte clickX = x;    //location of x coordinate of cursor when joystick pressed
byte clickY = y;    //location of y coordinate of cursor when joystick pressed 
byte previous_x = x; //last x location of cursor
byte previous_y = y; //last y location of cursor
unsigned long toggleTime = 0;

void loop() {                          
  static byte ledOn[8][8]; 
  previous_x = x; //last x location of cursor
  previous_y = y; //last y location of cursor
  stickRead(x, y);  //reads the direction of the joystick and modifies x, y accordingly


  /*This part of the loop function handles the "cursor." It works
   * by comparing the current x, y coordinate, maniuplated by stickRead,
   * to the previous coordinate of the cursor. ledOn[x][y] = 2 indicates
   * that the led was turned on explicitly by the user. Therefore, If the previous position
   * of the cursor did not have it's led engaged by the user, it will turn the led off.
   * Then, if the new location of the cursor's led is off, it will be turned on
   */
  if ((x != previous_x || y != previous_y) && ledOn[previous_x][previous_y] != 2 ) { //if this led does not have special permissions when the cursor passes it
    ledOn[previous_x][previous_y] = 0;                                               // turn it off
  }
  if (ledOn[x][y] == 0) { //if the current location of the cursor's led is off, turn it on
    ledOn[x][y] = 1;
  } 

  /*This part of the program handles the user toggling the switch, indicating
   * the led should remain on or off. The toggle switch is debounced as described
   * in the lab2b_pushbutton handout. The program refuses to read the toggle switch
   * until at least 50 ms have passed since its last input. When the switch is pressed,
   * the current position of the toggle is recorded as clickX and clickY. The previous
   * state of the toggle switch is recorded to preserve the led's that are already on. Resets
   * at the end of the statement. The if statement logic works as follows: if the switch
   * was pressed and the cursor is not on the coordinate that the switch was pressed, check
   * to see if the led was turned on the by user in the past. If it was on, turn it off. If 
   * it was off, turn it on. The led is also turned on with special permissions, denoted
   * by the ledOn[x][y] = 2. As a result, if the cursor hovers over an led that was toggled
   * on by the user, it will not turn off when the cursor moves past it.
   */
  if (0 == digitalRead(joyToggle) && ((toggleTime - millis()) > 50)) {    //if the switch is clicked, mark the current position of the cursor
    clickX = x;   
    clickY = y;
    toggleTime = millis();
    previous_toggleState = 0; 
  }
  if ((x != clickX  || y != clickY)  && previous_toggleState == 0) { //if the current position is not the marked position and the switch was pressed
    if (ledOn[clickX][clickY] == 2) { //turn the led off it it was on. 
       ledOn[clickX][clickY] = 0; 
      }
     else if (ledOn[clickX][clickY] == 0) { //if the led was already off
        ledOn[clickX][clickY] = 2;  //turn on with special user permission
      }
  previous_toggleState = 1;
  }
  display(ledOn);
}
