int main() {
    char d[5][5] = {1,2,3,4,5, {1,2,3,4,5}};
    int a = 10;
    int b = 20;
    printint(d[0][0]);
    printint(d[0][1]);
    printint(d[0][2]);
    printint(d[0][3]);
    printint(d[0][4]);
    printint(d[1][0]);

    char *c = d[0]; 
    *c = 10;
    printint(d[0][0]);
    printint(a + b);
    for (int i = 0; i < 5; i++) {
        d[i][i] = i + 1;
        if (i) {
            char *j = d[i];
            printint(*(j + i)); 
        }
    }
    return 0;
}