/* Task 3.1/3.2/3.4 + alternative designs */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
enum kind { TEMPERATURE, HUMIDITY, STATUS };
struct packet_all { float celsius; int percent; char status[8]; };            /* design A: struct of all */
struct packet_tag { enum kind type; union { float celsius; int percent; char status[8]; } value; }; /* design B */
struct packet_ptr { enum kind type; void *data; };                             /* alternative: void* */

int main(void)
{
    printf("A) struct with all three : %zu bytes\n", sizeof(struct packet_all));
    printf("B) tagged union          : %zu bytes (tag at %zu, union at %zu)\n", sizeof(struct packet_tag),
           offsetof(struct packet_tag, type), offsetof(struct packet_tag, value));
    printf("C) tag + void* (+ malloc'd payload 4/4/8 bytes): %zu bytes header\n", sizeof(struct packet_ptr));
    printf("D) three separate arrays : 4 / 4 / 8 bytes per packet, no tag, but no arrival order\n");
    printf("per 10000 packets: A=%zu  B=%zu  C>=%zu (+malloc overhead, typically 16-32 B each)\n",
           10000 * sizeof(struct packet_all), 10000 * sizeof(struct packet_tag), 10000 * (sizeof(struct packet_ptr) + 4));

    /* misuse experiment */
    struct packet_tag t; t.type = TEMPERATURE; t.value.celsius = 31.5f;
    printf("Misuse: stored celsius=31.5, read percent -> %d (hex 0x%X)\n", t.value.percent, (unsigned)t.value.percent);
    t.value.percent = 68;
    printf("After storing percent=68, celsius reads -> %g\n", t.value.celsius);
    return 0;
}
