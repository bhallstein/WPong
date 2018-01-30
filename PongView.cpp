#include "PongView.h"

#define PADDLE_W 20   // Constant
#define PADDLE_H 0.2  // Fraction of window height

struct DObjs {
  DObjs(W::View *_v) : v(_v)
  {
    bg = new W::DRect(v, W::position(), W::size(), W::Colour::Black);
    ball = new W::DRect(v, W::position(), W::size(), W::Colour::White);
    lPaddle = new W::DRect(v, W::position(), W::size(), W::Colour::White);
    rPaddle = new W::DRect(v, W::position(), W::size(), W::Colour::White);
    separatorLine = new W::DLine(v, W::position(), W::position(), W::Colour(1,1,1,0.8));
    lScore = new W::DText(v, W::position(), "0", W::Colour::White);
    rScore = new W::DText(v, W::position(), "0", W::Colour::White, W::TextAlign::Right);

    transpOverlay = new W::DRect(v, W::position(), W::size(), W::Colour::TransparentBlack);
    startText = new W::DText(v, W::position(), "press space to start", W::Colour::White, W::TextAlign::Centre);
  }
  ~DObjs()
  {
    delete bg;
    delete ball;
    delete lPaddle;
    delete rPaddle;
    delete separatorLine;
    delete lScore;
    delete rScore;
    delete transpOverlay;
    delete startText;
  }
  void layOut(const W::size &sz) {
    // Background
    bg->setSz(sz);

    // Paddle positions
    int curPaddleHeight = PADDLE_H * sz.height;
    int curLPaddleVPos, curRPaddleVPos;

    paddleMaxYPos = sz.height - 10 - curPaddleHeight;

    if (performInitialLayout) {
      // Initial layout
      curLPaddleVPos = curRPaddleVPos = (sz.height - curPaddleHeight) * 0.5;
    }
    else {
      // Check if need to move paddles up due to window smallening
      curLPaddleVPos = lPaddle->pos.y;
      curRPaddleVPos = rPaddle->pos.y;
      if (curLPaddleVPos > paddleMaxYPos) curLPaddleVPos = paddleMaxYPos;
      if (curRPaddleVPos > paddleMaxYPos) curRPaddleVPos = paddleMaxYPos;
    }
    lPaddle->setPos(W::position(paddleHOffs, curLPaddleVPos));
    rPaddle->setPos(W::position(sz.width-paddleHOffs-PADDLE_W, curRPaddleVPos));

    // Paddle sizes
    W::size paddleSize(PADDLE_W, curPaddleHeight);
    lPaddle->setSz(paddleSize);
    rPaddle->setSz(paddleSize);

    // Ball
    if (performInitialLayout) {
      // Initial layout
      ball->setPos(W::position(int((sz.width-ballSize)*0.5),int((sz.height-ballSize)*0.5)));
      ball->setSz(ballSize);
    }

    // Sep line
    separatorLine->setP1(W::position(sepLineHOffs, sepLineVOffs));
    separatorLine->setP2(W::position(sz.width-sepLineHOffs, sepLineVOffs));

    // Scores
    lScore->setPos(W::position(scoreHOffs, scoreVOffs));
    rScore->setPos(W::position(sz.width-scoreHOffs, scoreVOffs));

    // Overlay shizzle
    transpOverlay->setSz(sz);
    startText->setPos(W::position(sz.width*0.5, sz.height - 40));

    performInitialLayout = false;
  }

  W::View *v;

  W::DRect *bg, *ball, *lPaddle, *rPaddle, *transpOverlay;
  W::DLine *separatorLine;
  W::DText *lScore, *rScore, *startText;

  bool performInitialLayout = true;

  // Layout attributes
  int paddleHOffs = 10;
  int paddleMinYPos = 30, paddleMaxYPos = -1;
  int ballSize = 20;
  int sepLineHOffs = 10, sepLineVOffs = 30;
  int scoreHOffs = 10, scoreVOffs = 10;
};


/*******************************/
/*** PongView Implementation ***/
/*******************************/

PongView::PongView() : W::View(W::Positioner::WholeAreaPositioner), dObjs(new DObjs(this))
{
  dObjs->layOut(rct.sz);

#ifdef WTARGET_IOS
  dObjs->startText->setTxt("touch to start");
#endif

  using namespace W::EventType;
  W::Messenger::subscribe(KeyDown, W::Callback(&PongView::keyDown, this));
  W::Messenger::subscribe(KeyUp, W::Callback(&PongView::keyUp, this));
}
PongView::~PongView()
{
  delete dObjs;
}

void PongView::updatePosition(const W::size &winSize) {
  if (!running) dObjs->performInitialLayout = true;
  dObjs->layOut(rct.sz);
}
void PongView::processMouseEvent(W::Event *ev) {
  // Do things
}
W::EventPropagation::T PongView::keyDown(W::Event *ev) {
  if (ev->key == W::KeyCode::SPACE)    startGame();
  else if (ev->key == W::KeyCode::ESC) W::popState(W::EmptyReturny);

  else if (ev->key == W::KeyCode::_A)         lNudgeUpActive   = true;
  else if (ev->key == W::KeyCode::_Z)         lNudgeDownActive = true;
  else if (ev->key == W::KeyCode::UP_ARROW)   rNudgeUpActive   = true;
  else if (ev->key == W::KeyCode::DOWN_ARROW) rNudgeDownActive = true;

  return W::EventPropagation::ShouldStop;
}
W::EventPropagation::T PongView::keyUp(W::Event *ev) {
  if (ev->key == W::KeyCode::_A)              lNudgeUpActive   = false;
  else if (ev->key == W::KeyCode::_Z)         lNudgeDownActive = false;
  else if (ev->key == W::KeyCode::UP_ARROW)   rNudgeUpActive   = false;
  else if (ev->key == W::KeyCode::DOWN_ARROW) rNudgeDownActive = false;

  return W::EventPropagation::ShouldStop;
}
void PongView::touchDown(W::Event *ev) {
  startGame();
  W::Messenger::subscribeToTouchEvent(ev->touchID, W::Callback(&PongView::touchUpd, this));
  touches.push_back(TouchAndPosition(ev->touchID, ev->pos));
}
W::EventPropagation::T PongView::touchUpd(W::Event *ev) {
  using namespace W::EventType;
  if (ev->type == TouchMoved) {
    updateTouchPosn(ev->touchID, ev->pos);
  }
  else if (ev->type == TouchUp || ev->type == TouchCancelled) {
    int touchID = ev->touchID;
    removeTouch(touchID);
    W::Messenger::unsubscribeFromTouchEvent(touchID, this);
  }
  return W::EventPropagation::ShouldContinue;
}
void PongView::removeTouch(int touchID) {
  for (std::vector<TouchAndPosition>::iterator it = touches.begin(); it < touches.end(); )
    if (it->touchID == touchID) it = touches.erase(it);
    else ++it;
}
void PongView::updateTouchPosn(int touchID, const W::position &newpos) {
  for (std::vector<TouchAndPosition>::iterator it = touches.begin(); it < touches.end(); ++it)
    if (it->touchID == touchID) it->pos = newpos;
}

void PongView::update() {
  if (!running) return;

  incrementBallPos();

  // Move paddles
  int nudgeDist = 5;
  if      (lNudgeUpActive)   nudgePaddle(KLEFT, -nudgeDist);
  else if (lNudgeDownActive) nudgePaddle(KLEFT, nudgeDist);
  if      (rNudgeUpActive)   nudgePaddle(KRIGHT, -nudgeDist);
  else if (rNudgeDownActive) nudgePaddle(KRIGHT, nudgeDist);

  // Touch: let all current touches influence paddle movt
  for (std::vector<TouchAndPosition>::iterator it = touches.begin(); it < touches.end(); ++it) {
    const W::position &tPos = (*it).pos;
    lr_enum viewSector = (tPos.x < rct.sz.width * 0.5 ? KLEFT : KRIGHT);
    const W::DRect &paddle = *(viewSector == KLEFT ? dObjs->lPaddle : dObjs->rPaddle);
    float touchDistFromPaddleCenter = tPos.y - (paddle.pos.y+PADDLE_H*rct.sz.height*0.5);
    nudgePaddle(viewSector, touchDistFromPaddleCenter * 0.2);
  }

  // Test for ball touching top/bottom
  W::position &ballPos = dObjs->ball->pos;
  if (ballPos.y <= dObjs->paddleMinYPos) {
    ballPos.y = dObjs->paddleMinYPos;
    dObjs->ball->setPos(ballPos);
    reflectV();
  }
  else if (ballPos.y + dObjs->ballSize >= rct.sz.height - dObjs->paddleHOffs) {
    ballPos.y = rct.sz.height - dObjs->paddleHOffs - dObjs->ballSize;
    dObjs->ball->setPos(ballPos);
    reflectV();
  }

  W::position &lPaddlePos = dObjs->lPaddle->pos;
  W::position &rPaddlePos = dObjs->rPaddle->pos;

  // L-hand tests
  if (ballPos.x <= dObjs->paddleHOffs + PADDLE_W) {
    if (ballPos.y + dObjs->ballSize >= lPaddlePos.y && ballPos.y < lPaddlePos.y + PADDLE_H*rct.sz.height)
      reflectH(KLEFT);
    else
      incrementScore(KRIGHT), restart();
  }

  // R-hand tests
  else if (ballPos.x + dObjs->ballSize > rct.sz.width - PADDLE_W - dObjs->paddleHOffs) {
    if (ballPos.y + dObjs->ballSize >= rPaddlePos.y && ballPos.y < rPaddlePos.y + PADDLE_H*rct.sz.height)
      reflectH(KRIGHT);
    else
      incrementScore(KLEFT), restart();
  }
}

void PongView::startGame() {
  if (running) return;
  dObjs->transpOverlay->setCol(W::Colour(0,0,0,0));
  dObjs->startText->setCol(W::Colour(0,0,0,0));
  running = true;
  restart();
}
void PongView::restart() {
  dObjs->performInitialLayout = true;
  resetBallVel();
  reflectCount = 0;
  dObjs->layOut(rct.sz);
}

void PongView::incrementBallPos() {
  dObjs->ball->setPos(dObjs->ball->pos + ballVelocity);
}
void PongView::nudgePaddle(lr_enum lr, int dist) {
  W::position *paddle_pos;
  W::DRect *paddle;
  if (lr == KLEFT) {
    paddle_pos = &dObjs->lPaddle->pos;
    paddle = dObjs->lPaddle;
  }
  else {
    paddle_pos = &dObjs->rPaddle->pos;
    paddle = dObjs->rPaddle;
  }
  paddle_pos->y += dist;
  if (paddle_pos->y < dObjs->paddleMinYPos)      paddle_pos->y = dObjs->paddleMinYPos;
  else if (paddle_pos->y > dObjs->paddleMaxYPos) paddle_pos->y = dObjs->paddleMaxYPos;
  paddle->setPos(*paddle_pos);
}
void PongView::reflectH(lr_enum lr) {
  ballVelocity.x *= -1;

  // Increase speed every 2 bounces
  if(!(++reflectCount%2))
    ballVelocity.x += (ballVelocity.x > 0 ? 2 : -2);

  // Alter vertical velocity of ball depending on where it touches the paddle
  float paddle_height = PADDLE_H * rct.sz.height;
  W::position *paddle_pos = &(lr == KLEFT ? dObjs->lPaddle->pos : dObjs->rPaddle->pos);
  int v_offset_ball   = dObjs->ball->pos.y + 0.5*dObjs->ballSize;
  int v_offset_paddle = paddle_pos->y + 0.5*paddle_height;
  float ball_offset_from_paddle = float(v_offset_ball - v_offset_paddle) / paddle_height;
  ballVelocity.y += ball_offset_from_paddle * 10;
}
void PongView::reflectV() {
  ballVelocity.y *= -1;
}
void PongView::incrementScore(lr_enum lr) {
  char s[5];
  sprintf(s, "%d", ++(lr == KLEFT ? scoreL : scoreR));
  (lr == KLEFT ? dObjs->lScore : dObjs->rScore)->setTxt(s);
}
