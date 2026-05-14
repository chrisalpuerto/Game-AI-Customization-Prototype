#pragma once

#include <algorithm>

struct SimPoint {
  int x = 0;
  int y = 0;
};

struct SimRect {
  int left = 0;
  int top = 0;
  int right = 0;
  int bottom = 0;
};

inline void SetRectPortable(SimRect* rect, int left, int top, int right, int bottom) {
  rect->left = left;
  rect->top = top;
  rect->right = right;
  rect->bottom = bottom;
}

inline bool PtInRectPortable(const SimRect* rect, SimPoint pt) {
  return pt.x >= rect->left && pt.x <= rect->right && pt.y >= rect->top && pt.y <= rect->bottom;
}

inline bool RectsOverlapPortable(const SimRect& a, const SimRect& b) {
  const bool horizontal = a.left <= b.right && b.left <= a.right;
  const bool vertical = a.top <= b.bottom && b.top <= a.bottom;
  return horizontal && vertical;
}

