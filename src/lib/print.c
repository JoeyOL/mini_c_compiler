#include <stdio.h>
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

void printarray(int n, int arr[]) {
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

void printfloatarray(int n, double arr[]) {
    for (int i = 0; i < n; i++) {
        printf("%lf ", arr[i]);
    }
    printf("\n");
}