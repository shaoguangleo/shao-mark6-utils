// scatspeed - program to test multiple disk scattered write performance
//
//                                       2012.10.15  rjc
//  just do write speed                  2013.1.11   rjc
//  write to scattered disks             2013.4.9    rjc
//  handle subpool version of scat_write 2014.6.3    rjc

#include "../d-plane/dplane.h"

#define LBUFF 10240000
#define MAXBLK 50000

u_char buff[LBUFF];
struct global_tag glob;

int ifile[MAXBLK];

int main(int argc, char **argv)
{
    int i,
        rc,
        nblocks;

    double deltat,
        mb_total;

    struct timeval tv,
        tv0;
    struct timezone tz,
        tz0;

    pthread_t thrs[NFILES];

    int scat_write(u_char *, int, int);
    void *writer(void *);

    if (argc < 3 || argc > 34)
    {
        printf("Usage: scatspeed <file size (GB)> <file1> <file2> ... <file n>\n");
        printf("       number of files is variable (1..32)\n");
        return 0;
    }

    nblocks = 1000000000 / LBUFF * atoi(argv[1]);
    // ensure that ifile array within bounds
    if (nblocks > MAXBLK)
        nblocks = MAXBLK;

    glob.nfils = argc - 2;

    for (i = 0; i < glob.nfils; i++) // open all files
    {
        glob.sfile[i].phyle = fopen(argv[i + 2], "w");
        if (glob.sfile[i].phyle == NULL)
        {
            perror("scatspeed");
            printf("quitting due to open error for disk %d file %s\n", i, argv[i + 2]);
            exit(1);
        }
        glob.sfile[i].file_open = TRUE;
        glob.stream[0].fpool[i] = TRUE;
        // disable stream buffering
        setbuf(glob.sfile[i].phyle, NULL);
        glob.sfile[i].nbytes = 0;
        // clear write request and start writer thread
        glob.sfile[i].wr_pending = FALSE;
        pthread_create(&thrs[i], NULL, writer, (void *)(long)i);
    }

    // preload buffer
    for (i = 0; i < LBUFF; i++)
        buff[i] = i % 256;

    gettimeofday(&tv0, &tz0);

    // disk writing loop
    for (i = 0; i < nblocks; i++)
    {
        rc = scat_write(buff, LBUFF, 0);
        if (rc >= 0)
            ifile[i] = rc;
        else // no disks ready, sleep for a while
        {
            usleep(WSLEEP_US);
            i--;
        }
    }

    // flush cache before re-reading time
    for (i = 0; i < glob.nfils; i++) // by closing all files
        fclose(glob.sfile[i].phyle);

    gettimeofday(&tv, &tz);
    // elapsed time in seconds
    deltat = tv.tv_sec - tv0.tv_sec + 1e-6 * (tv.tv_usec - tv0.tv_usec);

    mb_total = 1e-6 * LBUFF * nblocks;
    printf("%8.1lf MB in %5.1lf sec --> %5.1lf MB/s\n",
           mb_total, deltat, mb_total / deltat);

    for (i = 0; i < glob.nfils; i++) // print speed results
        printf("f%2d: %5d MB %3d MB/s\n", i,
               (int)((glob.sfile[i].nbytes + 500000) / 1000000),
               (int)((glob.sfile[i].nbytes + 500000) / 1e6 / deltat));
    return 0;
}
