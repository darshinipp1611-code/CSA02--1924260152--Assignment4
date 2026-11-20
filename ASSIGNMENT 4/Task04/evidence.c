/* Task 4.2: address evidence that addInterest() works on a copy (p.number fixed to p->number so it compiles) */
#include <stdio.h>
struct account { int number; float balance; };
void addInterest(struct account a, float rate) {
    printf("  inside addInterest : &a = %p, a.balance before = %.2f\n", (void *)&a, a.balance);
    a.balance = a.balance + a.balance * rate / 100;
    printf("  inside addInterest : a.balance after  = %.2f (copy only)\n", a.balance);
}
int main(void) {
    struct account list[3] = {{101, 5000}, {102, 12000}, {103, 800}};
    struct account *p = list;
    for (int i = 0; i < 3; i++) {
        printf("main: &list[%d] = %p\n", i, (void *)&list[i]);
        addInterest(list[i], 4);
        printf("main: list[%d].balance after call = %.2f\n", i, list[i].balance);
    }
    printf("%d %f\n", p->number, p->balance);
    return 0;
}
