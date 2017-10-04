
// Modified hailstone, in conventional form (extra division in odd number case)

// c99 -o hailstone_legup hailstone_legup.c

// Check correctness against the output of this program
// Only need to check the first few few values from each seed, as they are all sequentially dependent.
/**********************************
#! /usr/bin/python

import sys

n = int(sys.argv[1])
while (n > 1):
    if(n % 2 == 0):
        n = n / 2
    else:
        n = (3*n + 1) / 2
    print "%d " % n,
**********************************/

#define COUNT 100

volatile int input [] = {
333,   15093, 53956, 91327, 26294, 85971, 25760, 51582, 30794, 69334, 62299,
49438, 84916, 58898, 64309, 95439, 76368, 36062, 92253, 38435, 14227, 40480,
87357, 87055, 56934, 58240, 44037, 43602, 46250, 24175, 14299, 91354, 31251,
56785, 55811, 49030, 17973, 35340, 45723, 47437, 30536, 76451, 68232, 93312,
36248, 99951, 92797, 27659, 59184, 51654, 87317, 81803, 69681, 43028, 14176,
88215, 42476, 30393, 93081, 81433, 12647, 40314, 59206, 76654,  2331, 13004,
69549, 71920, 36328, 67928, 25851, 12980, 72936, 90323, 94762, 18764,   435,
86581,   402, 41511, 36071,  4237, 16356, 40304,  6110, 11919, 18517, 45699, 
34058, 16748, 49922, 18452, 34965,  8700, 81423, 37177,  6577, 12411, 58089, 
56872, -1
};

int main()
{
  
  int i;
  loop: for (i = 0; i < COUNT; i++)
  {
    int temp = input[i];
    int val1 = ((temp << 1) + temp + 1) >> 1;
    int val2 = temp >> 1;
    // (temp & 1) ? val1 : val2;

    temp = (temp & 1);
    // Assumes the carry bit has no effect
    temp = (0xFFFFFFFF + temp);

    // From Aaron Severance at VectorBlox
    // ECL: Not used to avoid multiplier
    //temp = (temp * 0xFFFFFFFF);
    //input[i] = (temp & val1) | ((~temp) & val2);

    // One cycle shorter in theory since no ~temp, via Henry Wong
    input[i] = (val1 ^ (temp & (val1 ^ val2)));

    // input[i] = temp;
    printf("%d\n", input[i]);
  }
  return input[COUNT-1];
}
