#ifndef CONF_COMMON
#define CONF_COMMON



static const int NET_HOST_DEFAULT_TIMEOUT=70;
static const int MAX_HOST_CLIENTS_INDEX=0xffff;
static const int MAX_HOST_CLIENTS_BYTES=16;

static const int NET_STATE_STOP=0;
static const int NET_STATE_CONNECTING=1;
static const int NET_STATE_ESTABLISHED=2;


static const int NET_CONNECTION_NEW = 0;
static const int NET_CONNECTION_LEAVE = 1;
static const int NET_CONNECTION_DATA = 2;

static const int NET_HEAD_LENGTH_SIZE=4;



#endif