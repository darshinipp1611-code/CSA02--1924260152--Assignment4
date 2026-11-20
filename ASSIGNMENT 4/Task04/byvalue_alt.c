/* Alternative: pass by value and return the modified copy; plus timing of both styles */
#include <stdio.h>
#include <time.h>
struct account { int number; float balance; };
struct account addByValue(struct account a, float rate) { a.balance += a.balance * rate / 100; return a; }
void addByPtr(struct account *a, float rate) { a->balance += a->balance * rate / 100; }
struct big { struct account a; char pad[240]; };            /* 248-byte record to show copy cost grows */
struct big bigByValue(struct big b, float r) { b.a.balance += b.a.balance * r / 100; return b; }
void bigByPtr(struct big *b, float r) { b->a.balance += b->a.balance * r / 100; }
int main(void) {
    struct account list[3] = {{101, 5000}, {102, 12000}, {103, 800}};
    for (int i = 0; i < 3; i++) list[i] = addByValue(list[i], 4);
    for (int i = 0; i < 3; i++) printf("%d %.2f\n", list[i].number, list[i].balance);
    printf("sizeof(struct account) = %zu, copied twice per by-value call = %zu bytes; pointer = %zu bytes\n",
           sizeof(struct account), 2 * sizeof(struct account), sizeof(struct account *));
    printf("sizeof(struct big) = %zu, copied twice = %zu bytes\n", sizeof(struct big), 2 * sizeof(struct big));
    enum { R = 50000000 };
    volatile float sink = 0;
    struct account a = {1, 100}; struct big b = {{1, 100}, {0}};
    clock_t t0 = clock(); for (int i = 0; i < R; i++) { a = addByValue(a, 0.0001f); } sink += a.balance;
    clock_t t1 = clock(); for (int i = 0; i < R; i++) { addByPtr(&a, 0.0001f); } sink += a.balance;
    clock_t t2 = clock(); for (int i = 0; i < R / 5; i++) { b = bigByValue(b, 0.0001f); } sink += b.a.balance;
    clock_t t3 = clock(); for (int i = 0; i < R / 5; i++) { bigByPtr(&b, 0.0001f); } sink += b.a.balance;
    clock_t t4 = clock();
    printf("account (8 B): by value %.3f s, by pointer %.3f s (%d calls)\n", (double)(t1-t0)/CLOCKS_PER_SEC, (double)(t2-t1)/CLOCKS_PER_SEC, (int)R);
    printf("big (248 B)  : by value %.3f s, by pointer %.3f s (%d calls)\n", (double)(t3-t2)/CLOCKS_PER_SEC, (double)(t4-t3)/CLOCKS_PER_SEC, (int)R/5);
    return 0;
}
