/* Task 4.3: repaired program - pass by pointer, arrow operator */
#include <stdio.h>
struct account { int number; float balance; };
void addInterest(struct account *a, float rate) {
    printf("  inside addInterest : a = %p\n", (void *)a);
    a->balance = a->balance + a->balance * rate / 100;
}
int main(void) {
    struct account list[3] = {{101, 5000}, {102, 12000}, {103, 800}};
    struct account *p = list;
    for (int i = 0; i < 3; i++) { printf("main: &list[%d] = %p\n", i, (void *)&list[i]); addInterest(&list[i], 4); }
    for (int i = 0; i < 3; i++) printf("%d %.2f\n", (p + i)->number, (p + i)->balance);
    return 0;
}
