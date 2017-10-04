
// Increment an array +1, 10 times.
// Bounded by array size count

#define COUNT 10

volatile int array [COUNT+1] = {0,1,2,3,4,5,6,7,8,9,-1};

int main()
{
  int i, j;
  for (i = 0; i < COUNT; i++)
 loop: for (j = 0; j < COUNT; j++) {
      array[j]++;
    }
  
  return array[COUNT-1];
}
