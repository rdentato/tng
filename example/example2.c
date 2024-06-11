// These two macros could be defined in the header
// "tng.h" and the build script could remove the 
//    #include "tng.h"
// line from the tangled code. The IDE, instead, will
// (most probably) read the header and understand
// that CHUNK() creates a function scope.
// Unfortunately, variables that are local to
// myfunctions() will be undefined in the chunks.

#define HERE(...)
#define CHUNK(...) void TNG_##__LINE__(void)

int myfunction(int x) 
{
  int p;
  HERE(':do your stuff')
  return 0;
}

CHUNK('after: do your stuff')
{
  printf("%d\n",p+x); // p and x are uknown to the IDE :(
}

CHUNK("before: do your stuff")
{
  int y = 7;
  p=-12+y;
}


