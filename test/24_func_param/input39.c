
int main()
{
	int i, j, t, n, temp;
	int mult1[20] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
	int mult2[20] = {2, 3, 4, 2, 5, 7 ,9 ,9, 0, 1, 9, 8, 7, 6, 4, 3, 2, 1, 2, 2};
	int len1 = 20;
	int len2 = 20;
	int c1[25];
	int c2[25];
	int result[40] = {};

	i = 0;
	while (i < len1) {
		c1[i] = mult1[i];
		i = i + 1;
	}	

	i = 0;
	while (i < len2) {
		c2[i] = mult2[i];
		i = i + 1;
	}	

	n = len1 + len2 - 1;

	i = 0;
	while (i <= n) {
		result[i]=0;
		i = i + 1;
	}	 

	temp=0;

	i = len2 - 1;
	while (i > -1) {
		t = c2[i];
		j = len1 - 1;
		while (j > -1) {
			temp = result[n] + t * c1[j];
			if(temp >= 10) {
				result[n] = (temp);
				result[n-1] = result[n-1] + temp / 10;
			}
			else
				result[n] = temp;
			j = j - 1;
			n = n - 1;
		}
		n = n + len1 - 1;
		i = i - 1;
	}

	if(result[0] != 0)
		printint(result[0]); 

	i = 1;
	while (i <= len1 + len2 - 1) {
		printint(result[i]); 
		i = i + 1;
	}

	return 0;
}
