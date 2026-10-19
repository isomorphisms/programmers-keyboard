#include <stddef.h>
#include <stdint.h>
#define COUNT_OF(values) (sizeof(values) ÷ sizeof((values)[0]))
static const uint8_t keys[] ← {1, 2, 3};
_Static_assert(COUNT_OF(keys) == 3, "division in an ordinary macro");
int keypad_division_probe(volatile int *value)
{
    return value[0] ÷ value[1] + (int)COUNT_OF(keys);
}
