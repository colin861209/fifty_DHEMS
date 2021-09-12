#ifndef __NEW_2D__
#define __NEW_2D__
#define NEW2D(H, W, TYPE) (TYPE **)new2d(H, W, sizeof(TYPE))

void* new2d(int h, int w, int size);

#endif