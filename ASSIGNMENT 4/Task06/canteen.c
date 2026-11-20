/* College Canteen Inventory and Billing - Task 6 */
#include <stdio.h>
#include <string.h>

#define MAX_ITEMS 50
#define WARN_DAYS 2

struct date { int day, month, year; };
struct time_hm { int hour, minute; };

enum item_kind { PACKAGED, COOKED, COMBO };

struct item {
    int id;
    char name[24];
    enum item_kind kind;                 /* tag */
    float price;
    int stock;                           /* unused for COMBO (derived from components) */
    int units_sold;
    float revenue;
    union {
        struct date expiry;                          /* PACKAGED */
        struct time_hm prepared;                     /* COOKED   */
        struct { int count; int item_id[3]; } combo; /* COMBO: up to 3 other items */
    } u;
};

struct canteen {
    struct item items[MAX_ITEMS];
    int count;
};

/* global business date: read-only shared data, set once */
static const struct date TODAY = {19, 11, 2026};

/* status codes */
enum { SALE_OK = 0, ERR_NO_ITEM = -1, ERR_BAD_QTY = -2, ERR_STOCK = -3, ERR_EXPIRED = -4 };

/* prototypes (structure pointers) */
int  add_item(struct canteen *c, const struct item *it);
struct item *find_item(struct canteen *c, int id);
int  days_between(const struct date *from, const struct date *to);
int  record_sale(struct canteen *c, int id, int qty, const struct date *today);
void expiry_warning(const struct canteen *c, const struct date *today);
void print_summary(const struct canteen *c);

int add_item(struct canteen *c, const struct item *it)
{
    if (c->count >= MAX_ITEMS) return -1;
    c->items[c->count++] = *it;
    return 0;
}

struct item *find_item(struct canteen *c, int id)
{
    for (int i = 0; i < c->count; i++)
        if (c->items[i].id == id) return &c->items[i];
    return NULL;
}

/* days since 1970-01-01 (civil calendar algorithm) so dates subtract correctly across months/years */
static long day_number(const struct date *d)
{
    int y = d->year, m = d->month;
    y -= m <= 2;
    long era = (y >= 0 ? y : y - 399) / 400;
    long yoe = y - era * 400;
    long doy = (153L * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d->day - 1;
    long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

int days_between(const struct date *from, const struct date *to)
{
    return (int)(day_number(to) - day_number(from));
}

int record_sale(struct canteen *c, int id, int qty, const struct date *today)
{
    static int bill_no = 0;                       /* static: persists across calls */
    struct item *it = find_item(c, id);
    if (!it) return ERR_NO_ITEM;
    if (qty <= 0) return ERR_BAD_QTY;

    if (it->kind == PACKAGED) {
        if (days_between(today, &it->u.expiry) < 0) return ERR_EXPIRED;
        if (it->stock < qty) return ERR_STOCK;
        it->stock -= qty;                          /* validate first, then update */
    } else if (it->kind == COOKED) {
        if (it->stock < qty) return ERR_STOCK;
        it->stock -= qty;
    } else {                                       /* COMBO: all components must be available */
        for (int k = 0; k < it->u.combo.count; k++) {
            struct item *comp = find_item(c, it->u.combo.item_id[k]);
            int need = 0;
            for (int j = 0; j < it->u.combo.count; j++)
                if (it->u.combo.item_id[j] == it->u.combo.item_id[k]) need += qty;
            if (!comp || comp->stock < need) return ERR_STOCK;
            if (comp->kind == PACKAGED && days_between(today, &comp->u.expiry) < 0) return ERR_EXPIRED;
        }
        for (int k = 0; k < it->u.combo.count; k++)      /* commit */
            find_item(c, it->u.combo.item_id[k])->stock -= qty;
    }
    it->units_sold += qty;
    it->revenue += it->price * qty;
    bill_no++;
    printf("  Bill #%03d: %-14s x%-3d @ %6.2f = %7.2f\n", bill_no, it->name, qty, it->price, it->price * qty);
    return SALE_OK;
}

void expiry_warning(const struct canteen *c, const struct date *today)
{
    int found = 0;
    for (int i = 0; i < c->count; i++) {
        const struct item *it = &c->items[i];
        if (it->kind != PACKAGED) continue;
        int d = days_between(today, &it->u.expiry);
        if (d <= WARN_DAYS) {
            found = 1;
            if (d < 0) printf("  EXPIRED  : %-12s expired %02d/%02d/%04d (%d day(s) ago), stock %d\n", it->name,
                              it->u.expiry.day, it->u.expiry.month, it->u.expiry.year, -d, it->stock);
            else printf("  EXPIRING : %-12s expires %02d/%02d/%04d (in %d day(s)), stock %d\n", it->name,
                        it->u.expiry.day, it->u.expiry.month, it->u.expiry.year, d, it->stock);
        }
    }
    if (!found) printf("  No items expiring within %d days.\n", WARN_DAYS);
}

void print_summary(const struct canteen *c)
{
    const struct item *order[MAX_ITEMS];                 /* sort pointers, not whole structures */
    for (int i = 0; i < c->count; i++) order[i] = &c->items[i];
    for (int i = 0; i < c->count - 1; i++) {
        int m = i;
        for (int j = i + 1; j < c->count; j++)
            if (order[j]->revenue > order[m]->revenue) m = j;
        const struct item *t = order[i]; order[i] = order[m]; order[m] = t;
    }
    float total = 0;
    printf("  %-4s %-14s %6s %9s\n", "ID", "Item", "Sold", "Revenue");
    for (int i = 0; i < c->count; i++) {
        printf("  %-4d %-14s %6d %9.2f\n", order[i]->id, order[i]->name, order[i]->units_sold, order[i]->revenue);
        total += order[i]->revenue;
    }
    printf("  %-19s %6s %9.2f\n", "TOTAL", "", total);
}

static void report(int id, int qty, int rc)
{
    static const char *msg[] = {"OK", "item not found", "invalid quantity", "INSUFFICIENT STOCK - sale refused", "item EXPIRED - sale refused"};
    if (rc != SALE_OK) printf("  Sale of item %d x%d: %s\n", id, qty, msg[-rc]);
}

int main(void)
{
    static struct canteen c;                              /* static: large, zero-initialised */
    struct item list[] = {
        {101, "Biscuits", PACKAGED, 10.0f, 30, 0, 0, {.expiry = {20, 11, 2026}}},
        {102, "Juice", PACKAGED, 25.0f, 12, 0, 0, {.expiry = {21, 11, 2026}}},
        {103, "Chips", PACKAGED, 20.0f, 15, 0, 0, {.expiry = {30, 11, 2026}}},
        {104, "Samosa", COOKED, 15.0f, 20, 0, 0, {.prepared = {9, 30}}},
        {105, "Meal Combo", COMBO, 40.0f, 0, 0, 0, {.combo = {3, {104, 102, 101}}}},
        {106, "Milk", PACKAGED, 22.0f, 5, 0, 0, {.expiry = {18, 11, 2026}}},
    };
    for (int i = 0; i < 6; i++) add_item(&c, &list[i]);
    printf("Date: %02d/%02d/%04d\nsizeof(struct item) = %zu, sizeof(struct canteen) = %zu\n\n",
           TODAY.day, TODAY.month, TODAY.year, sizeof(struct item), sizeof(struct canteen));

    printf("[Test 1] Normal sales\n");
    report(104, 5, record_sale(&c, 104, 5, &TODAY));
    report(101, 4, record_sale(&c, 101, 4, &TODAY));
    report(103, 2, record_sale(&c, 103, 2, &TODAY));
    report(105, 3, record_sale(&c, 105, 3, &TODAY));

    printf("\n[Test 2] Sale exceeding stock (Juice stock now %d)\n", find_item(&c, 102)->stock);
    report(102, 20, record_sale(&c, 102, 20, &TODAY));
    printf("  Juice stock unchanged: %d\n", find_item(&c, 102)->stock);

    printf("\n[Test 3] Expired item, invalid quantity, unknown item\n");
    report(106, 1, record_sale(&c, 106, 1, &TODAY));
    report(103, 0, record_sale(&c, 103, 0, &TODAY));
    report(999, 1, record_sale(&c, 999, 1, &TODAY));

    printf("\n[Test 4] Combo with an unavailable component\n");
    find_item(&c, 104)->stock = 2;
    report(105, 3, record_sale(&c, 105, 3, &TODAY));
    printf("  Samosa stock unchanged (all-or-nothing): %d\n", find_item(&c, 104)->stock);

    printf("\n[Test 5] Expiry warning (within %d days)\n", WARN_DAYS);
    expiry_warning(&c, &TODAY);

    printf("\n[Test 6] Day's bill summary sorted by revenue\n");
    print_summary(&c);
    return 0;
}
