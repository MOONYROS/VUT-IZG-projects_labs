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

void ball() {
  fill(0);
  ellipse(posX, posY, 2 * ballR, 2 * ballR);
}

void paddle() {
  fill(0);
  rect(0, 0, paddleW, paddleH);
}

void setup() {
  size(640, 400);
  pposX = width - 1.5 * paddleW;
}

void draw() {
  background(255, 255, 255);
  
  pushMatrix();
  translate(posX, posX);
  ball();
  popMatrix();
  
  translate(pposX, pposY);
  paddle();
  
  if (keyPressed) {
    if (key == CODED) {
      if (keyCode == UP) {
        if (pposY >= 0) {
          pposY = pposY - 5;
        }
      }
      if (keyCode == DOWN) {
        if (pposY <= height - paddleH) {
          pposY = pposY + 5;
        }
      }
    }
  }
  
  //posX = posX + dX;
  //if(posX > width || posX < 0) {
  //  dX = -dX;
  //}
}
