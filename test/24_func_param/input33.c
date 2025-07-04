int n;
int swap (float array[], int i, int j){
    float temp;
    temp = array[i];
    array[i] = array[j];
    array[j] = temp;
    return 0;  
}
int heap_ajust(float arr[], int start, int end) {  
    int dad;
    dad = start;  
    int son;
    son = dad * 2 + 1;  
    while (son < end + 1) { 
        if (son < end) {
            if (arr[son] < arr[son + 1]) {
                son = son + 1;  
            }
        }
        if (arr[dad] > arr[son]) {
            return 0;  

        }
        else {
            dad = swap(arr,dad,son);  
            dad = son;  
            son = dad * 2 + 1;  
        }  
    }  
    return 0;  
}  
int heap_sort(float arr[], int len) {  
    int i;  
    int tmp;
    i = len / 2 - 1;
    while ( i > -1) {
        tmp = len - 1;
        tmp = heap_ajust(arr, i, tmp);  
        i = i - 1;
    }    
    i = len - 1;   
    while ( i > 0) {  
        int tmp0;
        tmp0 = 0;
        tmp = swap(arr,tmp0,i);
        tmp = i - 1;
        tmp = heap_ajust(arr, tmp0, tmp);  
        i = i-1;
    }  
    return 0;
}  

int main(){
    n = 10;
    float a[10];
    a[0]=4.5;a[1]=3.5;a[2]=9.5;a[3]=2.5;a[4]=0.5;
    a[5]=1.9;a[6]=6.1;a[7]=5.6;a[8]=7.6;a[9]=8.5;
    int i;
    i = 0;
    i = heap_sort(a, n);
    printfloatarray(n, a);
    return 0;
}
