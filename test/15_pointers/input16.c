float a = 3.14;
float i = 2.71;
float *b = &a + 1;
float c = *b; 
long d = 2147493647;
long *e = &d;
int f = 42;
int *g = &f;
int *h = &f;
int main() {
    printfloat(a);

    printfloat(c); 
    printfloat(*b + 1.0); 
    printfloat(*b - 1.0); 


    printlong(*e);


    printint(*g);

    printint(h == g);
    return 0;
}