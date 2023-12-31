// variables for the ball
float posX = random(300); // set the default X and Y positions to random
float posY = random(400); // posX in <0, 300) and posY in <0, 400)
float ballR = 10;
float dX = 3;
float dY = 3;

// variables for the paddle
float pposX;
float pposY = 15;
float paddleW = 15;
float paddleH = 50;

// setting up the visual of the ball
void ball() {
  // the ball will be black and the dimensions are set by the variables above
  fill(0);
  ellipse(posX, posY, 2 * ballR, 2 * ballR);
}

// setting up the visual of the paddle
void paddle() {
  // paddle will be black and have dimensions set by the variables above
  fill(0);
  rect(0, 0, paddleW, paddleH);
}

// setting up the screen + position of the player to be slighty lower
void setup() {
  size(640, 400);
  pposX = width - 1.5 * paddleW;
}

// main function to work with everything
void draw() {
  // setting up color of background
  background(255, 255, 255);
  ball();
  
  // moving the ball
  posX = posX + dX;
  posY = posY + dY;
  // if the ball reaches one of the screen borders, it should change direction
  // for both directions dX and dY
  if(posX > width || posX < 0) {
    dX = -dX;
  }
  if (posY > height || posY < 0) {
    dY = -dY;
  }
  
  // saving current transformation matrix onto stack
  pushMatrix();
  // moves the origin of the coordinate system to the specified (posX, posX) location
  translate(posX, posX);
  // popMatrix() restores the previous transformation matrix from the stack
  popMatrix();
  
  translate(pposX, pposY);
  paddle();
  
  // if there is a collision with the bat, change the direction
  if (collision()) {
    dX = -dX;
  }
  
  // if the ball is behind the screen, the game ends
  if (posX + ballR > width) {
    background(255, 117, 117); // when losing, the game just shows a red screen
    noLoop();
  }
  
  // handling of keypresses
  if (keyPressed) {
    if (key == CODED) {
      if (keyCode == UP) { // if up arrow was pressed
        if (pposY >= 0) { // move up unless you are at the top of the screen
          pposY = pposY - 5;
        }
      }
      if (keyCode == DOWN) { // if down arrow was pressed
        if (pposY <= height - paddleH) { // move down unless you are at the bottom of the screen
          pposY = pposY + 5;
        }
      }
    }
  }
}

// Function that will detect collisions with the bat
boolean collision() {
  if (posX + ballR >= pposX) {
    if ((posY >= pposY) && posY <= pposY + paddleH) {
      return true;
    }
  }
  return false;
}
