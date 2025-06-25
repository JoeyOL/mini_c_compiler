#include <stdio.h>
// char *str = "Hello, World!";
void printint(int x) {
  printf("%d\n", x);
}

void printfloat(double x) {
  printf("%lf\n", x);
}

void printlong(long x) {
    printf("%ld\n", x);
}

void printchar(char x) {
  printf("%c", x);
}

// double readfloat() {
//   double x = 3.14;
//   return x;
// }

// int main() {
//   int d[5];
//   int a = 10;
//   int b = 20;
//   int *c = d; 
//   *c = 10;
//   printint(d[0]);
//   printint(a + b);
//   for (int i = 0; i < 5; i++) {
//       d[i] = i + 1;
//       if (i) {
//           int j = d[i];
//           printint(j); 
//       }
//   }
//   return 0;
// }