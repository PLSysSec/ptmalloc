#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <malloc.h>


int main(int argc, char *argv[])
{
    void* alloc = malloc(32);
    printf("# of mallocs: %d\n", lffi_malloc_count(alloc));
}
