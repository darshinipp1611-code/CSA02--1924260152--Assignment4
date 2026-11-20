#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>

#define MAX_PATIENTS 100

struct date { int day, month, year; };
struct bloodpressure { int systolic, diastolic; };

struct patient {
    char name[50];
    int age;
    char phone[11];              /* 10 digits + '\0' */
    struct bloodpressure bp;     /* nested: 120/80 -> {120, 80} */
    struct date visit;           /* nested: dd/mm/yyyy */
};

struct patient_register {
    struct patient list[MAX_PATIENTS];
    int count;
};

/* returns 0 on success, -1 if register full, -2 on invalid data */
int addPatient(struct patient_register *r, const struct patient *p)
{
    if (r->count >= MAX_PATIENTS) return -1;
    if (p->age < 0 || p->age > 120 || strlen(p->phone) != 10) return -2;
    r->list[r->count] = *p;          /* one whole-structure copy */
    r->count++;
    return 0;
}

void sortByAge(struct patient_register *r)
{
    for (int i = 0; i < r->count - 1; i++) {
        int m = i;
        for (int j = i + 1; j < r->count; j++)
            if (r->list[j].age < r->list[m].age) m = j;
        if (m != i) { struct patient t = r->list[i]; r->list[i] = r->list[m]; r->list[m] = t; }
    }
}

static void show(const struct patient *p)
{
    printf("%-10s age %3d  ph %s  BP %d/%d  visit %02d/%02d/%04d\n", p->name, p->age,
           p->phone, p->bp.systolic, p->bp.diastolic, p->visit.day, p->visit.month, p->visit.year);
}

/* ---- Alternative: parallel arrays (structure of arrays) for timing ---- */
#define N 8000
static struct patient aos[N];
static int s_age[N]; static char s_name[N][50]; static char s_phone[N][11];
static struct bloodpressure s_bp[N]; static struct date s_vis[N];

static void sort_aos(void)
{
    for (int i = 0; i < N - 1; i++) {
        int m = i;
        for (int j = i + 1; j < N; j++) if (aos[j].age < aos[m].age) m = j;
        struct patient t = aos[i]; aos[i] = aos[m]; aos[m] = t;
    }
}
static void sort_soa(void)
{
    for (int i = 0; i < N - 1; i++) {
        int m = i;
        for (int j = i + 1; j < N; j++) if (s_age[j] < s_age[m]) m = j;
        int a = s_age[i]; s_age[i] = s_age[m]; s_age[m] = a;
        char nb[50]; memcpy(nb, s_name[i], 50); memcpy(s_name[i], s_name[m], 50); memcpy(s_name[m], nb, 50);
        char pb[11]; memcpy(pb, s_phone[i], 11); memcpy(s_phone[i], s_phone[m], 11); memcpy(s_phone[m], pb, 11);
        struct bloodpressure b = s_bp[i]; s_bp[i] = s_bp[m]; s_bp[m] = b;
        struct date d = s_vis[i]; s_vis[i] = s_vis[m]; s_vis[m] = d;
    }
}

int main(void)
{
    static struct patient_register reg;     /* static: zero-initialised, count = 0 */
    struct patient in[] = {
        {"Meena", 45, "9876543210", {120, 80}, {12, 10, 2026}},
        {"Arun",  23, "9123456780", {110, 70}, {12, 10, 2026}},
        {"Kavin", 67, "9000011111", {150, 95}, {13, 10, 2026}},
        {"Bad",  130, "12345",      {120, 80}, {13, 10, 2026}},   /* invalid */
    };
    for (int i = 0; i < 4; i++)
        printf("addPatient(%s) -> %d\n", in[i].name, addPatient(&reg, &in[i]));
    sortByAge(&reg);
    printf("--- sorted by age ---\n");
    for (int i = 0; i < reg.count; i++) show(&reg.list[i]);

    /* limit-check test */
    reg.count = MAX_PATIENTS;
    printf("full register -> %d\n", addPatient(&reg, &in[0]));

    printf("\nsizeof(struct patient) = %zu bytes\n", sizeof(struct patient));
    printf("offsets: name %zu age %zu phone %zu bp %zu visit %zu\n",
           offsetof(struct patient, name), offsetof(struct patient, age),
           offsetof(struct patient, phone), offsetof(struct patient, bp), offsetof(struct patient, visit));
    printf("original design per patient (parallel arrays) = %zu bytes\n",
           (size_t)(20 + sizeof(int) + 10 + sizeof(float) + 8));
    printf("SoA new per patient = %zu bytes\n",
           sizeof(int) + 50 + 11 + sizeof(struct bloodpressure) + sizeof(struct date));

    srand(1);
    for (int i = 0; i < N; i++) { aos[i].age = s_age[i] = rand() % 100; }
    clock_t t0 = clock(); sort_aos(); clock_t t1 = clock(); sort_soa(); clock_t t2 = clock();
    printf("sort %d patients: array-of-structures %.3f s (%zu bytes/swap), parallel arrays %.3f s (%zu bytes/swap in 5 moves)\n",
           N, (double)(t1 - t0) / CLOCKS_PER_SEC, sizeof(struct patient),
           (double)(t2 - t1) / CLOCKS_PER_SEC, sizeof(int) + 50 + 11 + sizeof(struct bloodpressure) + sizeof(struct date));
    return 0;
}
