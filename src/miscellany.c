// miscellaneous routines
//
// rjc  2012.5.3

// Bind a thread to a specific core

#include "dplane.h"

int bind2core(u_int core_id)
{
    cpu_set_t cpuset;
    int s;

    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    if ((s = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset)) != 0)
    {
        printf("Error while binding to core %u: errno=%i\n", core_id, s);
        return (-1);
    }
    else
        return (0);
}

// convert an ethernet address to a printable string

static char hex[] = "0123456789ABCDEF";

char *etheraddr_string(const u_char *ep, char *buf)
{
    u_int i, j;
    char *cp;

    cp = buf;
    if ((j = *ep >> 4) != 0)
        *cp++ = hex[j];
    else
        *cp++ = '0';

    *cp++ = hex[*ep++ & 0xf];

    for (i = 5; (int)--i >= 0;)
    {
        *cp++ = ':';
        if ((j = *ep >> 4) != 0)
            *cp++ = hex[j];
        else
            *cp++ = '0';

        *cp++ = hex[*ep++ & 0xf];
    }

    *cp = '\0';
    return (buf);
}

// transmit message to dplane via UDP broadcast
void sendmess(void *pmess,
              size_t lmess,
              char *ip_addr, // if not bcast local machine
              int port,
              int bcast) // 1 if broadcast, else 0
{
    static int first_time = TRUE,
               sequence = 13;

    static struct sockaddr_in addr;
    static int fd;

    // set up the socket only on the first time through
    if (first_time)
    {
        if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            perror("socket");
            printf("can't establish broadcast socket, quitting\n");
            exit(1);
        }
        // set up destination address
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(ip_addr);
        addr.sin_port = htons(port);
        first_time = FALSE;
        // prepare for broadcast, if desired
        if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (void *)&bcast, sizeof(int)) < 0)
        {
            perror("sendto");
            printf("problem assigning socket broadcast option, quitting\n");
            exit(1);
        }
    }

    ((struct header_tag *)pmess)->seqno = sequence++;
    if (sendto(fd, pmess, lmess, 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("sendto");
        printf("error sending message\n");
    }
}

// find device name associated with a particular file name
void get_device(char *fullpath, char *device)
{
    int i;
    char *s;
    FILE *popfil;
    char df_command[256], line[256];

    sprintf(df_command, "df %s", fullpath);
    popfil = popen(df_command, "r");
    if (popfil == NULL)
    {
        printf("problem with df command\n");
        perror("get_device");
        return;
    }
    // access first field on 2nd line
    s = fgets(line, 256, popfil);
    s = fgets(line, 256, popfil);

    pclose(popfil);

    strncpy(device, strchr(line + 1, '/') + 1, 8);
    // clean up string, remove trailing blanks
    device[7] = 0;
    for (i = 1; i < 7; i++)
        if (device[i] == ' ')
            device[i] = 0;
    printf("path %s device %s\n", fullpath, device);
}
