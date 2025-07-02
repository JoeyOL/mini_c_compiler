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

int param8(int a, int b, int c, int d, int e, int f, int g, int h, char str[]) {
    printint(a); printint(b); printint(c); printint(d);
    printint(e); printint(f); printint(g); printint(h);
    for (int i = 0; str[i]; i++) {
        printchar(str[i]);
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