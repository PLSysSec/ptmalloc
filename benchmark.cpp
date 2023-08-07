#include <iostream>
#include <stdint.h>
#include <string>
#include <assert.h>
#include <string.h>
#include <malloc.h>
#include "../liquid-ffi/tests/ffi_tests/bindings.h"



// timestamps!
#if defined(__clang__) && __has_builtin(__builtin_readcyclecounter)
#define rdtsc __builtin_readcyclecounter
#else
u_int64_t rdtsc(void) {
  u_int64_t out = 0;
# if (defined(__i386__) || defined(__x86_64__))
    __asm__ __volatile__ ("rdtsc" : "=A"(out));
# endif
  return out;
}
#endif


int main(int argc, char *argv[])
{
    //lffi_init("./allocator_hooks.so");
  
    // char s[6] = "hello";
    // auto stack_buf = (uint8_t const *)s;
    // auto n = 6;
    
    // auto num_iterations = 1000;
    // auto heap_buf = (uint8_t const *) malloc(n);
    // strncpy((char*)heap_buf, s, 6);


    auto num_iterations = 1000;
    void* allocs[num_iterations];
    int32_t* boxes[num_iterations];


    int64_t cpp_malloc_times[num_iterations];
    int64_t cpp_free_times[num_iterations];
    int64_t rust_malloc_times[num_iterations];
    int64_t rust_free_times[num_iterations];

    int64_t stack_check_times[num_iterations];
    int64_t heap_check_explicit_times[num_iterations];
    int64_t heap_check_implicit_times[num_iterations];


    auto start = rdtsc();
    auto end = rdtsc();
    auto rdtsc_overhead = end - start;

    auto ffi_start = rdtsc();
    auto _garbage = new_test(); // empty call
    auto ffi_end = rdtsc();
    auto ffi_overhead = ffi_end - ffi_start - rdtsc_overhead;

    // TODO: average over a 1000 runs?

    // 1) c++ malloc
    for(int idx = 0; idx < num_iterations; idx++){
      u_int64_t start = rdtsc();
      void* alloc = malloc(24); // size chosen to be same as boxes
      u_int64_t end = rdtsc();
      assert(end > start);
      // printf("%d %p\n", idx, alloc);
      assert(alloc != NULL);
      allocs[idx] = alloc;
      cpp_malloc_times[idx] = (int64_t)(end - start) - rdtsc_overhead;
      
    }

    // // 2) c++ free
    for(int idx = 0; idx < num_iterations; idx++){
      void* alloc = allocs[idx];
      // printf("%d %p\n", idx, alloc);
      u_int64_t start = rdtsc();
      free(alloc);
      u_int64_t end = rdtsc();
      assert(end > start);
      // cpp_free_times[idx] should be free() time when there are idx active allocs 
      cpp_free_times[num_iterations - idx - 1] = (int64_t)(end - start) - rdtsc_overhead; 
    }

    // // 3) rust malloc
    // for(int idx = 0; idx < num_iterations; idx++){
    //   u_int64_t start = rdtsc();
    //   auto box = make_box();
    //   u_int64_t end = rdtsc();
    //   boxes[idx] = box;
    //   rust_malloc_times[idx] = (int64_t)(end - start) - rdtsc_overhead - ffi_overhead;
    // }

    // // 4) rust free
    // for(int idx = 0; idx < num_iterations; idx++){
    //   auto box = boxes[idx];
    //   u_int64_t start = rdtsc();
    //   free_box(box);
    //   u_int64_t end = rdtsc();
    //   boxes[idx] = box;
    //   // rust_free_times[idx] should be free() time when there are idx active allocs 
    //   rust_free_times[num_iterations - idx - 1] = (int64_t)(end - start) - rdtsc_overhead - ffi_overhead; 
    // }



    // // 5) stack check (no fancy heap check)
    // int32_t val = 3;
    // for(int idx = 0; idx < num_iterations; idx++){
    //   u_int64_t start = rdtsc();
    //   check_buffer(&val, 4);
    //   u_int64_t end = rdtsc();
    //   // rust_free_times[idx] should be free() time when there are idx active allocs 
    //   stack_check_times[idx] = (int64_t)(end - start) - rdtsc_overhead - ffi_overhead; 
    //   allocs[idx] = malloc(4);
    // }
    // // free the associated allocs
    // for(int idx = 0; idx < num_iterations; idx++){
    //   free(allocs[idx]);
    // }

    // // 6) heap check (explicit)
    // int32_t* val_ptr = (int32_t*)malloc(4);
    // *val_ptr = 3;
    // for(int idx = 0; idx < num_iterations; idx++){
    //   u_int64_t start = rdtsc();
    //   check_buffer(val_ptr, 4);
    //   u_int64_t end = rdtsc();
    //   // rust_free_times[idx] should be free() time when there are idx active allocs 
    //   heap_check_explicit_times[idx] = (int64_t)(end - start) - rdtsc_overhead - ffi_overhead; 
    //   allocs[idx] = malloc(4);
    // }
    // // free the associated allocs
    // for(int idx = 0; idx < num_iterations; idx++){
    //   free(allocs[idx]);
    // }


    // // 7) heap check (implicit)
    // for(int idx = 0; idx < num_iterations; idx++){
    //   u_int64_t start = rdtsc();
    //   check_ptr(val_ptr);
    //   u_int64_t end = rdtsc();
    //   // rust_free_times[idx] should be free() time when there are idx active allocs 
    //   heap_check_implicit_times[idx] = (int64_t)(end - start) - rdtsc_overhead - ffi_overhead; 
    //   allocs[idx] = malloc(4);
    // }
    // // free the associated allocs
    // for(int idx = 0; idx < num_iterations; idx++){
    //   free(allocs[idx]);
    // }

    auto fp = fopen(argv[1], "w+");

    // dump 1) c++ malloc
    for(int idx = 0; idx < num_iterations; idx++){
      fprintf(fp, "%ld ", cpp_malloc_times[idx]);
    }
    fprintf(fp, "\n");

    // dump 2) c++ free
    for(int idx = 0; idx < num_iterations; idx++){
      fprintf(fp, "%ld ", cpp_free_times[idx]);
    }
    fprintf(fp, "\n");

    // // dump 3) rust malloc
    // for(int idx = 0; idx < num_iterations; idx++){
    //   fprintf(fp, "%ld ", rust_malloc_times[idx]);
    // }
    // fprintf(fp, "\n");

    // // dump 4) rust free
    // for(int idx = 0; idx < num_iterations; idx++){
    //   fprintf(fp, "%ld ", rust_free_times[idx]);
    // }
    // fprintf(fp, "\n");


    // // dump 5) stack check
    // for(int idx = 0; idx < num_iterations; idx++){
    //   fprintf(fp, "%ld ", stack_check_times[idx]);
    // }
    // fprintf(fp, "\n");


    // // dump 6) heap check (explicit)
    // for(int idx = 0; idx < num_iterations; idx++){
    //   fprintf(fp, "%ld ", heap_check_explicit_times[idx]);
    // }
    // fprintf(fp, "\n");


    // // dump 7) heap check (implicit)
    // for(int idx = 0; idx < num_iterations; idx++){
    //   fprintf(fp, "%ld ", heap_check_implicit_times[idx]);
    // }
    // fprintf(fp, "\n");

    fclose(fp);

    // // buffer check (stack)
    // u_int64_t valid_upto_stack_start = rdtsc();
    // for(int idx = 0; idx < num_iterations; idx++){
    //     encoding_utf8_valid_up_to(stack_buf, n);
    // }
    // u_int64_t valid_upto_stack_end = rdtsc();
    // printf("Average time for encoding_utf8_valid_upto (stack) = %lu cycles\n", (valid_upto_stack_end - valid_upto_stack_start) / num_iterations);

    // // buffer check (heap)
    // u_int64_t valid_upto_heap_start = rdtsc();
    // for(int idx = 0; idx < num_iterations; idx++){
    //     encoding_utf8_valid_up_to(heap_buf, n);
    // }
    // u_int64_t valid_upto_heap_end = rdtsc();
    // printf("Average time for encoding_utf8_valid_upto (heap) = %lu cycles\n", (valid_upto_heap_end - valid_upto_heap_start) / num_iterations);

    // // inferred check (arg in globals area)
    // u_int64_t encoding_start = rdtsc();
    // for(int idx = 0; idx < num_iterations; idx++){
    //     encoding_new_encoder(UTF_8_ENCODING);
    // }
    // u_int64_t encoding_end = rdtsc();
    // printf("Average time for encoding_new_encoder = %lu cycles\n", (encoding_end - encoding_start) / num_iterations);



    // // rust instrumentation
    // u_int64_t encoding_start = rdtsc();
    // for(int idx = 0; idx < num_iterations; idx++){
    //     encoding_new_encoder(UTF_8_ENCODING);
    // }
    // u_int64_t encoding_end = rdtsc();
    // printf("Average time for encoding_new_encoder = %lu cycles\n", (encoding_end - encoding_start) / num_iterations);


    //lffi_cleanup();

    return 0;
}
