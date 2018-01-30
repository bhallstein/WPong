#ifndef __WLibTest__PongState__
#define __WLibTest__PongState__

#include "W.h"
#include "PongView.h"

class PongState : public W::GameState {
public:
  PongState()
  {
    v = new PongView();
    addView(v);
  }
  ~PongState()
  {
    removeView(v);
    delete v;
  }

  void resume(W::Returny *) { }
  void update() { v->update(); }

private:
  PongView *v;

};

#endif
