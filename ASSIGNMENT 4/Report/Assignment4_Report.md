---
title: "CSA02 – C Programming: Assignment 4 Report"
subtitle: "Modelling Real-World Data with Structures and Unions (CO4)"
---

**Name:** <YOUR NAME>  **Register No.:** <YOUR REGISTER NUMBER>  **Compiler:** gcc 13.3.0, `-Wall -Wextra`

**GitHub repository:** https://github.com/<your-id>/CSA02-REGNO-Assignment4

*Screenshots: after running each program, paste a terminal screenshot where marked "Screenshot". The text outputs shown are the real outputs of the submitted programs.*

# Task 1 – Data Design Review (6 marks)

## 1.1 Diagnosis

| # | Deficiency | What could go wrong at the health centre |
|---|---|---|
| 1 | Parallel arrays (`name[]`, `age[]`, ...) | Fields of one patient are linked only by index. Sorting one array (e.g. age) without moving the others attaches Meena's age to Arun's name -> wrong treatment records. |
| 2 | `char phone[100][10]` | A 10-digit number needs 11 bytes with `'\0'`; `strcpy` overflows into the next phone, corrupting a neighbour's number. |
| 3 | `float bp` for "120/80" | Two values in one number. `120/80` in C is integer division = 1; no way to get systolic/diastolic, so hypertension cannot be detected. Floats also cannot store 120 and 80 separately. |
| 4 | `char visitDate[100][8]` | "12102026" is 8 characters + `'\0'` = 9 bytes -> overflow. Text dates cannot be compared, sorted or checked (month 13 accepted). |
| 5 | No limit check on `count` | The 101st patient writes beyond all arrays (undefined behaviour, crash or corrupted records). |
| 6 | Global variables | Any function can change `count` or arrays; two registers (e.g. two clinics) are impossible; hard to test. |
| 7 | Sorting by age is awkward | Every array must be swapped by hand (5 swaps per exchange) - easy to forget one. |
| 8 | `name[20]` too short, no validation | Long names overflow; invalid ages (-5, 400) accepted. |

## 1.2 Repaired design

```c
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
```

Memory layout of `struct patient` (measured, 64-bit GCC):

```
offset  0 ..49  name[50]
offset 50..51   padding (2)
offset 52..55   age
offset 56..66   phone[11]
offset 67       padding (1)
offset 68..75   bp {systolic, diastolic}
offset 76..87   visit {day, month, year}
sizeof(struct patient) = 88
```

Output (Screenshot: Task01/output_patient.png):

```
addPatient(Meena) -> 0
addPatient(Arun) -> 0
addPatient(Kavin) -> 0
addPatient(Bad) -> -2
--- sorted by age ---
Arun       age  23  ph 9123456780  BP 110/70  visit 12/10/2026
Meena      age  45  ph 9876543210  BP 120/80  visit 12/10/2026
Kavin      age  67  ph 9000011111  BP 150/95  visit 13/10/2026
full register -> -1

sizeof(struct patient) = 88 bytes
offsets: name 0 age 52 phone 56 bp 68 visit 76
original design per patient (parallel arrays) = 46 bytes
SoA new per patient = 85 bytes
sort 8000 patients: array-of-structures 0.096 s (88 bytes/swap), parallel arrays 0.059 s (85 bytes/swap in 5 moves)
```

Pointer operations: `r->count` (arrow, `r` is a pointer); `r->list[r->count] = *p` copies the whole structure; `p->bp.systolic` combines arrow and dot for the nested structure.

## 1.3 Design defence (<100 words)

Each patient is one object, so sorting swaps one structure (`t = a[i]; a[i] = a[j]; a[j] = t`) and no field can be left behind. Passing `&patient` sends one pointer instead of five parameters, and functions can validate everything in one place. Nested `date` and `bloodpressure` types are reusable and can be compared field by field. Extending the record (e.g. allergies, doctor) means adding one member; no function signature or parallel array changes. The register structure holds `count`, so no global variable is required and several registers can coexist.

## Alternative: parallel arrays vs array of structures

Parallel arrays are still preferred when an operation touches **one field of many records** (e.g. average age over 1 million records): the age array is contiguous so cache use and SIMD vectorisation are better; and also when padding must be avoided or fields are processed by different threads/GPU kernels.

| | Array of structures | Parallel arrays (structure of arrays) |
|---|---|---|
| Moves per swap | 1 structure copy = 88 bytes (3 moves of 88) | 5 separate swaps = 85 bytes but 5x3 moves, easy to miss one |
| Sort time (comparison phase) | O(n^2) selection; O(n log n) with `qsort` | same Big-O |
| Space | n x 88 B (includes 3 B padding per record) | n x 85 B (no padding between arrays) |
| Measured (n = 8000, selection sort by age) | 0.082 s | 0.045 s (age scanned contiguously) |

Both are O(n^2) swaps-and-compares for selection sort (n-1 swaps, n(n-1)/2 comparisons); space is O(n). SoA is faster to scan but riskier to maintain.

# Task 2 – Courier Parcel Tracking (8 marks)

## 2.1 Entities

```
Parcel   : id, weight, charge, service(enum NORMAL/EXPRESS)
  +-- Date  booked : day, month, year
  +-- Party sender : name, city, pin     (Party reused)
  +-- Party receiver : name, city, pin
```
Nested structures: `Date` and two `Party` inside `Parcel`.

## 2.2 Structure definitions

```c
typedef struct { int day, month, year; } Date;
typedef struct { char name[30]; char city[20]; int pin; } Party;
typedef enum { NORMAL, EXPRESS } Service;
typedef struct { int id; float weight; Date booked; Party sender, receiver; Service service; float charge; } Parcel;
Parcel today[100];   /* the day's bookings */
```

## 2.3 Pseudocode

```
ALGORITHM SortParcelsByWeight
INPUT : array P of n parcels
OUTPUT: P in ascending order of weight
BEGIN
  FOR i <- 0 TO n-2 DO
     min <- i
     FOR j <- i+1 TO n-1 DO
        IF P[j].weight < P[min].weight THEN min <- j
     END FOR
     IF min != i THEN swap whole structures P[i], P[min]
  END FOR
END

ALGORITHM ListParcelsForCity
INPUT : array P, n, city
OUTPUT: parcels whose receiver city equals city
BEGIN
  FOR i <- 0 TO n-1 DO
     IF P[i].receiver.city = city THEN DISPLAY P[i].id, P[i].weight, P[i].receiver.name
  END FOR
END
```

## 2.4 Access paths
(i) `today[2].receiver.pin` (ii) `Parcel *p = today; (p + 2)->receiver.pin` (or `p[2].receiver.pin`). Both gave 600001 in the test program.

## Complete the Code

```c

void print_parcel(const struct parcel *p)
{
    printf("%d %.1f kg %s-%d %02d/%02d/%04d\n", p->id, p->weight, p->to.city,
           p->to.pincode, p->booked.day, p->booked.month, p->booked.year);
}

/* Selection sort of the parcels in ascending order of weight. */
void sort_by_weight(struct parcel p[], int n)
{
    for (int i = 0; i < n - 1; i++) {
        int min = i;
        for (int j = i + 1; j < n; j++)
            if (p[j].weight < p[min].weight) min = j;
        if (min != i) {
            struct parcel tmp = p[i];
            p[i] = p[min];
            p[min] = tmp;
        }
    }
}
```

Output (matches expected; Screenshot: Task02/output_parcel.png):

```
502 1.2 kg Chennai-600077 12/10/2026
503 2.8 kg Coimbatore-641001 13/10/2026
501 4.5 kg Madurai-625001 12/10/2026
```

`sizeof(struct parcel) = 44` (4 + 4 + address 24 + date 12), so each swap copies 3 x 44 = **132 bytes** (tmp = p[i]; p[i] = p[min]; p[min] = tmp). 

Extended model (`parcel_full.c`) output:

```
Sorted by weight:
  504 0.5
  502 1.2
  503 2.8
  501 4.5
Parcels for Chennai:
  parcel 503  2.8 kg  to Devi, Chennai-600001
  parcel 501  4.5 kg  to Anu, Chennai-600077
3rd parcel receiver PIN: (i) today[2].receiver.pin = 600001  (ii) (p+2)->receiver.pin = 600001
sizeof(Date)=12 sizeof(Party)=56 sizeof(Parcel)=140 sizeof(FlatParcel)=140
Complete-the-code struct parcel: sizeof = 44 (bytes copied per swap, 3 copies = 132)
```

## Alternative: flat structure
A flat `FlatParcel` has `bd, bm, by, scity, spin, rcity, rpin ...` directly inside. Access becomes `p->rpin` (shorter) instead of `p->receiver.pin`, but the address type cannot be reused (sender and receiver each need copies of the same fields written twice), a date cannot be passed to a function as one unit, and adding a field to an address means editing every copy. Space: both measure **140 bytes** (no extra padding), so nesting costs no memory; it only improves reuse. Time: identical, O(1) access, sort O(n^2). Nested design preferred.

# Task 3 – Unions and Memory Layout (9 marks)

## 3.1 Structure or union?
Design A (structure of all three): `struct { float celsius; int percent; char status[8]; }` - 16 bytes, but only one field is ever valid. Design B (tagged union): `struct { enum kind type; union {float; int; char[8]} value; }` - 12 bytes and the tag says which member is valid. For thousands of packets on a low-memory station B saves 25% (10 000 packets: 160 000 B vs 120 000 B) and prevents reading the wrong field.

## 3.2 Predicted vs measured size (64-bit GCC)
Union = max(4, 4, 8) = 8 bytes, alignment = max(4,4,1) = 4. Struct A = 4 + 4 + 8 = 16. Struct B = tag 4 (offset 0) + union 8 (offset 4, already 4-aligned) = 12, no padding. (Adding a `double` would force 8-byte alignment and 16-24 bytes.) The measured output below confirms predictions.

```
A) struct with all three : 16 bytes
B) tagged union          : 12 bytes (tag at 0, union at 4)
C) tag + void* (+ malloc'd payload 4/4/8 bytes): 16 bytes header
D) three separate arrays : 4 / 4 / 8 bytes per packet, no tag, but no arrival order
per 10000 packets: A=160000  B=120000  C>=200000 (+malloc overhead, typically 16-32 B each)
Misuse: stored celsius=31.5, read percent -> 1107034112 (hex 0x41FC0000)
After storing percent=68, celsius reads -> 9.52883e-44
```

**Why 12 and not 4 + 4 + 4 + 8 = 20:** the three union members share the same storage, so the union costs only the largest member (8), not 4 + 4 + 8. 4 (tag) + 8 (union) = 12.

## 3.3 Implementation / Complete the Code

```c
{
    switch (r->type) {
    case TEMPERATURE: printf("Temp %.1f C\n", r->value.celsius); break;
    case HUMIDITY:    printf("Humidity %d %%\n", r->value.percent); break;
    case STATUS:      printf("Status %s\n", r->value.status); break;
    }
}

int main(void)
```

```
sizeof union = 8, sizeof struct reading = 12
Temp 31.5 C
Humidity 68 %
Status OK
```

## 3.4 Misuse experiment
Stored `celsius = 31.5f` and read `percent`: result **1107034112 (0x41FC0000)**. 31.5 in IEEE-754 single precision is 0x41FC0000; the union holds the same 4 bytes, and the int interpretation of those bits is 1107034112. Reverse: storing `percent = 68` and reading `celsius` gives 9.5e-44 (a denormal). The compiler gives no warning - the tag is the programmer's responsibility.

## Alternative concepts
| Design | Read time | Space per packet |
|---|---|---|
| Tagged union (chosen) | O(1) array index + switch | 12 B |
| Three separate arrays | O(1) per array but a packet's arrival order is lost unless an index/tag array is added (extra 1-4 B) | 4, 4 or 8 B (no waste) + index |
| `malloc` + `void *` | O(1) + pointer dereference (cache miss), cast needed | 16 B header (tag + pointer) + 4/4/8 B payload + 16-32 B allocator overhead = 40+ B; also O(n) frees |

Chosen: tagged union - smallest per-packet fixed-size record, contiguous, type-safe via tag, no allocation.

# Task 4 – Structure Forensics (7 marks)

## 4.1 Defects
1. **No change in balances:** `addInterest(struct account a, ...)` receives a **copy** of `list[i]`; the interest is added to the copy, which is destroyed on return.
2. **Compile error:** `p` is a `struct account *`; members must be accessed with `p->number`, not `p.number`.
(3. Also `#include <stdio.h>` is missing - modern GCC reports an implicit/incompatible `printf` declaration; added in the lab copy.)

Compiler output for the original:
```
Task04/buggy.c: In function 'main':
Task04/buggy.c:11:24: error: 'p' is a pointer; did you mean to use '->'?
   11 |     printf("%d %f\n", p.number, p.balance);
      |                        ^
      |                        ->
Task04/buggy.c:11:34: error: 'p' is a pointer; did you mean to use '->'?
   11 |     printf("%d %f\n", p.number, p.balance);
      |                                  ^
      |                                  ->
Task04/buggy.c:9:21: warning: variable 'p' set but not used [-Wunused-but-set-variable]
    9 |     struct account *p = list;
      |                     ^
```

## 4.2 Evidence
```
main: &list[0] = 0x7ffe5ae21260
  inside addInterest : &a = 0x7ffe5ae21238, a.balance before = 5000.00
  inside addInterest : a.balance after  = 5200.00 (copy only)
main: list[0].balance after call = 5000.00
main: &list[1] = 0x7ffe5ae21268
  inside addInterest : &a = 0x7ffe5ae21238, a.balance before = 12000.00
  inside addInterest : a.balance after  = 12480.00 (copy only)
main: list[1].balance after call = 12000.00
main: &list[2] = 0x7ffe5ae21270
  inside addInterest : &a = 0x7ffe5ae21238, a.balance before = 800.00
  inside addInterest : a.balance after  = 832.00 (copy only)
main: list[2].balance after call = 800.00
101 5000.000000
```
`&list[i]` is in main's frame (...dc0, dc8, dd0) whereas `&a` inside the function is a different address (...d98) - a separate copy, and `list[i].balance` is unchanged after the call.

## 4.3 Repaired program
```c
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
```
main: &list[0] = 0x7fff089cfb00
  inside addInterest : a = 0x7fff089cfb00
main: &list[1] = 0x7fff089cfb08
  inside addInterest : a = 0x7fff089cfb08
main: &list[2] = 0x7fff089cfb10
  inside addInterest : a = 0x7fff089cfb10
101 5200.00
102 12480.00
103 832.00
```
Addresses are now identical in `main` and the function - the same object is modified. 5000 -> 5200, 12000 -> 12480, 800 -> 832.

## 4.4 Passing guideline
- **By value:** small structures (a few bytes) that the function must not change, or when a private copy is wanted.
- **By pointer:** when the function must modify the caller's structure, or when the structure is large.
- **By const pointer:** large structures that are only read (print, search) - no copy and the compiler rejects accidental writes.

## Alternative: by value and return
```c
struct account addInterest(struct account a, float rate) { a.balance += a.balance * rate / 100; return a; }
list[i] = addInterest(list[i], 4);
```
Works (balances 5200, 12480, 832) but copies sizeof(struct account) = 8 bytes in, 8 out (16 B per call) against one 8-byte pointer. Time O(sizeof(struct)) vs O(1); stack space: ~2 x sizeof(struct) vs sizeof(pointer). Measured (-O0):

```
101 5200.00
102 12480.00
103 832.00
sizeof(struct account) = 8, copied twice per by-value call = 16 bytes; pointer = 8 bytes
sizeof(struct big) = 248, copied twice = 496 bytes
account (8 B): by value 0.717 s, by pointer 0.383 s (50000000 calls)
big (248 B)  : by value 0.249 s, by pointer 0.077 s (10000000 calls)
```
For 8 bytes the difference is small; for a 248-byte record it is about 3x. Preferred: pointer (non-const to modify, const to read).

# Task 5 – Storage Classes (7 marks)

## 5.1 Four implementations

| Version | Source | Output |
|---|---|---|
| (a) auto | `auto_token()` in `token.c` | `auto : 1001 1001 1001` (restarts each call) |
| (b) static | `next_token()` in `token.c` | `static: 1001 1002 1003` |
| (c) extern | `extern_counter.c` defines `token_counter`; `extern_main.c` declares `extern int token_counter;` | `extern: 1001 1002` |
| (d) register | `register_loop.c` loop var `i` | `register loop tokens: 1001 ... 1005` |

Complete the code (`token.c`):
```c
#include <stdio.h>
/* Issues the next mess token number, starting from 1001. */
int next_token(void)
{
    static int token = 1000;   /* initialised once; keeps its value between calls */
    return ++token;
}
```
static: 1001 1002 1003
auto : 1001 1001 1001

extern: 1001 1002
after other file bumps counter by 10: 1013

register loop tokens: 1001 1002 1003 1004 1005

```

Evaluation order experiment: `printf("%d %d %d\n", next_token(), next_token(), next_token());` printed `1003 1002 1001
`on this GCC (right-to-left). The C standard does not specify the order in which function arguments are evaluated, so another compiler/optimisation level may print 1001 1002 1003. Fix: store each call in its own variable (separate statements, sequenced).

## 5.2 Two counters (veg / non-veg)

```
auto     veg: 1001 1001 | nonveg: 2001 2001  (both restart: WRONG)
static   veg: 1001 1002 | nonveg: 2001 2002
extern   veg: 1001 1002 | nonveg: 2001 2002
pointer  veg: 1001 1002 | nonveg: 2001 2002
pointer  two more independent counters: 1001 1002 1001
```

| Version | Change needed | Lines changed |
|---|---|---|
| auto | Cannot work - nothing persists | not possible |
| static | One more function (or a `kind` parameter) with its own static | +3 lines |
| extern | One more global (defined once) + `extern` declaration in every user file + function | +1 definition, +1 declaration per file, +3 lines |
| register | No change - register is for the loop variable only | 0 |
| (alt.) pointer | Caller declares another `int`; function unchanged | 1 line in caller |

## 5.3 Comparison

| | auto | static (local) | extern (global) | register |
|---|---|---|---|---|
| Scope | block | block | all files (declared via extern) | block |
| Lifetime | block execution | whole program | whole program | block execution |
| Initial value | garbage | 0 (init once) | 0 | garbage |
| Accidental modification | low | low (only the function) | HIGH (any file, e.g. +10 in demo) | low |
| Address `&` | yes | yes | yes | **no** |

Taking `&` of a register variable is not allowed because it may live in a CPU register, which has no memory address. Compiler output:
```
Task05/register_addr_error.c: In function 'main':
Task05/register_addr_error.c:1:1: error: address of register variable 'i' requested
    1 | int main(void) { register int i = 0; int *p = &i; return *p; }
      | ^~~
```
(Modern compilers may ignore `register` for optimisation, but the C rule remains.)

Static storage demo:
```
static s = 1, global g = 0, file_static = 0, &s = 0x563cc85b701c
static s = 2, global g = 0, file_static = 0, &s = 0x563cc85b701c
static s = 3, global g = 0, file_static = 0, &s = 0x563cc85b701c
```

## Alternative: pointer counter
`int next_token(int *counter){ return ++*counter; }` with `int veg=1000, nonveg=2000;` in the caller. Time O(1) in all cases. Space: one int per counter, in the caller's frame (lifetime controlled by the caller, not the program), and any number of independent counters is trivial (see `two_counters.c`). Costs: the caller must pass the right pointer; counter is not shared automatically between files. Preferred for scalability and testability; static is best for a single hidden counter.

# Task 6 – College Canteen Design Challenge (13 marks)

## Stage 1 – Interpret the problem
Entities: **Item** (id, name, kind, price, stock, units sold, revenue) in three kinds: **Packaged** (expiry date), **Cooked** (prepared time), **Combo** (up to 3 other items); **Date**; **Canteen** (collection of items); **Sale/Bill**.

Testable requirements:
R1 A sale of qty <= stock reduces stock by qty and adds price x qty to revenue.
R2 A sale of qty > stock is refused and stock/revenue do not change.
R3 Qty <= 0 or unknown id is refused.
R4 An expired packaged item cannot be sold.
R5 A combo sale needs every component available; stock of all components decreases, otherwise nothing changes.
R6 Items expiring within 2 days (and already expired) are reported.
R7 The summary lists all items in descending revenue with a total.

## Stage 2 – Data model
- **Tagged union**: `enum item_kind` tag + `union { struct date expiry; struct time_hm prepared; struct {int count; int item_id[3];} combo; }` - the three kinds hold different data, only one at a time.
- **Nested structures**: `date` inside the packaged union member; `time_hm` for cooked.
- **Array of structures**: `struct canteen { struct item items[50]; int count; }`.

## Stage 3 – Critical decisions
- **Safe stock update:** validate everything first (item exists, qty > 0, not expired, enough stock for the item or all components) and only then subtract (commit). No negative stock; combos are all-or-nothing.
- **Expiry check:** `days_between(from, to)` converts both `date` structures to a day number so month/year boundaries work; warn when `days <= 2`, flag `< 0` as expired.
- **Static / global:** `static int bill_no` inside `record_sale` (persistent bill counter, private); `static const struct date TODAY` (read-only business date); the large `canteen` object is `static` in `main` (zero-initialised, off the stack).

## Stage 4 – Structures, prototypes, pseudocode
```c
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
```
Prototypes:
```c
int  add_item(struct canteen *c, const struct item *it);
struct item *find_item(struct canteen *c, int id);
int  days_between(const struct date *from, const struct date *to);
int  record_sale(struct canteen *c, int id, int qty, const struct date *today);
void expiry_warning(const struct canteen *c, const struct date *today);
void print_summary(const struct canteen *c);
```
ALGORITHM RecordSale(c, id, qty, today)
BEGIN
  it <- FindItem(c, id);   IF it = NULL THEN RETURN ERR_NO_ITEM
  IF qty <= 0 THEN RETURN ERR_BAD_QTY
  IF it->kind = PACKAGED THEN
      IF DaysBetween(today, it->expiry) < 0 THEN RETURN ERR_EXPIRED
      IF it->stock < qty THEN RETURN ERR_STOCK
      it->stock <- it->stock - qty
  ELSE IF it->kind = COOKED THEN
      IF it->stock < qty THEN RETURN ERR_STOCK
      it->stock <- it->stock - qty
  ELSE                                  // COMBO
      FOR each component k DO
          need <- qty x (number of times k appears in the combo)
          IF component missing OR stock < need OR expired THEN RETURN error
      FOR each component k DO stock <- stock - qty
  it->units_sold += qty;  it->revenue += it->price x qty
  bill_no <- bill_no + 1; PRINT bill line;  RETURN OK
END

ALGORITHM ExpiryWarning(c, today)
BEGIN
  FOR i <- 0 TO c->count-1 DO
     IF c->items[i].kind = PACKAGED THEN
        d <- DaysBetween(today, items[i].expiry)
        IF d < 0 THEN PRINT "EXPIRED"
        ELSE IF d <= 2 THEN PRINT "EXPIRING in d days"
END
```
Summary sort: an array of pointers to items is sorted (selection sort by revenue, descending) so no 64-byte structure is moved.

## Stage 5 – Implementation and demonstration
Full source: `Task06/canteen.c` (see repository). Output (Screenshot: Task06/output_canteen.png):
```
Date: 19/11/2026
sizeof(struct item) = 64, sizeof(struct canteen) = 3204

[Test 1] Normal sales
  Bill #001: Samosa         x5   @  15.00 =   75.00
  Bill #002: Biscuits       x4   @  10.00 =   40.00
  Bill #003: Chips          x2   @  20.00 =   40.00
  Bill #004: Meal Combo     x3   @  40.00 =  120.00

[Test 2] Sale exceeding stock (Juice stock now 9)
  Sale of item 102 x20: INSUFFICIENT STOCK - sale refused
  Juice stock unchanged: 9

[Test 3] Expired item, invalid quantity, unknown item
  Sale of item 106 x1: item EXPIRED - sale refused
  Sale of item 103 x0: invalid quantity
  Sale of item 999 x1: item not found

[Test 4] Combo with an unavailable component
  Sale of item 105 x3: INSUFFICIENT STOCK - sale refused
  Samosa stock unchanged (all-or-nothing): 2

[Test 5] Expiry warning (within 2 days)
  EXPIRING : Biscuits     expires 20/11/2026 (in 1 day(s)), stock 23
  EXPIRING : Juice        expires 21/11/2026 (in 2 day(s)), stock 9
  EXPIRED  : Milk         expired 18/11/2026 (1 day(s) ago), stock 5

[Test 6] Day's bill summary sorted by revenue
  ID   Item             Sold   Revenue
  105  Meal Combo          3    120.00
  104  Samosa              5     75.00
  103  Chips               2     40.00
  101  Biscuits            4     40.00
  102  Juice               0      0.00
  106  Milk                0      0.00
  TOTAL                         275.00
```
Tests covered: normal sales (R1), insufficient stock (R2), expired/invalid/unknown (R3, R4), combo all-or-nothing (R5), expiry warning (R6), sorted summary with total (R7). `sizeof(struct item) = 64` (name 24 + ints/floats + 16-byte union).

## Alternative: linked list
```c
struct node { struct item data; struct node *next; };
```
| Operation | Array | Linked list |
|---|---|---|
| Add | O(1) at end (if capacity left), fixed MAX_ITEMS | O(1) at head (malloc) |
| Remove | O(n) find + O(n) shifting | O(n) find + O(1) unlink |
| Search by id | O(n) (O(log n) if sorted) | O(n) |
| Extra space | none | 8 B `next` pointer per item + 16-32 B malloc overhead; must free; poor cache locality |

For ~50 items with few removals the array is simpler, faster and safer; a linked list helps only for frequent additions/removals of an unbounded menu. **Final design: array of structures with a tagged union**, pointer array for sorting.

# Conclusion
Structures (with nesting) group related data; unions with a tag save memory safely; passing by pointer is required for modification; and the correct storage class (static for hidden persistent state, extern sparingly, register only for loop counters) keeps programs correct and maintainable. All programs compiled warning-free with `gcc -Wall -Wextra` (the two intentionally failing demos excepted).
