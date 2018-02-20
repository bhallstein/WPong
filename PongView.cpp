#include "PongView.h"

#define PADDLE_W 20   // Constant
#define PADDLE_H 0.2  // Fraction of window height

struct DObjs {
  DObjs(W::View *_v) : v(_v)
  {
    bg = new W::Rectangle(v, W::v2i(), W::v2i(), W::Colour::Black);
    ball = new W::Rectangle(v, W::v2i(), W::v2i(), W::Colour::White);
    lPaddle = new W::Rectangle(v, W::v2i(), W::v2i(), W::Colour::White);
    rPaddle = new W::Rectangle(v, W::v2i(), W::v2i(), W::Colour::White);
    separatorLine = new W::Line(v, W::v2f(), W::v2f(), W::Colour(1,1,1,0.8), 1);
    lScore = new W::RetroText(v, W::v2f(), "0", W::Colour::White, W::TextAlign::Left);
    rScore = new W::RetroText(v, W::v2i(), "0", W::Colour::White, W::TextAlign::Right);

    transpOverlay = new W::Rectangle(v, W::v2i(), W::v2i(), W::Colour::TransparentBlack);
    startText = new W::RetroText(v, W::v2f(), "press space to start", W::Colour::White, W::TextAlign::Centre);
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

    hideStartText();
  }
  void hideStartText() {
    if (startText) {
      delete startText;
      startText = NULL;
    }
  }

  void layOut(const W::v2i &sz) {
    // Background
    bg->setSz(sz);

    // Paddle positions
    int curPaddleHeight = PADDLE_H * sz.b;
    int curLPaddleVPos, curRPaddleVPos;

    paddleMaxYPos = sz.b - 10 - curPaddleHeight;

    if (performInitialLayout) {
      // Initial layout
      curLPaddleVPos = curRPaddleVPos = (sz.b - curPaddleHeight) * 0.5;
    }
    else {
      // Check if need to move paddles up due to window smallening
      curLPaddleVPos = lPaddle->pos.b;
      curRPaddleVPos = rPaddle->pos.b;
      if (curLPaddleVPos > paddleMaxYPos) curLPaddleVPos = paddleMaxYPos;
      if (curRPaddleVPos > paddleMaxYPos) curRPaddleVPos = paddleMaxYPos;
    }
    lPaddle->setPos(W::v2i(paddleHOffs, curLPaddleVPos));
    rPaddle->setPos(W::v2i(sz.a-paddleHOffs-PADDLE_W, curRPaddleVPos));

    // Paddle sizes
    W::v2i paddleSize(PADDLE_W, curPaddleHeight);
    lPaddle->setSz(paddleSize);
    rPaddle->setSz(paddleSize);

    // Ball
    if (performInitialLayout) {
      // Initial layout
      ball->setPos(W::v2i(int((sz.a-ballSize)*0.5),int((sz.a-ballSize)*0.5)));
      ball->setSz(ballSize);
    }

    // Sep line
    separatorLine->setP1(W::v2i(sepLineHOffs, sepLineVOffs));
    separatorLine->setP2(W::v2i(sz.a-sepLineHOffs, sepLineVOffs));

    // Scores
    lScore->setPos(W::v2i(scoreHOffs, scoreVOffs));
    rScore->setPos(W::v2i(sz.a-scoreHOffs, scoreVOffs));

    // Overlay shizzle
    transpOverlay->setSz(sz);

    if (startText) {
      startText->setPos(W::v2i(sz.a*0.5, sz.b - 40));
    }

    performInitialLayout = false;
  }

  W::View *v;

  W::Rectangle *bg, *ball, *lPaddle, *rPaddle, *transpOverlay;
  W::Line *separatorLine;
  W::RetroText *lScore;
  W::RetroText *rScore;
  W::RetroText *startText;

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
  dObjs->layOut(rct.size);

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

void PongView::updatePosition(const W::v2i &winSize) {
  if (!running) dObjs->performInitialLayout = true;
  dObjs->layOut(rct.size);
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
void PongView::updateTouchPosn(int touchID, W::v2f newpos) {
  for (auto &t : touches) {
    if (t.touchID == touchID) t.pos = newpos;
  }
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
    const W::v2f &tPos = (*it).pos;
    lr_enum viewSector = (tPos.a < rct.size.a * 0.5 ? KLEFT : KRIGHT);
    const W::Rectangle &paddle = *(viewSector == KLEFT ? dObjs->lPaddle : dObjs->rPaddle);
    float touchDistFromPaddleCenter = tPos.b - (paddle.pos.b+PADDLE_H*rct.size.b*0.5);
    nudgePaddle(viewSector, touchDistFromPaddleCenter * 0.2);
  }

  // Test for ball touching top/bottom
  W::v2f &ballPos = dObjs->ball->pos;
  if (ballPos.b <= dObjs->paddleMinYPos) {
    ballPos.b = dObjs->paddleMinYPos;
    dObjs->ball->setPos(ballPos);
    reflectV();
  }
  else if (ballPos.b + dObjs->ballSize >= rct.size.b - dObjs->paddleHOffs) {
    ballPos.b = rct.size.b - dObjs->paddleHOffs - dObjs->ballSize;
    dObjs->ball->setPos(ballPos);
    reflectV();
  }

  W::v2f &lPaddlePos = dObjs->lPaddle->pos;
  W::v2f &rPaddlePos = dObjs->rPaddle->pos;

  // L-hand tests
  if (ballPos.a <= dObjs->paddleHOffs + PADDLE_W) {
    if (ballPos.b + dObjs->ballSize >= lPaddlePos.b && ballPos.b < lPaddlePos.b + PADDLE_H*rct.size.b)
      reflectH(KLEFT);
    else
      incrementScore(KRIGHT), restart();
  }

  // R-hand tests
  else if (ballPos.a + dObjs->ballSize > rct.size.a - PADDLE_W - dObjs->paddleHOffs) {
    if (ballPos.b + dObjs->ballSize >= rPaddlePos.b && ballPos.b < rPaddlePos.b + PADDLE_H*rct.size.b)
      reflectH(KRIGHT);
    else
      incrementScore(KLEFT), restart();
  }
}

void PongView::startGame() {
  if (running) return;
  dObjs->transpOverlay->setCol(W::Colour(0,0,0,0));
  dObjs->hideStartText();

  running = true;
  restart();
}
void PongView::restart() {
  dObjs->performInitialLayout = true;
  resetBallVel();
  reflectCount = 0;
  dObjs->layOut(rct.size);
}

void PongView::incrementBallPos() {
  dObjs->ball->setPos(dObjs->ball->pos + ballVelocity);
}
void PongView::nudgePaddle(lr_enum lr, int dist) {
  W::v2f *paddle_pos;
  W::Rectangle *paddle;
  if (lr == KLEFT) {
    paddle_pos = &dObjs->lPaddle->pos;
    paddle = dObjs->lPaddle;
  }
  else {
    paddle_pos = &dObjs->rPaddle->pos;
    paddle = dObjs->rPaddle;
  }
  paddle_pos->b += dist;
  if (paddle_pos->b < dObjs->paddleMinYPos)      paddle_pos->b = dObjs->paddleMinYPos;
  else if (paddle_pos->b > dObjs->paddleMaxYPos) paddle_pos->b = dObjs->paddleMaxYPos;
  paddle->setPos(*paddle_pos);
}
void PongView::reflectH(lr_enum lr) {
  ballVelocity.a *= -1;

  // Increase speed every 2 bounces
  if(!(++reflectCount%2))
    ballVelocity.a += (ballVelocity.a > 0 ? 2 : -2);

  // Alter vertical velocity of ball depending on where it touches the paddle
  float paddle_height = PADDLE_H * rct.size.b;
  W::v2f *paddle_pos = &(lr == KLEFT ? dObjs->lPaddle->pos : dObjs->rPaddle->pos);
  int v_offset_ball   = dObjs->ball->pos.b + 0.5*dObjs->ballSize;
  int v_offset_paddle = paddle_pos->b + 0.5*paddle_height;
  float ball_offset_from_paddle = float(v_offset_ball - v_offset_paddle) / paddle_height;
  ballVelocity.b += ball_offset_from_paddle * 10;
}
void PongView::reflectV() {
  ballVelocity.b *= -1;
}
void PongView::incrementScore(lr_enum lr) {
  char s[5];
  sprintf(s, "%d", ++(lr == KLEFT ? scoreL : scoreR));
  (lr == KLEFT ? dObjs->lScore : dObjs->rScore)->setText(s);
}
