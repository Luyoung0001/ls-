#include <stdio.h>
int n = 10;

int main(){
    for(int i = 0; i < n; i++){
        n = 5;
        printf("%d ",i);
    }
    return 0;
}