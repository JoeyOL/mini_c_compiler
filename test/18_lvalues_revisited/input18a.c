int   a;
int e;
int  *b;
char  c;
char *d;

int main()
{
  b= &a; *(b + 1) = 15; printint(e);
  d= &c; *d= 16; printint(c);

  for (*b = 1; *b < 10; *b = *b + 1) {
    printint(*b);
  }
  printint(a);
  return(0);
}
