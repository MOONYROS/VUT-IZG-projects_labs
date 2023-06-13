// variables for the ball
float posX = 30;
float posY = 30;
float ballR = 30;
float dX = 1;

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
  
  posX = posX + dX;
  if(posX > width || posX < 0) {
    dX = -dX;
  }
  
  pushMatrix();
  translate(posX, posX);
  ball();
  popMatrix();
  
  translate(pposX, pposY);
  paddle();
  
  if (collision()) {
    dX = -dX;
  }
  if (posX + ballR > width) {
    fill(255, 0, 0, 100);
    rect(0, 0, width, height);
    noLoop();
  }
  
  // handling of keypresses
  if (keyPressed) {
    if (key == CODED) {
      if (keyCode == UP) { // if up arrow was pressed
        if (pposY >= 0) { // move up unless you are on top of the screen
          pposY = pposY - 5;
        }
      }
      if (keyCode == DOWN) { // if down arrow was pressed
        if (pposY <= height - paddleH) { // move down unless you are on the bottom of the screen
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
