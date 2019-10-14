
int myfunction(int x)
{
  int p;
  // (:do your stuff)
  return 0;
}

// (after: do your stuff)
{
  printf("%d\n",p+x);
}

// (before: do your stuff)
{
  p=-12;
}


