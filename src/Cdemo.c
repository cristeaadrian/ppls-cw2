// Save to Cdemo.c,  compile with gcc -o Cdemo Cdemo.c, run with ./Cdemo
#include<stdlib.h>
#include<stdio.h>

// something like a class, but no methods or access control, just data
typedef struct {
    int first;
    int second;
} mystruct;

// Oops -  parameters are call-by-value, so x and y are integer values
void badswap (int x, int y) {
    int temp;
    temp = x;
    x = y;
    y = temp;
}

// Pass pointers to the variables we want to swap so x and y
// are pointer values (ie addresses of integers)
void goodswap (int *x, int *y) {
    int temp;

    temp = *x;  // *x means "deference" x, ie get the value
    // from the location pointed to by x
    *x = *y;    // copy the value pointed to by y, into the location
    // pointed to by x
    *y = temp;
}

// Everything starts here - like a Java  public static void main(String[] args)
// but giving the argument count explicitly and returning an integer code
// indicating successful completion (or otherwise) to the calling environment
int main(int argc, char *argv[])
{
    int a, b;     // just like Java
    mystruct *s;  // a pointer to a structure, like a Java reference variable

    a = 1; b = 2;

    // C's print statement. The string is printed, with  values filled
    // in from the subsequent parameters to the %d placeholders in the string
    // (%d means print as a decimal)
    printf("a is %d and b is %d\n\n", a, b);

    badswap (a, b);
    printf("a is %d and b is %d\n\n", a, b);

    goodswap (&a, &b);  // pass in the addresses, obtained with &
    printf("a is %d and b is %d\n\n", a, b);

    // create a structure -  like s = new mystruct();
    // malloc allocates the space and returns a generic
    // pointer (ie a "void *"). Its "prototype" is
    //
    //      void *  malloc  (size_t size);
    //
    // The "(mystruct *)" casts this value to a "mystruct pointer"
    // to convince the compiler that it can be safely assigned to s

    s = (mystruct *) malloc (sizeof(mystruct));

    s->first  = 3;   s->second = 4;
    printf("s->first is %d and s->second is %d\n\n", s->first, s->second);

    // see that (*p).f is the same as p->f
    goodswap(&s->first, &s->second);
    printf("(*s).first is %d and (*s).second is %d\n\n", (*s).first, (*s).second);
}
