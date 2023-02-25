#include <stdio.h>
int x = 3;
int add(){
  return x + 3;
}
int y = add();
printf("%d", y);