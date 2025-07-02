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

extern int foo(int a, int b);
extern int foo2(int* a, int* b);
extern int foo3(int a[], int b[]);
extern int param8(int a, int b, int c, int d, int e, int f, int g, int h, char str[]);
extern int param5(int a, int b, int c, int d, int e);
extern int param2(int a, int b);
extern int param0();
extern double foo4(double a, double b);
extern double foo5(double* a, double* b);
extern double foo6(double a[][2], double b[][2]);

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

int main() {
    int a = 1, b = 2;
    int arr1[2] = {3, 4};
    int arr2[2] = {5, 6};

    printint(foo(a, b));
    printint(foo2(&arr1[0], &arr2[0]));
    printint(foo3(arr1, arr2));

    double f1 = 1.5, f2 = 2.5;
    printfloat(foo4(f1, f2));
    printfloat(foo5(&f1, &f2));
    double arr3[2][2] = {{1.0, 2.0}, {3.0, 4.0}};
    double arr4[2][2] = {{5.0, 6.0}, {7.0, 8.0}};
    printfloat(arr3[0][0] + arr4[0][0]);
    printfloat(foo6(arr3, arr4));

    char str[] = "Hello World!\n";
    printint(param8(1, 2, 3, 4, 5, 6, 7, 8, str));
    printint(param5(1, 2, 3, 4, 5));
    printint(param2(1, 2));
    printint(param0());

    return 0; 
}