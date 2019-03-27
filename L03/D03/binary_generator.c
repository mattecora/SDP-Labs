#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char const *argv[])
{
    int i, num, len;
    FILE *fp;

    srand(time(0));

    if (argc < 2)
    {
        fprintf(stderr, "Too few arguments.\n");
        return -1;
    }

    len = atoi(argv[1]);
    fp = fopen(argv[2], "wb");

    if (len == 0 || fp == NULL)
    {
        fprintf(stderr, "An error occurred.\n");
        return -1;
    }

    for (i = 0; i < len; i++)
    {
        num = rand() % len + 1;
        fwrite(&num, sizeof(int), 1, fp);
    }

    fclose(fp);
    return 0;
}
