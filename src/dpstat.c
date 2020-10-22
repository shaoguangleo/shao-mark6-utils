// dpstat -- monitors UDP messages and displays info from status (only)
//
// initial version - based on message_handler()  rjc 2013.1.7

#include "dplane.h"
#define MAXMESS 5000 // must be >= longest message (bytes)

int main(int argc, char **argv)
{
    static int sockfd;

    U8 buff[MAXMESS];
    char sport[11];
    char desired_sender[20];

    int i,
        numbytes,
        rc,
        argu = 1;

    struct addrinfo hints,
        *servinfo,
        *p;

    struct sockaddr_storage their_addr;
    socklen_t addr_len;

    struct tm ts;
    time_t tt;

    struct header_tag *phdr;

    struct Status status;

    if (argc != 2)
    {
        printf("usage: dpstat <originating-hostname>\n\n");
        exit(1);
    }
    strncpy(desired_sender, argv[1], 20);
    printf("dpstat monitoring UDP messages from %s...\n\n", desired_sender);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    // get socket information for incoming port
    sprintf(sport, "%d", FROM_DPLANE);
    if ((rc = getaddrinfo(NULL, sport, &hints, &servinfo)) != 0)
    {
        printf("getaddrinfo: %s\n", gai_strerror(rc));
        printf("fatal error, quitting\n");
        exit(1);
    }

    // get datagram socket file descriptor
    // loop through all the results trying
    // to get a file descriptor
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("dpstat: socket call");
            continue;
        }

        // allow for multiple instances of listeners
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&argu, sizeof(int)) < 0)
        {
            perror("dpstat");
            printf("problem setting socket port for reuse, quitting\n");
            exit(1);
        }

        // found a descriptor, bind to the socket
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("dpstat: bind call");
            continue;
        }
        break;
    }

    if (p == NULL)
    {
        printf("dpstat: failed to bind socket\n");
        return -1;
    }
    // free up the linked list
    freeaddrinfo(servinfo);

    printf("\nsystem time : GSW : ring buffs (MB) : vdif times\n\n");

    while (TRUE)
    {
        if ((numbytes = recvfrom(sockfd, buff, MAXMESS - 1, 0,
                                 (struct sockaddr *)&their_addr, &addr_len)) == -1)
        {
            // report errors other than non-reception of data
            if (errno != EWOULDBLOCK)
                perror("recvfrom");
        }
        else // successful message read -- process it
        {
            phdr = (struct header_tag *)buff;

            switch (phdr->code)
            {
            case STATUS:
                memcpy(&status, buff, sizeof(status));

                if (strcmp(status.hostname, desired_sender))
                    break; // ignore messages from other senders
                           // print line of useful fields

                tt = (time_t)status.tsys;
                gmtime_r(&tt, &ts);
                printf("\n%03d-%02d%02d%02d", ts.tm_yday + 1, ts.tm_hour, ts.tm_min, ts.tm_sec);

                printf(":%08x", status.gsw);

                printf(":");
                for (i = 0; i < status.nstreams; i++)
                    printf("%7d", status.stream[i].mb_ring);

                printf(":");
                for (i = 0; i < status.nstreams; i++)
                    printf("%9d", status.stream[i].t_vdif_recent);
                break;
                // ignore all other messages (for now)
            case ABORT:
            case CAPTURE:
            case NEW_STREAMS:
            case OUTFILES:
            case REQUEST_MODULE_INFO:
            case REQUEST_STATUS:
            case TERMINATE:
            default:
                break;
            }
        }
    }
}
