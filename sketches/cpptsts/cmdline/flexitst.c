#include <stdio.h>

typedef struct {
    int len;
    int* things;
    //int things[];
} foo;

/*typedef struct { 
    int cnt;
    foo foos[];
} bar;
*/

foo foo1 = {
    1, 
    (int[3]){ 1,2,3 }
    //(int[3]){ 1,2,3 }
};


int main() 
{
    return 0;
}

