// gather - a program to reassemble a single file from
// a group of files originally created by dplane
//
// Initial version                   2012.9.14   rjc
// support for v.2 input format      2013.4.22   rjc
// add -o field to prevent accidents 2014.12.19  rjc

#include "dplane.h"
#include <aio.h>

#define MAXFILES 32       // maximum number of input files
#define RB_CELLS 10       // blocks per each input ring buffer
#define MAXBLOCKS 1000000 // maximum number of output blocks
#define MICRO_SLEEP 1000  // 1 ms sleep
#define FHD_SIZE 20

enum gstates
{
    CLOSED,
    IDLING,
    IN_PROGRESS
};

int main(int argc, char **argv)
{
    int i,
        j,
        k,
        rc,
        nfiles,
        nbl,
        fdi[MAXFILES],
        fdo,
        wptr[MAXFILES],
        rptr[MAXFILES],
        oq_max[MAXFILES],
        niblocks[MAXFILES],
        istate[MAXFILES],
        ostate,
        min_oq_max, // minimum of oq_max array
        max_oq_max, // maximum of oq_max array
        old_min_oq_max,
        oblnum,
        did_nothing,
        inputs_open,
        wb_payload,
        nbadblocks,
        ibuf_len[MAXFILES + 1][RB_CELLS],
        fhd_block[MAXFILES][FHD_SIZE / sizeof(int)],
        fhd_length[4] = {0, 20, 20, 0},
        sync_word,
        version,
        pkt_size, /// Package size, 8032 for example
        blk_size, /// Block size, 10MB by default
        pkt_per_blk,
        lbhead; // length of the block header (bytes)

    unsigned int *pint;

    u_char *ibuf[MAXFILES + 1][RB_CELLS], *pbuf;

    long int rb_size, nibytes[MAXFILES], nobytes;

    double elapsed;

    struct output_queue
    {
        short int bufnum;
        short int bufidx;
    };

    struct output_queue *oq_ptr;

    struct aiocb aiocbi[MAXFILES], aiocbo;

    struct timeval tv0, tv;

    struct timezone tz;

    struct wb_header_tag *phead;

    void get_block_size(char *, int *, int *);

    if (argc < 4 || argc > MAXFILES + 3 || strcmp(argv[argc - 2], "-o") != 0)
    {
        printf("Usage:\n");
        printf("gather <ifile1> ... <ifilen> -o <ofile>\n");
        exit(0);
    }

    nfiles = argc - 3; // number of input files [1..MAXFILES]

    gettimeofday(&tv0, &tz); // note the start time

    // get block & header size from header in file
    get_block_size(argv[1], &wb_payload, &version);
    // trap illegal versions
    switch (version)
    {
    case 1:
        lbhead = 4;
        break;

    case 2:
        lbhead = 8;
        break;

    default:
        printf("unsupported version# %d in %s\n", version, argv[1]);
        exit(-1);
    }

    // initialize variables
    for (i = 0; i < MAXFILES; i++)
    {
        wptr[i] = 0;
        rptr[i] = 0;
        istate[i] = CLOSED;
        oq_max[i] = -1;
        nibytes[i] = 0;
        niblocks[i] = 0;
    }
    min_oq_max = -1;
    oblnum = 0;  // point to next block to be written out
    nobytes = 0; // total number of output bytes

    rb_size = (nfiles * RB_CELLS + 1) * (long int)(wb_payload + lbhead);
    pbuf = (u_char *)malloc(rb_size);
    if (pbuf == NULL)
    {
        printf("Error allocating input file ring buffer memory of %ld bytes\n", rb_size);
        exit(-1);
    }
    // point to each of the write block frames
    for (i = 0; i < nfiles + 1; i++)
        for (j = 0; j < RB_CELLS; j++)
        {
            ibuf[i][j] = pbuf + (long int)(wb_payload + lbhead) * (i * RB_CELLS + j);
            ibuf_len[i][j] = wb_payload + lbhead;
        }
    // allocate and initialize output indices
    oq_ptr = (struct output_queue *)malloc(MAXBLOCKS * sizeof(struct output_queue));
    if (oq_ptr == NULL)
    {
        printf("Error allocating output index buffer\n");
        exit(-1);
    }
    // initialize to get output blocks from fill pattern
    for (i = 0; i < MAXBLOCKS; i++)
    {
        oq_ptr->bufnum = MAXFILES; // block at end has fill pattern
        oq_ptr->bufidx = 0;
    }
    // preload fill pattern into 32nd file, 0 block
    for (i = 0; i < wb_payload / sizeof(int) + 1; i++)
        *((int *)(ibuf[nfiles][0]) + i) = MK5B_FILL;

    // open all input files in async mode
    for (i = 0; i < nfiles; i++)
    {
        fdi[i] = open(argv[i + 1], O_RDONLY /*| O_DIRECT*/);
        if (fdi[i] < 0)
        {
            printf("quitting due to error opening input file %s\n", argv[i + 1]);
            perror("gather");
            exit(-1);
        }
        // read file header block
        rc = read(fdi[i], &fhd_block[i][0], FHD_SIZE);
        if (rc != FHD_SIZE)
        {
            printf("quitting due to error reading header from input file %s\n", argv[i + 1]);
            perror("gather");
            exit(-1);
        }
        nibytes[i] = FHD_SIZE;

        // trap corrupt files
        sync_word = fhd_block[i][0];
        if (sync_word != SYNC_WORD)
        {
            printf("unrecognized sync word %x in %s\n", sync_word, argv[i + 1]);
            exit(-1);
        }

        blk_size = fhd_block[i][2];
        pkt_size = fhd_block[i][4];
        pkt_per_blk = blk_size / pkt_size;

        istate[i] = IDLING; // not (yet) reading

        // form skeleton aiocbi input blocks
        aiocbi[i].aio_fildes = fdi[i];
        aiocbi[i].aio_offset = fhd_length[version];
        aiocbi[i].aio_buf = ibuf[i][0];
        aiocbi[i].aio_nbytes = wb_payload + lbhead;
        aiocbi[i].aio_reqprio = 0;
        aiocbi[i].aio_sigevent.sigev_notify = SIGEV_NONE;
        aiocbi[i].aio_lio_opcode = LIO_NOP;
    }
    // open the output file in async mode
    fdo = open(argv[nfiles + 2], O_WRONLY | O_CREAT /* | O_DIRECT */,
               S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH);
    if (fdo < 0)
    {
        printf("quitting due to error opening output file %s\n", argv[nfiles + 2]);
        perror("open");
        exit(-1);
    }
    ostate = IDLING;
    // form skeleton aiocbo output block
    aiocbo.aio_fildes = fdo;
    aiocbo.aio_offset = 0;
    aiocbo.aio_buf = ibuf[0][0];
    aiocbo.aio_nbytes = wb_payload;
    aiocbo.aio_reqprio = 0;
    aiocbo.aio_sigevent.sigev_notify = SIGEV_NONE;
    aiocbo.aio_lio_opcode = LIO_NOP;
    // read/write loop while there is still any
    // more data in the pipeline
    while (ostate != CLOSED)
    {
        did_nothing = TRUE;
        inputs_open = FALSE;
        // loop over all input files, reading if possible
        for (i = 0; i < nfiles; i++)
            if (istate[i] != CLOSED) // only examine open files
            {
                inputs_open = TRUE;
                // see if there is a read currently in progress
                if (istate[i] == IN_PROGRESS)
                {
                    // examine read status & possibly update it
                    rc = aio_error(&aiocbi[i]);
                    if (rc < 0 && rc != EINPROGRESS)
                    { // handle the error, for now just tombstone
                        perror("aio_error/read");
                        exit(-1);
                    }
                    else if (rc == 0)
                    { // read is complete
                        rc = aio_return(aiocbi + i);
                        if (rc < 0)
                        { // handle the error, for now just tombstone
                            perror("read aio_return");
                            exit(-1);
                        }
                        else if (rc == 0)
                        { // EOF detected
                            close(fdi[i]);
                            // printf ("read %ld bytes from %s\n", nibytes[i], argv[i+1]);
                            istate[i] = CLOSED;
                            did_nothing = FALSE;
                        }
                        else
                        { // bytes were read, update pointers & stati
                            niblocks[i]++;

                            switch (version)
                            {
                                // v.1 patch: detect short blocks and adjust length
                                // of block and next read pointer accordingly
                            case 1:
                                pint = (unsigned int *)(ibuf[i][wptr[i]]) + 9;
                                for (j = 1; j < pkt_per_blk; j++)
                                {
                                    if ((*(pint + j * pkt_size / 4) & 0xfffffff0) == 0xabaddee0)
                                    {
                                        rc = j * pkt_size + 4;
                                        printf("short block detect j %d word %x rc %d\n", j,
                                               *(pint + j * pkt_size / 4), rc);
                                        break;
                                    }
                                }
                                break;

                                // in v.2 format, correct read for shorter blocks
                            case 2:
                                phead = (struct wb_header_tag *)ibuf[i][wptr[i]];
                                rc = phead->wb_size;
                                break;
                            }

                            ibuf_len[i][wptr[i]] = rc;
                            nibytes[i] += rc;
                            aiocbi[i].aio_offset += rc;

                            nbl = *((int *)(ibuf[i][wptr[i]]));
                            // printf ("read block# %d from file %d length %d\n",nbl,i,rc);
                            // check for sensible block #
                            if (nbl < 0 || nbl > MAXBLOCKS)
                            {
                                if (nbadblocks < 10)
                                    printf("error: bad block# %d from %s after %d "
                                           "reads\n",
                                           nbl, argv[i + 1], niblocks[i]);
                                nbadblocks++;
                            }
                            else // good block - process it
                            {
                                (oq_ptr + nbl)->bufnum = i;
                                (oq_ptr + nbl)->bufidx = wptr[i];
                                oq_max[i] = nbl;
                                // printf("nbl %d into oq_max[%d]\n",nbl,i);
                                // advance the write block pointer by 1 cell
                                wptr[i] = ++wptr[i] % RB_CELLS;
                            }
                            istate[i] = IDLING;
                            did_nothing = FALSE;
                        }
                    }
                }

                if (istate[i] == IDLING)
                { // if currently idle check for space
                    // for another transfer into RAM
                    // (adds in RB_CELLS to keep modulo arg positive)
                    if (wptr[i] != (rptr[i] + RB_CELLS - 1) % RB_CELLS)
                    {
                        // room for another block to be read, start it
                        aiocbi[i].aio_buf = ibuf[i][wptr[i]];

                        rc = aio_read(aiocbi + i);
                        // printf("reading file %d rptr %d wptr %d\n",i,rptr[i],wptr[i]);
                        if (rc < 0)
                        { // handle the error, for now just tombstone
                            perror("aio_read");
                            exit(-1);
                        }
                        istate[i] = IN_PROGRESS;
                        did_nothing = FALSE;
                    }
                }
            } // bottom of input file loop

        // see if there is a write currently in progress
        if (ostate == IN_PROGRESS)
        {
            // examine write status
            rc = aio_error(&aiocbo);
            if (rc < 0 && rc != EINPROGRESS)
            { // handle the error, for now just tombstone
                perror("aio_error/write ");
                exit(-1);
            }
            else if (rc == 0)
            { // write is complete
                rc = aio_return(&aiocbo);
                if (rc < 0)
                { // handle the error, for now just tombstone
                    perror("write aio_return");
                    exit(-1);
                }
                // keep tally of bytes written
                nobytes += rc;
                // printf("wrote block of %d bytes\n",rc);
                aiocbo.aio_offset += rc;
                // update read point for the input buffer
                rptr[oq_ptr[oblnum].bufnum] = ++rptr[oq_ptr[oblnum].bufnum] % RB_CELLS;
                oblnum++;
                ostate = IDLING;
                did_nothing = FALSE;
            }
        }

        // regenerate the slowest reader point
        old_min_oq_max = min_oq_max;
        min_oq_max = MAXBLOCKS;
        max_oq_max = 0;
        for (k = 0; k < nfiles; k++)
        {
            if (istate[k] != CLOSED && oq_max[k] < min_oq_max)
                min_oq_max = oq_max[k];
            if (oq_max[k] > max_oq_max)
                max_oq_max = oq_max[k];
        }
        // no files were open, keep the previous write-limit
        if (min_oq_max == MAXBLOCKS)
            min_oq_max = old_min_oq_max;

        if (ostate == IDLING) // if currently idle check to see if
        {                     // we should initiate another write
            if (oblnum <= min_oq_max || !inputs_open && oblnum <= max_oq_max)
            { // yes, we aren't caught up to the slowpoke reader
                // or all are closed and we're draining buffers
                aiocbo.aio_buf = ibuf[oq_ptr[oblnum].bufnum][oq_ptr[oblnum].bufidx] + lbhead;
                aiocbo.aio_nbytes = ibuf_len[oq_ptr[oblnum].bufnum][oq_ptr[oblnum].bufidx] - lbhead;
                aio_write(&aiocbo);
                // printf("writing block# %d min_oq_max %d length %d\n",
                //         oblnum, min_oq_max, (int)aiocbo.aio_nbytes);
                ostate = IN_PROGRESS;
                did_nothing = FALSE;
            }
            else if (!inputs_open)
            { // we're caught up, and there is no more to be input
                close(fdo);
                printf("wrote %ld bytes to %s\n", nobytes, argv[nfiles + 1]);
                ostate = CLOSED;
            }
        }
        // sleep for a bit if no useful activity
        if (did_nothing)
            usleep((useconds_t)MICRO_SLEEP);
    }
    gettimeofday(&tv, &tz); // get the ending time
    elapsed = tv.tv_sec - tv0.tv_sec + 1e-6 * (tv.tv_usec - tv0.tv_usec);
    if (nbadblocks > 0)
        printf("total ignored blocks due to bad block numbers %d\n", nbadblocks);
    printf("Gather time %6.2f secs; rate %5.0f MB/s\n",
           elapsed, 1e-6 * nobytes / elapsed);
    exit(0);
}

void get_block_size(char *fname,  // pointer to file name (input)
                    int *bsize,   // pointer to block size (output)
                    int *version) // pointer to file version (output)
{
    int rc;

    FILE *fin;

    char buffer[20];

    fin = fopen(fname, "r");

    if (fin == NULL)
    {
        printf("can't open file %s\n", fname);
        exit(1);
    }
    // find out how large records are
    rc = fread(buffer, 20, 1, fin);
    if (rc != 1)
    {
        printf("Problem reading %s\n", fname);
        perror(NULL);
        exit(1);
    }
    fclose(fin); // tidy up as we go
                 // may need to change if file header changes
    *bsize = *((int *)(buffer + 8));
    *version = *((int *)(buffer + 4));

    printf("version %d block size %d\n", *version, *bsize);
}
