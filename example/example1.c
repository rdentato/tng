#define _(...) int TNG_F_(void)

int myfunction(int x)
{
  int p;

  _(":do your stuff");
  return 0;
}

_("after: do your stuff")
{
  printf("%d\n",p+x);
}

_("before: do your stuff")
{
  p=-12;
}


