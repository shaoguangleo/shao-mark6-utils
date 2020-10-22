#define M5PAYLOADSIZE 5008
#define M5VDIFLENGTH 1254
#define CHANS_16 0x04000000
#define TWOBITS 0x04000000
// use 'm5' as station ID
#define STATION_ID 0x00006d35

// standard mk5b header definition
struct mk5b_header
{
    u_int32_t sync_word;
    u_int32_t frame_num : 15;
    u_int32_t tvg : 1;
    u_int32_t user_spec : 16;
    u_int32_t sssss : 20;
    u_int32_t jjj : 12;
    u_int32_t crcc : 16;
    u_int32_t mmmm : 16;
};
