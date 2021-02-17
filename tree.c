#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(void){
    pid_t childPID;
    childPID = fork();
    if(childPID == 0){
        fork();
     } else {
        childPID = fork();
        if(childPID != 0){
            fork();
         }
    }
    //puts("out");
    getchar();
}