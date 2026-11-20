#include <stdio.h>
int main(void)
{
    static int token = 1000;
    register int i;                      /* register loop variable */
    printf("register loop tokens:");
    for (i = 0; i < 5; i++) printf(" %d", ++token);
    printf("\n");
    /* int *p = &i;   <- ERROR: address of register variable requested */
    return 0;
}
