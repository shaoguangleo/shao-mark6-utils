// scat_write ()  performs scattered write from circular
// buffer to multiple files
//
// created  -                                     rjc 2012.9.5
// modify to cache-equalization algorithm         rjc 2012.11.1
// concept is that by keeping the difference between sectors written
// from the program's pov, and sectors actually written to disk, we
// can keep any slow disk from getting overused and dominating the
// cache
// changed algorithm to multi-threaded writer     rjc 2013.4.23
// support writing to file pools, based on stream rjc 2014.6.3

#include "dplane.h"
#define NOT_USED_AT_ALL 9999999999999999L

extern struct global_tag glob;

int scat_write(u_char *pbuf, // pointer to buffer containing data
               int wr_size,  // size of write block (bytes)
               int kstr)     // stream index for block to be written
{
    int i,
        ii,
        ileast,
        rc,
        k;
    // file index for starting point of next write tries
    static int next[4] = {0, 0, 0, 0};

    long int least_used;

    // find least-used disk (for current scan)
    least_used = NOT_USED_AT_ALL;
    rc = -1;
    for (i = 0; i < glob.nfils; i++)
    {
        // use cyclical starting point (distributes ties)
        ii = (next[kstr] + i) % glob.nfils;
        if (glob.stream[kstr].fpool[ii] && glob.sfile[ii].file_open && glob.sfile[ii].status == READY && glob.sfile[ii].nbytes < least_used)
        {
            least_used = glob.sfile[ii].nbytes;
            ileast = ii;
        }
    }
    // check for time-outs on file write
    for (i = 0; i < glob.nfils; i++)
        if (glob.stream[kstr].fpool[i] && glob.sfile[i].wr_pending == TRUE && glob.tnow > glob.sfile[i].wr_t_init + WRITER_TIMEOUT)
        {
            glob.status.file_timeout |= (1 << i);
            // reset timeout clock to space error messages
            glob.sfile[i].wr_t_init = glob.tnow;
        }

    if (least_used < NOT_USED_AT_ALL)
    {
        // instruct writer to write to it
        glob.sfile[ileast].pbuf = pbuf;
        glob.sfile[ileast].wr_size = wr_size;
        glob.sfile[ileast].status = BUSY;
        glob.sfile[ileast].wr_pending = TRUE;
        // keep track of file size (# bytes written)
        glob.sfile[ileast].nbytes += wr_size;
        // update cyclical starting point
        for (i = 1; i < glob.nfils + 1; i++)
        {
            // use cyclical starting point (distributes ties)
            ii = (next[kstr] + i) % glob.nfils;
            if (glob.stream[kstr].fpool[ii])
                break; // we've found next start for stream kstr
        }
        // now adjust all streams that use that file
        for (k = 0; k < 4; k++)
            if (glob.stream[k].fpool[ii])
                next[k] = ii;
        rc = ileast;
    }
    // successful: return index of file written
    // otherwise return -1
    return rc;
}
