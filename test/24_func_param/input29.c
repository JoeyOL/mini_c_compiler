int foo(int a, int b) {
    return a + b;
}

int foo2(int* a, int* b) {
    return *a + *b;
}

int foo3(int a[], int b[]) {
    return a[0] + b[0];
}

float foo4(float a, float b) {
    return a + b;
}

float foo5(float* a, float* b) {
    return *a + *b;
}

float foo6(float a[][2], float b[][2]) {
    return a[0][0] + b[0][0];
}

int param8(int a, int b, int c, int d, int e, int f, int g, int h, char *str) {
    printint(a); printint(b); printint(c); printint(d);
    printint(e); printint(f); printint(g); printint(h);
    for (; *str; str++) {
        printchar(*str);
    }
    return(0);
  }
  
  int param5(int a, int b, int c, int d, int e) {
    printint(a); printint(b); printint(c); printint(d); printint(e);
    return(0);
  }
  
  int param2(int a, int b) {
    int c; int d; int e;
    c= 3; d= 4; e= 5;
    printint(a); printint(b); printint(c); printint(d); printint(e);
    return(0);
  }
  
  int param0() {
    int a; int b; int c; int d; int e;
    a= 1; b= 2; c= 3; d= 4; e= 5;
    printint(a); printint(b); printint(c); printint(d); printint(e);
    return(0);
  }

int main() {
    int a = 1, b = 2;
    int arr1[2] = {3, 4};
    int arr2[2] = {5, 6};

    printint(foo(a, b));
    printint(foo2(&arr1[0], &arr2[0]));
    printint(foo3(arr1, arr2));

    float f1 = 1.5, f2 = 2.5;
    printfloat(foo4(f1, f2));
    printfloat(foo5(&f1, &f2));
    float arr3[2][2] = {{1.0, 2.0}, {3.0, 4.0}};
    float arr4[2][2] = {{5.0, 6.0}, {7.0, 8.0}};
    printfloat(arr3[0][0] + arr4[0][0]);
    printfloat(foo6(arr3, arr4));

    char *str = "Hello World!\n";
    printint(param8(1, 2, 3, 4, 5, 6, 7, 8, str));
    printint(param5(1, 2, 3, 4, 5));
    printint(param2(1, 2));
    printint(param0());

    return 0; 
}