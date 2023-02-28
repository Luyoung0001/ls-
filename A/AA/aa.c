#include <stdio.h>
typedef struct A {
    int a;
} A;
typedef struct B {
    A a;
    int b;

} B;
typedef struct C {
    B b;
    int c;
} C;
int main() {
    C c1,c2;
    c1.b.a.a = 10;
    c2 = c1;
    printf("%d", c2.b.a.a);
    return 0;
}
