/*用VS code调程序文件路径不能有中文*/

#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include "pid.h"
#include "pid.c"


#define random(x) (rand()%x)

int main()
{
    int i;
    char a[2][10] = {"start!","end!"};
    float randrmValue,temp;
    srand((int)time(0));
    printf("%s\n",a[0]);

    pid.SetSpeed = 202.02;
    for(i = 0;i < 200;i++)
    {
        temp = (pid.pid_realize)();
        pid.ActualSpeed = temp;
        printf("%.2f\t",temp);
    }


    printf("%s\n",a[1]);
    return 0;
}