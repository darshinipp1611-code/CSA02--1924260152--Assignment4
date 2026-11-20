/* Task 5.2: veg and non-veg counters that must not interfere */
#include <stdio.h>
/* (a) automatic: cannot work - both restart at 1001 every call */
int veg_auto(void)    { int t = 1000; return ++t; }
int nonveg_auto(void) { int t = 2000; return ++t; }
/* (b) static locals: one extra function (or a kind parameter) */
int veg_static(void)    { static int t = 1000; return ++t; }
int nonveg_static(void) { static int t = 2000; return ++t; }
/* (c) global/extern: one extra global */
int veg_counter = 1000, nonveg_counter = 2000;
int veg_ext(void)    { return ++veg_counter; }
int nonveg_ext(void) { return ++nonveg_counter; }
/* (d) register: only a loop variable, issuing tokens is unaffected */
/* (e) pointer alternative: caller owns each counter */
int next_ptr(int *counter) { return ++*counter; }

int main(void)
{
    int a1 = veg_auto(), a2 = veg_auto(), b1 = nonveg_auto(), b2 = nonveg_auto();
    printf("auto     veg: %d %d | nonveg: %d %d  (both restart: WRONG)\n", a1, a2, b1, b2);
    int v1 = veg_static(), v2 = veg_static(), n1 = nonveg_static(), n2 = nonveg_static();
    printf("static   veg: %d %d | nonveg: %d %d\n", v1, v2, n1, n2);
    v1 = veg_ext(); v2 = veg_ext(); n1 = nonveg_ext(); n2 = nonveg_ext();
    printf("extern   veg: %d %d | nonveg: %d %d\n", v1, v2, n1, n2);
    int veg = 1000, nonveg = 2000;
    v1 = next_ptr(&veg); v2 = next_ptr(&veg); n1 = next_ptr(&nonveg); n2 = next_ptr(&nonveg);
    printf("pointer  veg: %d %d | nonveg: %d %d\n", v1, v2, n1, n2);
    int a = 1000, b = 1000;   /* independent counters: trivially supported by pointer version */
    v1 = next_ptr(&a); v2 = next_ptr(&a); n1 = next_ptr(&b);
    printf("pointer  two more independent counters: %d %d %d\n", v1, v2, n1);
    return 0;
}
