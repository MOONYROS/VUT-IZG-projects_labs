float posX = 30;
float posY = 30;
float ballR = 30;

void ball() {
  fill(0);
  ellipse(posX, posY, 2 * ballR, 2 * ballR);
}

void setup() {
  size(640, 400);
}

void draw() {
  background(255, 255, 255);
  ball();
  posX = posX + 1;
}
