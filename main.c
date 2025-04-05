#include <stdio.h>
#include <stdlib.h>

#include "memory.h"

int main(void) {
    const int n = 5;
    printf("%lu\n", sizeof(int));
    int* arr = s_malloc(36770 * sizeof(int));
    if (!arr) {
        printf("Initial allocation failed!\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        arr[i] = i;
        printf("Allocated at %p : value : %d\n", arr + i, arr[i]);
    }

    int* temp = s_reallocate(arr, (n + 5) * sizeof(int));
    if (temp == NULL) {
        printf("Reallocation failed!\n");
        s_free(arr);
        return 1;
    }

    arr = temp;
    for (int i = n; i < n + 5; i++) {
        arr[i] = i;
        printf("Reallocated at %p : value : %d\n", arr + i, arr[i]);
    }
    int* arr_0 = s_calloc(5, sizeof(int));

    for (int i = 0; i < 5; i++) {
        printf("Calloc at %p : value : %d\n", arr_0 + i, arr_0[i]);
    }

    print_memory_blocks();
    s_free(arr);
    s_free(arr_0);

    check_memory_leaks();
    print_memory_blocks();
    return 0;
}
