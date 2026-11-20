#include <stdio.h>
extern int token_counter;          /* declaration: defined in extern_counter.c */
int next_token_ext(void);          /* defined in extern_counter.c */
void other_source_issue(void);     /* second use in same file for demo */
int main(void)
{
    int e1 = next_token_ext(), e2 = next_token_ext();
    printf("extern: %d %d\n", e1, e2);
    token_counter += 10;           /* ANY file can modify it - accidental modification risk */
    printf("after other file bumps counter by 10: %d\n", next_token_ext());
    return 0;
}
