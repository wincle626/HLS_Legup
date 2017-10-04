
// Generalized swap with 4 steps.
// Parallel(?) description over 2 memories

volatile int array1 [] = {
 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
43, 44, 45, 46, 47, 48, 49, 50 };
 
volatile int array2 [] = {
51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90,
91, 92, 93, 94, 95, 96, 97, 98, 99, 100 };

int main()
{
  int i;
 loop:  for (i = 0; i < 50; i++) {
    int temp1   = array1[i];
    int temp2   = array2[49-i];
    array2[49-i] = temp1;
    array1[i]    = temp2;
  }

  return array2[49];
}

