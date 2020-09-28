#include <iostream>
#include <malloc.h>
#include <unistd.h>
#include <string.h>

using namespace std;

int main(){
    char a = '*';
    int temp = 43;
    a = temp;
    cout << a << endl;
}