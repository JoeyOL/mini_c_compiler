int a[100][100];
int i;
int main() {
    for (i =0;i<100;i=1 +i) {
        a[0][i] = i;
    }
    for (i = 1; i < 100; i = i+1) {
        a[0][i] = a[0][i-1] + a[0][i];
    }
    printint(a[0][99]); 
    return 0;
}