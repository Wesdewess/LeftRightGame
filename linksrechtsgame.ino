#include <LiquidCrystal.h>
#include <Servo.h>
#include <LiquidCrystal.h>
#include<Wire.h>


//author Wessel Bakker

//set pins to use
//input: sonar
const int trig_sonic = 9;
const int echo_sonic = 8;

//input: tilt
const int MPU=0x68; 
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

//output: screen
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
//output: servo
Servo hatch;

//other variables
boolean active;
boolean endScreen = false;
int score;
int mode = 0;
//during game
int side=2;
boolean isCorrect;
boolean dropped;
String dMode;
String dSide;
int reactionTime = 10; //how long does it take for the ball to drop after the side has been displayed on screen
boolean detected = false;
boolean startup = true;

//game settings
int balls = 10; //how many balls are loaded in the hopper
int droppedBalls = 0; //how many balls have been dropped in total
int avgDelay = 2000; //average delay between drops in singleplayer

void setup() {
  //setup accelerometor
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); 
  Wire.write(0);    
  Wire.endTransmission(true);
  Serial.begin(9600);
  
  //configure pins accordingly
  pinMode(trig_sonic, OUTPUT); //ultrasonic sensor
  pinMode(echo_sonic,INPUT);   //ultrasonic sensor
  hatch.attach(10);      //servo to drop ball
  lcd.begin(16, 2);

  //other variables
  dropped=false;
  active = false;
  endScreen = false;
  score = 0; //points to start with
  droppedBalls = 0;
  updateDisplay();
  //first, check wheter the user has selected single or multiplayer before entering the loop
  mode = checkPlayers(); //mode is (0)single- or (1)multiplayer
  Serial.println("mode is " + mode);
  //mode = 1;
  //activate game
  active = true;
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("Get ready!");
  lcd.setCursor(7,1);
  lcd.print("3");
  delay(1000);
  lcd.setCursor(7,1);
  lcd.print("2");
  delay(1000);
  lcd.setCursor(7,1);
  lcd.print("1");
  delay(1000);
  lcd.clear();
  updateDisplay();
  startup=false;
}



void loop() {

//singleplayer code
  if(mode==0 && active==true){
  
  //randomly choose left or right
  side = randomLeftRight();
  
  //random delay between drops
  int rndDelay = randomDelay();
  delay(rndDelay);
  updateDisplay();
  delay(reactionTime);
  dropBall(); //drop the ball from the top
  updateDisplay();
  generalCheck();
  } 

//multiplayer code
  if(mode==1 && active==true){
    
  //player 2 chooses left or right
  side = chooseLeftRight();
  updateDisplay();
  //player 2 chooses when to drop
  //updateDisplay();
  delay(reactionTime);
  dropBall();
  updateDisplay();

  generalCheck();
  }

updateDisplay();

while(endScreen==true){
  delay(500);
  Serial.println("Checking for restart");
  checkRestart();
}

}



/*
 * All functions used, below this point
 */
void checkRestart(){
 
 while(checkDistance()>2){
   //do nothing
   //Serial.println("distance is: " + checkDistance());
 }
 setup();
 
}

int checkPlayers(){ //check to see if single or multiplayer is chosen. Dont exit funtion before user has chosen
    int chosenSide;
  while(tiltY()<5000 && tiltY()>-5000){
    //do nothing, wait for user input
  }
  if(tiltY()<=-5000){
   chosenSide = 0;
  }
  if(tiltY()>=5000){
   chosenSide = 1; 
  }
  Serial.println("side: " + chosenSide);
  return chosenSide;
}

int randomLeftRight(){ //decides where the ball should be dropped (singleplayer)
  int rSide = random(0,2);
  Serial.println(rSide);
  detected=false;
  return rSide;
}

int chooseLeftRight(){ //player 2 chooses where to drop the ball (multiplayer)
  int chosenSide;
  while(tiltY()<5000 && tiltY()>-5000){
    //do nothing, wait for user input
  }
  if(tiltY()<=-5000){
   chosenSide = 0;
  }
  if(tiltY()>=5000){
   chosenSide = 1; 
  }
  return chosenSide;
}

void dropBall(){ //opens the hatch at the top to drop a ball
    hatch.write(0);
    delay(150);
    hatch.write(80);
    droppedBalls++;
    dropped=true;
}

void playerDrop(){ //player 2 chooses when to drop the ball
  //wait for action by user, then drop ball
  //(dont wait for user input for now. Ball drops when side is chosen)
  dropBall();
}

int checkLeftRight(){ //to check if the player dropped a ball down the left or the right hole
    int distance = checkDistance();
    int dSide;
    while(distance>23){
    
    distance = checkDistance();
  if(distance<=23){
    if(distance<=10){ //if the ball is left
    dSide = 0;  
    }
    if(distance>13 && distance<=23){ //if the ball is right
    dSide = 1;
    }
    dropped=false;
  }
}
detected = true;
return dSide;
}

int checkDistance(){
    digitalWrite(trig_sonic, LOW);
    delayMicroseconds(2);
    digitalWrite(trig_sonic, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig_sonic, LOW);
  
    long duration= pulseIn(echo_sonic, HIGH);
    int distance= duration/29/2; //distance in centimeters
    Serial.println(distance);
    return distance;
}

void addScore(){ //adds a point to the total score
  score++;
}

void subtractScore(){ //subtracts a point from the total score (possibly wont be needed)
  //score--;
}

void generalCheck(){ //same code for single and multiplayer
  
  //check which side the ball dropped
  int droppedSide = checkLeftRight();

  //for debugging purposes print chosen side to serial monitor
  String kant;
  if(droppedSide==0){
    kant = "links";
  }else{
    kant = "rechts";
  }
  Serial.println("Gekozen kant is: " + kant);

  
  if(droppedSide==side){ //check if the dropped side is the same as the chosen correct side
    isCorrect = true;
    addScore();
  } else{
    isCorrect = false;
    subtractScore();
  }

  //check if the game is out of balls to drop, if so, end the round and show endscreen
  if(droppedBalls>=balls){
    active=false;
    endScreen = true;
  }

}

int randomDelay(){ //create a random delay between ball drops
  int rnd = random(-1500, 1500);
  int Delay = avgDelay + rnd;
  return Delay;
}

void updateDisplay(){ //updates information on display
  if(endScreen==true && active==false){
    lcd.clear();
    lcd.print("Ronde voorbij   ");
    //lcd.setCursor(0,1);
    //lcd.print("Score:     ");
  }else{ 
    if(active==false){ //voordat het spel is begonnen
  //display: Kies 1 of 2 speler modus
  //lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Kies een modus:");
  lcd.setCursor(0,1);
  lcd.print("1 of 2 spelers"); 
  return;
 } else{
    if(mode==0){
      dMode = "Single";
    }else{
      dMode = "Multi ";
    }
    
    if(dropped==false && startup == false){
      if(side==0){
        dSide = "Links ";
      }
      if(side==1){
        dSide = "Rechts";
      }
      if(side==2){
        
      }
           
    }
    if(detected==true || startup == true){
      dSide="      ";
    }
 }
    
    //write to display: mode + score + side
  lcd.setCursor(0,0);
  lcd.print(dMode + "     " + droppedBalls + "/" + balls);
  lcd.setCursor(4,1);
  lcd.print(dSide + " " + score + "/" + balls);

 }
 }

int tiltX(){ //read X axis from accelerometor
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,12,true);  
  AcX=Wire.read()<<8|Wire.read();
  return AcX;    
 }

int tiltY(){ //read Y axis from accelerometor
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,12,true);
  AcY=Wire.read()<<8|Wire.read();
  return AcY;
}


