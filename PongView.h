#ifndef PongView_hpp
#define PongView_hpp

#include "W.h"

enum lr_enum {
  KLEFT,
  KRIGHT,
};

struct DObjs;

struct TouchAndPosition {
  TouchAndPosition(int _id, const W::position &_pos) : touchID(_id), pos(_pos) { }
  int touchID;
  W::position pos;
};

class PongView : public W::View {
public:
  PongView();
  ~PongView();

  void updatePosition(const W::size &);
  void processMouseEvent(W::Event *);
  W::EventPropagation::T keyDown(W::Event *);
  W::EventPropagation::T keyUp(W::Event *);
  void touchDown(W::Event *);
  W::EventPropagation::T touchUpd(W::Event *);

  void update();

private:
  DObjs *dObjs;

  void startGame();
  void restart();

  void incrementBallPos();
  void nudgePaddle(lr_enum lr, int dist);
  void resetBallVel() {
    ballVelocity.x = (W::Rand::intUpTo(2) ? 4 : -4);
    ballVelocity.y = W::Rand::intUpTo(5) - 2;
  }
  void incrementBall();
  void reflectH(lr_enum);
  void reflectV();

  void incrementScore(lr_enum);

  W::position ballVelocity;
  bool running = false;
  int nudge_speed = 5;
  int reflectCount;
  int scoreL = 0, scoreR = 0;

  // Paddle control key tracking
  bool rNudgeUpActive = false;
  bool lNudgeUpActive = false;
  bool rNudgeDownActive = false;
  bool lNudgeDownActive = false;

  // Touch tracking
  std::vector<TouchAndPosition> touches;
  void removeTouch(int);
  void updateTouchPosn(int, const W::position &);
};


#endif
