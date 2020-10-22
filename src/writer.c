// writer thread - handles file writing for one file (=disk) at a time
//
// created  - rjc 2013.4.12
// initial code                                  rjc 2013.4.12

#include "dplane.h"

extern struct global_tag glob;

void *writer(void *id)
{
    int kfile = (int)(long)id; // file index for this thread (0..31)
    int rc,
        oldtype;
    useconds_t period_us;
    // allow this thread to get canceled in fwrite call
    rc = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
    if (rc)
        perror("bad setcanceltype return");

    while (TRUE)
    {
        period_us = (glob.sfile[kfile].file_open) ? OPEN_WAIT : CLOSED_WAIT;
        glob.sfile[kfile].status = READY;
        while (glob.sfile[kfile].wr_pending == FALSE)
            usleep(period_us);
        // write to file
        rc = fwrite(glob.sfile[kfile].pbuf,
                    glob.sfile[kfile].wr_size,
                    1,
                    glob.sfile[kfile].phyle);
        if (rc != 1) // handle write error return
        {
            cprintf(1, "file %d disk write error rc %d: ", kfile, rc);
            perror(NULL);
            glob.status.gsw |= GSW_WRITE_ERROR;
            // glob.stream[kstr].abort_request = TRUE;
        }
        glob.sfile[kfile].wr_pending = FALSE;
    } // do I need a bail-out condition?
}
