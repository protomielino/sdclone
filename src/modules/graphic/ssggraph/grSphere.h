// grSphere.h
#include <plib/sg.h>
#include <plib/ssg.h>

// return a sphere object as an ssgBranch (connected to the specified ssgSimpleState)
ssgBranch *grMakeSphere(
  ssgSimpleState *state, ssgColourArray *cl, 
  float radius, int slices, int stacks,
  ssgCallback predraw, ssgCallback postdraw );

