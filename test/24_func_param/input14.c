int fred() {
  return(20);
}

int main() {
  int result;
  printint(10);
  result= fred();
  printint(result);
  printint(fred()+10);
  return(0);
}
