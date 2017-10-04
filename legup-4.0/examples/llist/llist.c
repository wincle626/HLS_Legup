#include <stdio.h>
#include <stdlib.h>

// Tests a linked list node struct
// with struct pointers

struct record {
   volatile struct record * next;
   short a, b, c, d;
};

volatile struct record tail = {NULL, 1, 2, 3, 4};

int main()
{
   volatile struct record node3 = {&tail, 2, 5, 7, 34};
   volatile struct record node2 = {&node3, 34, 35, 27, 14};
   volatile struct record node1 = {&node2, 12, 54, 23, 334};
   volatile struct record head = {&node1, 24, 245, 537, 3434};
   int result = 0;
   volatile struct record * ptr = &head;

   while (ptr) {
       result += ptr->a + ptr->b + ptr->c + ptr->d;
       ptr = ptr->next;
   }

   printf("Result: %d\n", result);
   if (result == 4831) {
       printf("RESULT: PASS\n");
   } else {
       printf("RESULT: FAIL\n");
   }
   
   return result;
}

