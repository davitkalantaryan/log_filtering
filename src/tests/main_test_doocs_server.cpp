//
//  file:           main_victim_app.cpp
//  created on:     2019 Nov 07
//
#include <stdio.h>
#include <unistd.h>

int main()
{
    int nIteration(0);

    printf("Starting application!\n");
    fflush(stdout);

    while(1){
        printf("Hello world (iteration=%d)!\n",++nIteration);
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!! Seldom Hello world !\n");
        fflush(stdout);
        usleep(500000);
    }

    //return 0;
}
