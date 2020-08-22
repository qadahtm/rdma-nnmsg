#include <iostream>

using namespace std;

int main(){
    volatile int64_t *ptr1, *ptr2;
    ptr1 = (volatile int64_t *) calloc(0, sizeof(ptr1));
    ptr2 = (volatile int64_t *) calloc(0, sizeof(ptr2));

    *ptr1 = 12;
    *ptr2 = 23;

    cout << *ptr1 << *ptr2 << "  " << (uintptr_t) ptr1 << "  " << (uintptr_t) ptr2 << endl;
    cout << &ptr1 << " " << &ptr2 << endl;
    return 0;
}