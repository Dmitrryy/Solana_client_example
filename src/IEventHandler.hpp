#pragma once

enum class EventTy {
  INVOKE,
  NOTHING,
  ERROR,
};

class IEventHandler {
public:
  virtual void handleEvent(EventTy event) = 0;
  virtual ~IEventHandler() {}
};
