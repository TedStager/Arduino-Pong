#include <SPI.h>
#include <TFT.h>

// pin definitions, the SPI bus on the Uno is predefined
// SCK and MOSI are already there
#define CS 10
#define DC 9 // data/command toggle
#define RST 8 // reset
#define INPUT A0

// geometric attributes, all in pixels
#define PADDLE_LENGTH 25
#define PADDLE_WIDTH 3
#define BALL_SIZE 4 // leave small or else it gets confused
#define SCREEN_WIDTH 127
#define SCREEN_HEIGHT 127

#define P1col 14
#define P2col 113 // ie, 127 - 14 or SCREEN_WIDTH - P1col

// 2d vector struct
struct Vec2 {
  int x;
  int y;

  // constructors
  Vec2();
  Vec2(int x_pos, int y_pos);

  // operators
  bool operator == (const Vec2 &other);
  bool operator != (const Vec2 &other);
  Vec2 operator + (const Vec2 &other);
};

void logVec(Vec2 vector, String label); // prints to serial monitor for debugging

// class for each paddle
class Paddle {
private:
  int pos; // vertical position
  const int col;

public:
  Paddle (int column);

  Vec2 getPos() const;

  void update();
};

// class for the ball
class Ball {
private:
  Vec2 vel;
  Vec2 pos;

  bool checkCollision (Paddle* P1);

public:
  Ball (Vec2 initVel, Vec2 initPos);

  Vec2 getPos() const;
  Vec2 getVel() const;

  void update (Paddle* P1);
};

// functions for drawing ball and paddles to screen
void drawBall(Vec2 position);
void eraseBall(Vec2 position);
void drawPaddle(Vec2 position);
void erasePaddle(Vec2 position);

// some global pointers to easily access objects in all scopes
// will define all in setup()
Ball* ball = nullptr; 
Vec2* oldBallPos = nullptr;
Paddle* P1 = nullptr;
Vec2* oldP1Pos = nullptr;

// global TFT object to use in various functions
TFT TFTscreen = TFT(CS, DC, RST);

void setup() {
  TFTscreen.begin();
  TFTscreen.background(0, 0, 0); // black background
  TFTscreen.stroke(255, 255, 255); // white foreground

  Serial.begin(9600);

  Vec2 velocity(2, 3);
  oldBallPos = new Vec2(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2); // here oldPos also acts as the start position
  ball = new Ball(velocity, *oldBallPos);
  
  P1 = new Paddle(P1col);
  oldP1Pos = new Vec2(P1->getPos());
}

void loop() { // game function
  logVec(ball->getPos(), "Ball Position");
  logVec(ball->getVel(), "Ball Velocity");
  logVec(P1->getPos(), "P1 Position");
  Serial.println(" ");

  erasePaddle(*oldP1Pos);
  drawPaddle(P1->getPos());

  eraseBall(*oldBallPos);
  drawBall(ball->getPos());

  *oldBallPos = ball->getPos();
  *oldP1Pos = P1->getPos();
  ball->update(P1);
  P1->update();

  // delay a tiny amount so that the game doesn't break on faster hardware
  delayMicroseconds(10);
}

// ============= VEC2 FUNCTION DEFINITIONS ============

Vec2::Vec2(int x_pos, int y_pos) {
  x = x_pos;
  y = y_pos;
}

Vec2::Vec2() {
  x = 0;
  y = 0;
}


bool Vec2::operator == (const Vec2 &other) {
  if (this->x == other.x && this->y == other.y) 
    return true;
  return false;
}

bool Vec2::operator != (const Vec2 &other) {
  if (*this == other)
    return false;
  return true;
}

Vec2 Vec2::operator + (const Vec2 &other) {
  Vec2 newVec;
  newVec.x = this->x + other.x;
  newVec.y = this->y + other.y;
  return newVec;
}



void logVec(Vec2 vector, String label) {
  Serial.print(label + " x: ");
  Serial.println(vector.x);
  Serial.print(label + " y: ");
  Serial.println(vector.y);
}

// ============= BALL CLASS DEFINITIONS ============

bool Ball::checkCollision(Paddle* P1) {
  // collide with box
  if (pos.x == BALL_SIZE || pos.x == SCREEN_WIDTH) {
    vel.x *= -1;
    return true;
  }
  if (pos.y == BALL_SIZE || pos.y == SCREEN_HEIGHT) {
    vel.y *= -1;
    return true;
  }

  // paddle 1 collision


  // neither case was triggered
  return false;
}

Ball::Ball (Vec2 initVel, Vec2 initPos) {
  vel = initVel;
  pos = initPos;
}

Vec2 Ball::getPos() const {
  return pos;
}

Vec2 Ball::getVel() const {
  return vel;
}

void Ball::update (Paddle* P1) {
  Vec2 newPos(pos.x + vel.x, pos.y + vel.y);
  logVec(newPos, "newPos");

  // update x pixel and y pixel by pixel to see if we collide
  while (pos != newPos) {
    if (pos.x != newPos.x && vel.x != 0) 
      pos.x += vel.x / abs(vel.x);
    
    if (pos.y != newPos.y && vel.y != 0) 
      pos.y += vel.y / abs(vel.y);

    if (checkCollision(P1))
      break;
  }
}

// ============ PADDLE FUNCTION DEFINITIONS ===========

Paddle::Paddle (int column) : col(column) {
  pos = SCREEN_HEIGHT / 2 + PADDLE_LENGTH / 2;
}

Vec2 Paddle::getPos() const {
  return Vec2(col, pos);
}

void Paddle::update () {
  float percent = analogRead(INPUT) / 1024.0;
  pos = SCREEN_HEIGHT * percent - PADDLE_LENGTH;
  if (pos < 0) pos = 0; // disallow negative position
}

// ============= DRAW FUNCTION DEFINITIONS ============

void drawBall (Vec2 position) {
  TFTscreen.stroke(255, 255, 255); // white foreground
  TFTscreen.fill(255, 255, 255); // white fill
  TFTscreen.rect(position.x, position.y, BALL_SIZE, BALL_SIZE);
}

void eraseBall (Vec2 position) {
  TFTscreen.stroke(0, 0, 0); // black foreground
  TFTscreen.fill(0, 0, 0); // black fill
  TFTscreen.rect(position.x, position.y, BALL_SIZE, BALL_SIZE);
}

void drawPaddle (Vec2 position) {
  TFTscreen.stroke(255, 255, 255); // white foreground
  TFTscreen.fill(255, 255, 255); // white fill
  TFTscreen.rect(position.x, position.y, BALL_SIZE, PADDLE_LENGTH);
}

void erasePaddle (Vec2 position) {
  TFTscreen.stroke(0, 0, 0); // black foreground
  TFTscreen.fill(0, 0, 0); // black fill  
  TFTscreen.rect(position.x, position.y, BALL_SIZE, PADDLE_LENGTH);
}
