#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queueADT.h"

void *cpyStr(void *ptr);
void freeStr(void *ptr);

void *
cpyStr(void *ptr)
{
    char *str = (char *)ptr;
    return (void *)strdup(str);
}

void
freeStr(void *ptr)
{
    if (ptr != NULL)
        free(ptr);

    return;
}

int main(void)
{
    char *str1 = "First";
    char *str2 = "Second";
    char *str3 = "Third";
    queueADT queue;

    queue = newQueue(cpyStr, freeStr);

    if ((enqueue(queue, str1))==0)
        printf("Error\n");

    if ((enqueue(queue, str2))==0)
        printf("Error\n");

    if ((enqueue(queue, str3))==0)
        printf("Error\n");

    str3 = dequeue(queue);
    printf("%s\n", str3);
    str1 = dequeue(queue);
    printf("%s\n", str1);
    str2 = dequeue(queue);
    printf("%s\n", str2);

    str2 = dequeue(queue);

    if (str2 == NULL)
        printf("str2 doesn't exist\n");
    else
        printf("%s\n", str2);

    free(str1);
    free(str2);
    free(str3);
    freeQueue(queue);
    return 0;
}

