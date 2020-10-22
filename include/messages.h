//  binary messages to/from dplane
//                                          rjc  2012.8.3

#define MAX_NAME 256 // max size of a filename (bytes)
#define NFILES 32    // max # of scattered files

typedef unsigned char U8;
typedef unsigned short int U16;
typedef unsigned int U32;

struct header_tag
{
    U32 code;  // command identifier - small integer; see enum below
    U32 seqno; // message identifier - increments for originating msgs.
};

struct Abandon_Files        // c_plane --> dplane
{                           // forcibly shuts down file writing for a subset of disks
    struct header_tag head; // message code & seqno
    int file_mask;          // iff bit n is set, quit all access to file n
};

struct Abort                // c_plane --> dplane
{                           // aborts current capture and/or file writing
    struct header_tag head; // message code & seqno
    int drain_buffers;      // iff true allow buffers to drain after capture stop
};

struct Capture              // c_plane --> dplane
{                           // initiates/stops capture of data streams
    struct header_tag head; // message code & seqno
    int start_control;      // 0|1|2|3|4 for immediate|wall-clock|vex|vdif|mk5
    int unix_time;          // wall-clock time: roughly secs since 1970.0
    char vex_time[23];      // start epoch in vex format (e.g. 2012y212d09h26m26.125s)
    int epoch_time;         // start time since vdif epoch (s)
    int duration;           // length of capture (s)
    char scan_name[32];     // arbitrary scan identifier
    char exp_name[8];       // arbitrary experiment identifier
    char stn_code[8];       // arbitrary station identifier
    int write_to_disk;      // 0|1 for do not | do write data out to file(s) (NYI)
    int close_on_drain;     // 0|1 for do not | do close files when capture complete
};

struct Module_Info // dplane --> c_plane
{
    struct header_tag head; // message code & seqno
};

struct New_Streams          // c_plane --> dplane
{                           // defines new input data streams
    struct header_tag head; // message code & seqno
    int num_streams;        // 0..4
    struct stream_def
    {
        char id[16];        // interface ID (e.g. eth2)
        char format[8];     // data format (e.g. vdif or mk5b)
        int payload_offset; // size of non-payload portion of packet (bytes)
        int payload_size;   // application payload size (bytes)
        int psn_offset;     // byte offset to pkt ser# field (-1: don't check psn)
        char name[16];      // arbitrary identifier for debug messages
        int source_ip[6];   // IP address of source
        int port;           // incoming port number
    } stream[4];
};

struct Outfiles             // c_plane --> dplane
{                           // gives output filenames for subsequent capture
    struct header_tag head; // message code & seqno
    int mode;               // 1 | 2 = sg | raid
    unsigned int fmask[4];  // sg mode only: mask of active disks bit n set for
                            // n = 8 * (module - 1) + disk#; indexed by stream
    char fname[MAX_NAME];   // scan's file name (with no directory structure)
};

struct Request_Module_Info // c_plane --> dplane
{
    struct header_tag head; // message code & seqno
    int interval;           // in sec; 0 to stop
};

struct Request_Status       // c_plane --> dplane
{                           // ask for status message(s) to be sent
    struct header_tag head; // message code & seqno
    int interval;           // in sec; 0 to stop
};

struct Status               // dplane --> c_plane
{                           // current status of dplane software and mk6 hardware
    struct header_tag head; // message code & seqno
    double tsys;            // recent system time (s since 1970.0)
    char hostname[20];      // host's name, null-terminated
    int gsw;                // general status word - see bit defs below
    int gsw_fileno;         // for gsw b1, the # of the file involved
    int nstreams;           // number of active streams
    struct stream_stat
    {
        int mb_ring;              // amount of data in stream's ring buffer (rounded MB)
        int t_vdif_recent;        // last-read time since vdif epoch (sec)
        int t_epoch_recent;       //   "   "   vdif epoch
        long int npack_good;      // # of usable packets received since stream configured
        long int npack_wrong_psn; // # of discarded packets due wrong psn since configure
        long int npack_wrong_len; // # of discarded packets due to wrong length since configure
        long int nbytes;          // # of bytes received since configure
    } stream[4];
    int mb_cache;     // amount of data in file cache (rounded MB)
    int file_timeout; // bit n is set iff write to file n has timed out
};

struct Terminate            // c_plane --> dplane
{                           // terminate dplane
    struct header_tag head; // message code & seqno
    int force_terminate;    // 0|1 = don't|do terminate even if data still draining
};

// GSW bit definitions
#define GSW_FACTIVE_ERR 0x00000001
#define GSW_FOPEN_ERR 0x00000002
#define GSW_NO_STREAMS_ERR 0x00000004
#define GSW_ACTIVE_STREAMS_ERR 0x00000008
#define GSW_BUFFER_OVERRUN_ERR 0x00000010
#define GSW_WRITE_ERROR 0x00000020
#define GSW_M5PKT_SEQUENCE_ERR 0x00000040
#define GSW_NO_FILES_ERR 0x00000080
#define GSW_STR0_SEQUENCE_ERR 0x00000100
#define GSW_STR1_SEQUENCE_ERR 0x00000200
#define GSW_STR2_SEQUENCE_ERR 0x00000400
#define GSW_STR3_SEQUENCE_ERR 0x00000800
#define GSW_STR0_SPOOLING 0x00001000
#define GSW_STR1_SPOOLING 0x00002000
#define GSW_STR2_SPOOLING 0x00004000
#define GSW_STR3_SPOOLING 0x00008000
#define GSW_FILES_OPEN 0x00010000
#define GSW_PFRING_OPEN_ERR 0x00020000

#define GSW_ALL_ERRORS (GSW_FACTIVE_ERR | GSW_FOPEN_ERR | GSW_NO_STREAMS_ERR | GSW_ACTIVE_STREAMS_ERR | GSW_BUFFER_OVERRUN_ERR | GSW_WRITE_ERROR | GSW_M5PKT_SEQUENCE_ERR | GSW_NO_FILES_ERR | GSW_STR0_SEQUENCE_ERR | GSW_STR1_SEQUENCE_ERR | GSW_STR2_SEQUENCE_ERR | GSW_STR3_SEQUENCE_ERR | GSW_PFRING_OPEN_ERR)

// command tokens
enum command_tag
{
    ABANDON_FILES,
    ABORT,
    CAPTURE,
    MODULE_INFO,
    NEW_STREAMS,
    OUTFILES,
    REQUEST_MODULE_INFO,
    REQUEST_STATUS,
    STATUS,
    TERMINATE,
    NUM_COMMANDS // add new commands *above* this line
};

// outfiles mode possibilities
#define SG 1
#define RAID 2
