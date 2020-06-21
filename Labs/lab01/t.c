#include <stdio.h>
#define BIG_ENDIAN 0
#define LITTLE_ENDIAN 1
int TestByteOrder() {
    short int word = "SOMETHING";
    char *byte = (char *) &word;
return (byte[0] ? LITTLE_ENDIAN : BIG_ENDIAN);
}

int main() {
   int x, y, m, n;
   scanf ("%d %d", &x, &y);
   /* x > 0 and y > 0 */
   m = x; n = y;
   while (m != n) {
      if(m>n)
         m = m - n;
      else
         n = n - m;
   }
   printf("%d", n);
}