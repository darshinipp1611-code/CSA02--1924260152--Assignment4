/* Scope / lifetime / initial value demonstration for the four classes */
#include <stdio.h>
int g;                                   /* extern-style global: zero-initialised */
static int file_static;                  /* internal linkage */
void f(void)
{
    int a;                               /* auto: indeterminate (not printed) */
    static int s;                        /* static: 0 */
    register int r = 5;                  /* register: indeterminate unless set */
    a = 1; s++; (void)a; (void)r;
    printf("static s = %d, global g = %d, file_static = %d, &s = %p\n", s, g, file_static, (void *)&s);
}
int main(void) { f(); f(); f(); return 0; }
