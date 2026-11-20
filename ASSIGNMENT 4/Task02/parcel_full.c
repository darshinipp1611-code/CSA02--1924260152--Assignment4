/* Full Task 2 model: complete parcel entity, access paths, city listing, flat comparison */
#include <stdio.h>
#include <string.h>
typedef struct { int day, month, year; } Date;
typedef struct { char name[30]; char city[20]; int pin; } Party;       /* sender / receiver */
typedef enum { NORMAL, EXPRESS } Service;
typedef struct { int id; float weight; Date booked; Party sender, receiver; Service service; float charge; } Parcel;

/* flat alternative */
typedef struct { int id; float weight; int bd, bm, by;
    char sname[30], scity[20]; int spin; char rname[30], rcity[20]; int rpin; Service service; float charge; } FlatParcel;

#define MAXP 100
static Parcel today[MAXP];

static void sortByWeight(Parcel *p, int n)
{ for (int i = 0; i < n - 1; i++) { int m = i;
    for (int j = i + 1; j < n; j++) if (p[j].weight < p[m].weight) m = j;
    Parcel t = p[i]; p[i] = p[m]; p[m] = t; } }

static void listByCity(const Parcel *p, int n, const char *city)
{ for (int i = 0; i < n; i++) if (strcmp(p[i].receiver.city, city) == 0)
    printf("  parcel %d  %.1f kg  to %s, %s-%d\n", p[i].id, p[i].weight, p[i].receiver.name, p[i].receiver.city, p[i].receiver.pin); }

int main(void)
{
    Parcel seed[4] = {
        {501, 4.5f, {12,10,2026}, {"Ravi","Madurai",625001}, {"Anu","Chennai",600077}, EXPRESS, 120.0f},
        {502, 1.2f, {12,10,2026}, {"Sita","Madurai",625001}, {"Bala","Coimbatore",641001}, NORMAL, 40.0f},
        {503, 2.8f, {13,10,2026}, {"Kumar","Salem",636001}, {"Devi","Chennai",600001}, NORMAL, 70.0f},
        {504, 0.5f, {13,10,2026}, {"Lata","Trichy",620001}, {"Mani","Madurai",625002}, EXPRESS, 55.0f}};
    int n = 4; memcpy(today, seed, sizeof seed);
    sortByWeight(today, n);
    printf("Sorted by weight:\n"); for (int i = 0; i < n; i++) printf("  %d %.1f\n", today[i].id, today[i].weight);
    printf("Parcels for Chennai:\n"); listByCity(today, n, "Chennai");
    Parcel *p = today;
    printf("3rd parcel receiver PIN: (i) today[2].receiver.pin = %d  (ii) (p+2)->receiver.pin = %d\n",
           today[2].receiver.pin, (p + 2)->receiver.pin);
    printf("sizeof(Date)=%zu sizeof(Party)=%zu sizeof(Parcel)=%zu sizeof(FlatParcel)=%zu\n",
           sizeof(Date), sizeof(Party), sizeof(Parcel), sizeof(FlatParcel));
    struct addr { char city[20]; int pincode; }; struct par { int id; float weight; struct addr to; struct { int d,m,y; } booked; };
    printf("Complete-the-code struct parcel: sizeof = %zu (bytes copied per swap, 3 copies = %zu)\n", sizeof(struct par), 3 * sizeof(struct par));
    return 0;
}
