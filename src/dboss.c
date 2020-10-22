// dboss - main program -- test harness for dplane
//
// created                                 rjc 2012.8.3

#include "dplane.h"

int main(int argc, char **argv)
{
    int i,
        im,
        id;

    unsigned int fm;

    char ascnum[6] = "01234";

    struct Abandon_Files abandon_files;
    struct Abort abort;
    struct Capture capture;
    struct New_Streams new_streams;
    struct Outfiles outfiles;
    //  struct Request_Module_Info request_module_info;
    struct Request_Status request_status;
    struct Terminate terminate;
    // responses
    //  struct Module_Info module_info;
    //  struct Status status;
    // function prototypes
    void usage(void);
    void get_device(char *, char *);

    if (argc < 2 || argc > 36)
        usage();

    switch (*argv[1])
    {
    case 'a':
        if (argc != 3)
            usage();
        abort.drain_buffers = atoi(argv[2]);
        abort.head.code = ABORT;
        sendmess(&abort, sizeof(abort), LOCAL_IP, TO_DPLANE, FALSE);
        break;

    case 'c':
        if (argc != 5)
            usage();
        memset(&capture, 0, sizeof(capture));
        if (atoi(argv[2]) == 3) // vdif time format?
        {
            capture.duration = atoi(argv[3]);
            capture.epoch_time = atoi(argv[4]);
        }
        capture.close_on_drain = TRUE;
        capture.head.code = CAPTURE;
        sendmess(&capture, sizeof(capture), LOCAL_IP, TO_DPLANE, FALSE);
        break;

    case 'f':
        abandon_files.file_mask = atoi(argv[2]);
        abandon_files.head.code = ABANDON_FILES;
        sendmess(&abandon_files, sizeof(abandon_files), LOCAL_IP, TO_DPLANE, FALSE);
        break;

    case 'i':
        if (argc != 2)
            usage();
        break;

    case 'n':
        if (argc < 4 || argc > 7 || (strcmp("vdif", argv[2]) != 0 && strcmp("mk5b", argv[2]) != 0))
            usage();
        memset(&new_streams, 0, sizeof(new_streams));
        new_streams.head.code = NEW_STREAMS;
        new_streams.num_streams = argc - 3;
        for (i = 0; i < MAXSTREAM; i++)
            if (i < new_streams.num_streams)
            {
                strncpy(new_streams.stream[i].id, argv[i + 3], 16);
                strncpy(new_streams.stream[i].format, argv[2], 8);
                if (strcmp("vdif", argv[2]) == 0)
                {
                    new_streams.stream[i].payload_size = VDIFPAYLOADSIZE;
                    new_streams.stream[i].payload_offset = UDP_HEADLEN;
                    // psn_offset depends on eVLBI standard or embedded in header
                    // new_streams.stream[i].psn_offset = 42;  // eVLBI standard
                    new_streams.stream[i].psn_offset = 66;
                }
                else if (strcmp("mk5b", argv[2]) == 0)
                {
                    new_streams.stream[i].payload_size = M5PAYLOADSIZE;
                    new_streams.stream[i].payload_offset = UDP_HEADLEN + 8;
                    new_streams.stream[i].psn_offset = UDP_HEADLEN + 4;
                }
            }
            else // null out unused stream id's
                new_streams.stream[i].id[0] = 0;
        sendmess(&new_streams, sizeof(new_streams), LOCAL_IP, TO_DPLANE, FALSE);
        break;

    case 'o':
        if (argc != 5 || strcmp(argv[2], "sg") && strcmp(argv[2], "raid"))
            usage();
        memset(&outfiles, 0, sizeof(outfiles));
        outfiles.head.code = OUTFILES;
        // copy in filename
        strncpy(outfiles.fname, argv[3], MAX_NAME);
        // set the file mode to either sg or raid
        outfiles.mode = (strcmp(argv[2], "sg") == 0) ? SG : RAID;

        // in sg mode - loop over all disks
        if (strcmp(argv[2], "sg") == 0)
        {
            fm = 0;
            for (im = 1; im < 5; im++)
                for (id = 0; id < 8; id++)
                {
                    fm >>= 1;
                    // if this module is in the group, select all its disks
                    if (strrchr(argv[4], ascnum[im]) != NULL)
                        fm |= 0x80000000;
                }
            for (i = 0; i < 4; i++) // for now, all streams share files
                outfiles.fmask[i] = fm;
        }
        // override masks for tests
        //outfiles.fmask[0] = 0x0000ffff;
        //outfiles.fmask[1] = 0xffff0000;
        //outfiles.fmask[2] = 0x00ff0000;
        //outfiles.fmask[3] = 0xff000000;

        sendmess(&outfiles, sizeof(outfiles), LOCAL_IP, TO_DPLANE, FALSE);
        break;

    case 's':
        if (argc != 3)
            usage();
        request_status.head.code = REQUEST_STATUS;
        request_status.interval = atoi(argv[2]);
        sendmess(&request_status, sizeof(request_status), LOCAL_IP, TO_DPLANE, FALSE);
        break;

    case 't':
        if (argc < 2 || argc > 3)
            usage();
        terminate.head.code = TERMINATE;
        if (argc == 3)
            terminate.force_terminate = atoi(argv[2]);
        sendmess(&terminate, sizeof(terminate), LOCAL_IP, TO_DPLANE, FALSE);
        break;

    default:
        usage();
    }
    exit(0);
}

void usage(void) // kindly educate the user
{
    printf("Usage:\n");
    printf("dboss a <drain_buffers>                  -- abort\n");
    printf("dboss c <start_cntl> <dur> <time>        -- capture data\n");
    printf("  where <start_cntl> is 0|1|2|3|4 for immed.|wall-clock|vex|vdif|mk5\n");
    printf("        <dur> is duration in secs\n");
    printf("        <time> is start time in appropropiate format\n");
    printf("dboss f <bit-mask>                       -- abandon files by bit #\n");
    printf("dboss i                                  -- request module information (NYI)\n");
    printf("dboss n <vdif|mk5b> <1..4 input devices> -- defines new stream(s)\n");
    printf("dboss o <sg|raid> <filename> <sg-group>  -- specifies output file(s)\n");
    printf("dboss s <interval secs>                  -- request status\n");
    printf("dboss t [0|1]                            -- terminate dplane\n");
    printf("  where 0|1 is do_not|do force termination without buffer drain\n");
    exit(0);
}
