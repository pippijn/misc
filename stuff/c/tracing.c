#include <stdio.h>

// compile with instrumentation
//  gcc -Wl,-Map=hello.map -g -finstrument-functions hello.c -o hello

// run and show a live trace
//  ./hello | xargs addr2line -f -e hello

void __cyg_profile_func_enter( void *, void * ) 
	__attribute__ ((no_instrument_function));

void __cyg_profile_func_exit( void *, void * )
	__attribute__ ((no_instrument_function));

void __cyg_profile_func_enter( void *this, void *callsite )
{
    printf("%p\n", (int *)this);
}

void __cyg_profile_func_exit( void *this, void *callsite )
{
    printf("%p\n", (int *)this);
}

void bar(void)
{
    fprintf(stderr, "bar");
}

void foo(void)
{
    bar();
}

int main(int argc, char *argv[])
{
    foo();
    return 0;
}
