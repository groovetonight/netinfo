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

int parse_subnet_address(const char* subnet_address, Network* network);
int format_address(uint32_t ipv4, char** address);
NetworkClass get_network_class(uint32_t ipv4);
void print_network_info(Network* network);
