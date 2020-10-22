// dspeed - program to test disk performance
//
//                        2012.10.15  rjc
//  just do write speed   2013.1.11   rjc

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>

#define LBUFF 10000000
#define NBLOCKS 1000 // default #of blocks

int buff[LBUFF];

int main(int argc, char **argv)
{
    int i,
        nblocks = NBLOCKS;

    double deltat,
        mb_total;

    FILE *fout;

    struct timeval tv,
        tv0;
    struct timezone tz,
        tz0;

    if (argc != 2 && argc != 3)
    {
        printf("Usage: dspeed <filename> [<file size (GB)]\n");
        return 0;
    }

    // preload buffer
    for (i = 0; i < LBUFF; i++)
        buff[i] = i;

    fout = fopen(argv[1], "w");
    if (fout == NULL)
    {
        perror("dspeed");
        printf("quitting due to open error\n");
        exit(1);
    }

    if (argc == 3)
        nblocks = 100 * atoi(argv[2]);

    setbuf(fout, NULL);

    gettimeofday(&tv0, &tz0);

    for (i = 0; i < nblocks; i++)
        fwrite(buff, LBUFF, 1, fout);
    // flush cache before re-reading time
    fclose(fout);

    gettimeofday(&tv, &tz);
    // elapsed time in seconds
    deltat = tv.tv_sec - tv0.tv_sec + 1e-6 * (tv.tv_usec - tv0.tv_usec);

    mb_total = 1e-6 * LBUFF * nblocks;
    printf("%6.1lf MB in %5.1lf sec --> %5.1lf MB/s\n",
           mb_total, deltat, mb_total / deltat);
    return 0;
}
