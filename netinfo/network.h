#pragma once

#include <stdint.h>

#define FAILED_ALLOCATION 1
#define FAILED_SCAN 2
#define INVALID_SUBNET 3

typedef struct
{
    uint8_t length;
    uint32_t mask;
} NetworkPrefix;

typedef struct
{
    char character;
    NetworkPrefix prefix;
} NetworkClass;

typedef struct
{
    uint32_t ipv4;
    NetworkPrefix prefix;
    NetworkClass class;
} Network;

void print_network_info(Network* network);
int read_subnet_address(Network *network, char *address, char *error_msg);
int format_ipv4(uint32_t ipv4, char** address);
