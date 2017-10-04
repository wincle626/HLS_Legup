int foo()
{
  int a = 1, b = 2, c = 3;
  return a + b + c;
}

int main()
{
  float a = 9000, b = 555;
  int c = a / b + foo();
  return c;
}
