#include <SPI.h>
#include <TFT.h>
#include <string.h>

#define CS 10
#define DC 9 // data/command toggle
#define RST 8 // reset
#define INPUT A0

#define PADDLE_LENGTH 25
#define BALL_SIZE 4
#define SCREEN_WIDTH 127
#define SCREEN_HEIGHT 127

TFT TFTscreen = TFT(CS, DC, RST);

// 2d vector struct
struct Vec2 {
  int x;
  int y;

  Vec2(int x_pos, int y_pos) {
    x = x_pos;
    y = y_pos;
  }

  Vec2() {
    x = 0;
    y = 0;
  }

  bool operator == (const Vec2 &other) const {
    if (this->x == other.x && this->y == other.y) 
      return true;
    return false;
  }

  bool operator != (const Vec2 &other) const {
    if (*this == other)
      return false;
    return true;
  }

  Vec2 operator + (const Vec2 &other) {
    Vec2 newVec;
    newVec.x = this->x + other.x;
    newVec.y = this->y + other.y;
    return newVec;
  }
};

void logVec(Vec2 vector, String label) {
  Serial.print(label + " x: ");
  Serial.println(vector.x);
  Serial.print(label + " y: ");
  Serial.println(vector.y);
}

class Ball {
private:
  Vec2 vel; 
  Vec2 pos;

  bool checkCollision() {
    // collide with box
    if (pos.x == BALL_SIZE || pos.x == SCREEN_WIDTH) {
      vel.x *= -1;
      return true;
    }
    if (pos.y == BALL_SIZE || pos.y == SCREEN_HEIGHT) {
      vel.y *= -1;
      return true;
    }

    // add in paddle cases

    // neither case was triggered
    return false;
  }

public:

  Ball (Vec2 initVel) {
    vel = initVel;
    pos.x = SCREEN_WIDTH / 2;
    pos.y = SCREEN_HEIGHT / 2;
  }

  Vec2 getPos() const {
    return pos;
  }

  Vec2 getVel() const {
    return vel;
  }

  void update () {
    Vec2 newPos(pos.x + vel.x, pos.y + vel.y);
    logVec(newPos, "newPos");

    // update x pixel and y pixel by pixel to see if we collide
    while (pos != newPos) {
      if (pos.x != newPos.x && vel.x != 0) 
        pos.x += vel.x / abs(vel.x);
      
      if (pos.y != newPos.y && vel.y != 0) 
        pos.y += vel.y / abs(vel.y);

      if (checkCollision())
        break;
    }
  }
};

// global pointer for the ball so we can get it in all scopes
Ball* ball = nullptr; // will define in setup()

void drawBall () {
  Vec2 position = ball->getPos();
  TFTscreen.stroke(255, 255, 255); // white foreground
  TFTscreen.fill(255, 255, 255); // white fill
  TFTscreen.rect(position.x, position.y, BALL_SIZE, BALL_SIZE);
}

void eraseBall () {
  Vec2 position = ball->getPos();
  TFTscreen.stroke(0, 0, 0); // black foreground
  TFTscreen.fill(0, 0, 0); // black fill
  TFTscreen.rect(position.x, position.y, BALL_SIZE, BALL_SIZE);
}

void setup() {
  TFTscreen.begin();
  TFTscreen.background(0, 0, 0); // black background
  TFTscreen.stroke(255, 255, 255); // white foreground

  Serial.begin(9600);

  Vec2 velocity(3, 4);
  ball = new Ball(velocity);
}

void loop() { // game function
  logVec(ball->getPos(), "Position");
  logVec(ball->getVel(), "Velocity");
  Serial.println(" ");
  drawBall();
  delay(100);
  eraseBall();
  ball->update();
}