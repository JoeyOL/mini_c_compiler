int buf[2][100];
void merge_sort(int l, int r)
{
    if (l + 1 >= r){
        return;
    }

    int mid = (l + r) / 2;
    merge_sort(l, mid);
    merge_sort(mid, r);

    int i = l, j = mid, k = l;
    while (j < r) {
        if (i >= mid) {
            break;
        }
        if (buf[0][i] < buf[0][j]) {
            buf[1][k] = buf[0][i];
            i = i + 1;
        } else {
            buf[1][k] = buf[0][j];
            j = j + 1;
        }
        k = k + 1;
    }
    while (i < mid) {
        buf[1][k] = buf[0][i];
        i = i + 1;
        k = k + 1;
    }
    while (j < r) {
        buf[1][k] = buf[0][j];
        j = j + 1;
        k = k + 1;
    }

    while (l < r) {
        buf[0][l] = buf[1][l];
        l = l + 1;
    }
}

int main()
{
    int n = 10;
    buf[0][0] = 4; buf[0][1] = 3; buf[0][2] = 9; buf[0][3] = 2; buf[0][4] = 0;
    buf[0][5] = 1; buf[0][6] = 6; buf[0][7] = 5; buf[0][8] = 7; buf[0][9] = 8;
    merge_sort(0, n);
    printarray(n, buf[0]);
    return 0;
}
