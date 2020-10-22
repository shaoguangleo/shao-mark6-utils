// dplane - include file
//
// created  - rjc 2012.5.1
//

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <ifaddrs.h>
#include <locale.h>
#include <monetary.h>
#include <netdb.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <net/ethernet.h>
#include <pwd.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "messages.h"
#include "mk5b.h"
//#include "pfring.h"
#include "vdif.h"
#include "version.h"

struct state_tag
{
    int state;    // current execution state of command
    double tlast; // time of last execution
};

#define MAXSTREAM 4

struct global_tag
{
    struct state_tag command[20]; // state of all commands
    double tnow;                  // time of most recent system clock reading (s)
    int nstreams;                 // number of input streams
    int duration;                 // capture duration (s)
    int verbosity;                // 0|1|2 = mute | terse | prolix
    char eth0_ip[16];             // ip address of eth0 interface (e.g. "192.52.61.60")
    char bcast_addr[16];          // local b'cast address (e.g. "192.52.61.255")
    u_char *porb;                 // pointer to origin of ring buffer
    long int rb_total_size;       // ring buffer total size (bytes)
    long int rb_size;             // ring buffer (per stream) size (bytes)
    long int previous_lead;       // total lead on previous block write cycle (bytes)
    int wb_payload_size;          // write block size without header (bytes)
    int outpkt_size;              // size of packets in output file
    int numproc;                  // number of (logical) processors

    int close_streams; // signal to spoolers to close their streams
    struct stream_tag  // per stream structure
    {
        // *** input side ***
        int terminate_spooler;    // true iff time to terminate spooler for thread[i]
        int terminated_spooler;   // true iff spooler for thread[i] terminated
        long int wpind;           // (large) ring buffer write pointer (index in bytes)
        char dev[5];              // device names of input streams
        int format;               // data format (VDIF, MK5B, ...)
        int paysize;              // size of packet payload (bytes)
        int packsize;             // size of full packet (bytes) w/ e.g. UDP header
        int rb_state;             // ring buffer state (see enum rb_states below)
        int capture_enabled;      // 0|1 disable|enable input stream capture
        int tstart_vdif;          // vdif start time (secs past epoch)
        int tstop_vdif;           // vdif stop  time ( "    "     "  )
        int t_vdif;               // vdif most recently read time (")
        int t_epoch;              // epoch of most recently read time
        int abort_request;        // true iff an there is an active abort request
        int ready_to_close;       // true iff this stream is done writing to files
        long int npack_good;      // # of usable packets received since configure
        long int npack_wrong_psn; // # of discarded packets due wrong psn since configure
        long int npack_wrong_len; // # of discarded packets due to wrong length since configure
        long int nbytes;          // # of bytes received since configure

        // *** output side ***
        long int rpind; // (large) ring buffer read pointer

        // *** applies to both sides ***
        u_char *rb_origin;      // ring (sub)buffer origin for each stream
        volatile long int lead; // amount by which rb write leads read (bytes)

        // *** file connection ***
        char fpool[NFILES]; // 0|1 is file[n] in|not-in the file pool for this stream
    } stream[MAXSTREAM];
    // *** output-related for all streams ***
    int terminate_phylum;  // true iff time to terminate phylum
    int terminated_phylum; // true iff phylum terminated
    int overrun_detect;    // 0|1 is inactive|active detection logic
    int nfils;             // number of open scatter files
    struct scatter_files
    {
        int order;               // ordered indices to scatter files
        char filename[MAX_NAME]; // file name for each output thread
        FILE *phyle;             // handle for opened file
        int file_open;           // 0|1 = file is closed|open
        int status;
        int wr_pending;   // true iff there is a pending write request to writer
        int wr_size;      // write block size (bytes)
        double wr_t_init; // time at initiation of write (system clock)
        u_char *pbuf;
        long int nbytes;  // total number of bytes written to each scatter file
        long int backlog; // difference of (written bytes - bytes on disk)
    } sfile[NFILES];
    long int backlog_total; // sum of backlog over all files (i.e. cache usage)
                            // copies of most-recent incoming messages
    struct Abandon_Files abandon_files;
    struct Abort abort;
    struct Capture capture;
    struct New_Streams new_streams;
    struct Outfiles outfiles;
    struct Request_Module_Info request_module_info;
    struct Request_Status request_status;
    struct Terminate terminate;
    // copies of most-recent outgoing messages
    struct Module_Info module_info;
    struct Status status;
};

struct file_header_tag // file header - one per file
{
    unsigned int sync_word; // see SYNC_WORD - nominally 0xfeed6666
    int version;            // defines format of file
    int block_size;         // length of blocks including header (bytes)
    int packet_format;      // format of data packets, enumerated below
    int packet_size;        // length of packets (bytes)
};

struct wb_header_tag // write block header - version 2
{
    int blocknum; // block number, starting at 0
    int wb_size;  // same as block_size, or occasionally shorter
};

struct wb_header_tag_v1 // write block header - version 1
{
    int blocknum; // block number, starting at 0
};

#define MK5B_SYNC 0xabaddeed // traditional mk5b sync
#define MK5B_FILL 0x11223344 // traditional mk5b fill pattern

#define SYNC_WORD 0xfeed6666 // identifier for a proper output fileset
#define FILE_VERSION 2       // file version # defines format
#define INACTIVE -1
// write block size in characters
// 10 MB will take 20 ms at 4 Gb/s
#define WBLOCK_SIZE 10000000
// sleep between block writes if not a
// full block in the buffer (us)
#define WSLEEP_US 2000

#define NON_BLOCK 0 // flags to control blocking behavior of i/o
#define BLOCK 1
#define BUFLEN 10000          // size of local buffer for a single packet
#define UDP_HEADLEN 42        // udp header length (bytes)
#define MAX_CONSECUTIVE_MT 10 // number of consecutive empty reads w/o sleep
#define PACKET_SLEEP 100      // period (us) of packet loop sleep, if necessary

#define LOOP_PERIOD 100000 // main loop cycle time (us)

#define OPEN_WAIT 2500     // writer thread wait when file is open (us)
#define CLOSED_WAIT 200000 //   "      "      "   "    "   "  closed (us)
#define WRITER_TIMEOUT 5   // maximum allowable time to complete write (s)

#define DEFAULT_STATUS_INTERVAL 4 // between sending status messages (s)

#define LAN_MASK "192.52.61.255" // masked
#define LOCAL_IP "127.0.0.1"     // localhost's IP address
#define TO_DPLANE 7213           // incoming port# for dplane
#define FROM_DPLANE 7214         // outgoing port# for dplane

#define RESERVED_SPACE 8589934592L // set aside 8 GB for system & other code
#define MAX_PSN_GAP 1000           // max # of missing packets that can be filled in

// function prototypes
int bind2core(u_int);
char *etheraddr_string(const u_char *, char *);
void sendmess(void *, size_t, char *, int, int);

enum boolean
{
    FALSE,
    TRUE
};

enum packet_formats
{
    VDIF,
    MK5B,
    UNKNOWN_FORMAT
};

enum states
{
    IDLE,
    POSTED,
    ACTIVE
};

enum rb_states
{
    QUIESCENT,
    CONFIGURING,
    CONFIGURED,
    ARMED,
    SPOOLING,
    DRAINING
};

enum file_stati
{
    READY,
    BUSY
};
// variadic macro for conditional printf
#define cprintf(v, ...)      \
    if (v <= glob.verbosity) \
    {                        \
        printf(__VA_ARGS__); \
        fflush(stdout);      \
    }
