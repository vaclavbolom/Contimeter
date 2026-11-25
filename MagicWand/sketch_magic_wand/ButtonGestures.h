#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

class ButtonGestures {
public:
  using Action = void (*)();

  struct Timing {
    uint32_t debounceMs;
    uint32_t shortPressMaxMs;
    uint32_t dotMs;
    uint32_t dashMs;
    uint32_t toleranceMs;
    uint32_t sequenceGapMs;
  };

  struct Hold {
    uint32_t durationMs;
    Action action;
  };

  struct Sequence {
    const char* pattern;
    Action action;
  };

  ButtonGestures(const Timing& timing, const Hold* holds, size_t holdCount,
                 const Sequence* sequences, size_t sequenceCount,
                 Action shortPress, Action holdReleased)
      : timing_(timing), holds_(holds), holdCount_(holdCount),
        sequences_(sequences), sequenceCount_(sequenceCount),
        shortPress_(shortPress), holdReleased_(holdReleased) {}

  bool isPressed() const { return pressed_; }

  void update(bool pressed, uint32_t now) {
    if (pressed != rawPressed_) {
      rawPressed_ = pressed;
      rawChangedAt_ = now;
    }

    if (rawPressed_ != pressed_ &&
        elapsed(now, rawChangedAt_) >= timing_.debounceMs) {
      pressed_ = rawPressed_;
      if (pressed_) {
        if (sequenceLength_ &&
            elapsed(rawChangedAt_, releasedAt_) >= timing_.sequenceGapMs) {
          finishSequence();
        }
        pressedAt_ = rawChangedAt_;
        nextHold_ = 0;
      } else {
        const uint32_t duration = elapsed(rawChangedAt_, pressedAt_);        
        fireHolds(duration);
        releasedAt_ = rawChangedAt_;
        if (nextHold_) {
          resetSequence();
          if (holdReleased_) {
            holdReleased_();
          }
        } else if (duration < timing_.shortPressMaxMs) {
          resetSequence();
          if (shortPress_) {
            shortPress_();
          }
        } else if (matchesDuration(duration, timing_.dotMs)) {
          appendSymbol('.');
        } else if (matchesDuration(duration, timing_.dashMs)) {
          appendSymbol('-');
        } else {
          resetSequence();
        }
        Serial.print("Duration: ");
        Serial.println(duration);
      }
    }

    // Do not count release debounce time as additional hold time.
    if (pressed_ && rawPressed_) {
      fireHolds(elapsed(now, pressedAt_));
    }
    if (!pressed_ && !rawPressed_ && sequenceLength_ &&
        elapsed(now, releasedAt_) >= timing_.sequenceGapMs) {
      finishSequence();
    }
  }

private:
  static uint32_t elapsed(uint32_t now, uint32_t then) {
    return now - then;
  }

  bool matchesDuration(uint32_t duration, uint32_t target) const {
    const uint32_t difference =
        duration > target ? duration - target : target - duration;
    return difference <= timing_.toleranceMs;
  }

  void fireHolds(uint32_t duration) {
    // Hold bindings are ordered by increasing duration and fire once per press.
    while (nextHold_ < holdCount_ && duration >= holds_[nextHold_].durationMs) {
      resetSequence();
      const Action action = holds_[nextHold_++].action;
      if (action) {
        action();
      }
    }
  }

  void appendSymbol(char symbol) {
    if (sequenceOverflow_) {
      return;
    }
    if (sequenceLength_ == sizeof(sequence_) - 1) {
      sequenceOverflow_ = true;
      return;
    }
    sequence_[sequenceLength_++] = symbol;
    sequence_[sequenceLength_] = '\0';
  }

  void finishSequence() {
    Action action = nullptr;
    if (!sequenceOverflow_) {
      for (size_t i = 0; i < sequenceCount_; ++i) {
        if (strcmp(sequence_, sequences_[i].pattern) == 0) {
          action = sequences_[i].action;
          break;
        }
      }
    }
    resetSequence();
    if (action) {
      action();
    }
  }

  void resetSequence() {
    sequenceLength_ = 0;
    sequence_[0] = '\0';
    sequenceOverflow_ = false;
  }

  Timing timing_;
  const Hold* holds_;
  size_t holdCount_;
  const Sequence* sequences_;
  size_t sequenceCount_;
  Action shortPress_;
  Action holdReleased_;
  bool rawPressed_ = false;
  bool pressed_ = false;
  uint32_t rawChangedAt_ = 0;
  uint32_t pressedAt_ = 0;
  uint32_t releasedAt_ = 0;
  size_t nextHold_ = 0;
  char sequence_[9] = {};
  size_t sequenceLength_ = 0;
  bool sequenceOverflow_ = false;
};
