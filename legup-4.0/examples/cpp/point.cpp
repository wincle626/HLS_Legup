// Actually a cpp file, with stdio.h and printf

#include <stdio.h>

class Point {
    int x, y;
  public:
    Point() { x = 0; y = 0; }
    Point(int x) { setX(x); y = 0; };
    Point(int x, int y) { setXY(x, y); };
    int getX() { return x; };
    void setX(int x) { this->x = x; };
    int getY() { return y; };
    void setY(int y) { this->y = y; };
    void setXY(int x, int y) { this->x = x; this->y = y; };
};

int main() {
  Point a(3, 5), array[5];
  volatile int i;
  i = 0;
  Point &b = array[i];
  i = 1;
  Point &c = array[i];

  b.setXY(2, -4);
  printf("(%d,%d) (%d,%d) (%d,%d)", a.getX(), a.getY(), b.getX(), b.getY(), c.getY(), c.getX());

  int result = (a.getX() == 3) + (a.getY() == 5) + (b.getX() == 2) + (b.getY()
          == -4) + (c.getX() == 0) + (c.getY() == 0);
  printf("Result: %d\n", result);
  if (result == 6) {
      printf("RESULT: PASS\n");
  } else {
      printf("RESULT: FAIL\n");
  }
  return result;
}
