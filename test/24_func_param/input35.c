int fibo(int n) {
    if (n <= 1) {
        return n;
    }
    return fibo(n - 1) + fibo(n - 2);
}

int main() {
    int n = 10; 
    int result = fibo(n);
    printint(result); 
    return 0;
}