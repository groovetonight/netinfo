#include "network.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static uint32_t pack_octets(uint8_t *octets);
static NetworkClass get_network_class(uint32_t ipv4);

void print_network_info(Network* network)
{
    char* address;
    format_ipv4(network->ipv4, &address);
    printf("IPv4: %s/%hhu\n", address, network->prefix.length);
    free(address);

    printf("  Class        : %c/%hhu\n", network->class.character, network->class.prefix.length);
    
    char* network_address;
    char* broadcast_address;
    format_ipv4(network->ipv4 & network->class.prefix.mask, &network_address);
    format_ipv4((network->ipv4 & network->class.prefix.mask) + ~network->class.prefix.mask, &broadcast_address);
    printf("  Network      : %s\n", network_address);
    printf("  Broadcast    : %s\n", broadcast_address);
    free(network_address);
    free(broadcast_address);

    uint8_t subnet[4];
    subnet[0] = network->prefix.mask >> 24;
    subnet[1] = (network->prefix.mask & 0xFFFFFF) >> 16;
    subnet[2] = (network->prefix.mask & 0xFFFF) >> 8;
    subnet[3] = network->prefix.mask & 0xFF;
    printf("  Subnet mask  : %hhu.%hhu.%hhu.%hhu\n", subnet[0], subnet[1], subnet[2], subnet[3]);

    char* configuration = (char*)calloc(36, sizeof(char));
    if (configuration == 0) return;
    int j = 0;
    for (int i = 0; i < 32; i++)
    {
        configuration[i + j] = i < network->class.prefix.length ? 'n' : (i < network->prefix.length ? 's' : 'h');
        if (i == 7 || i == 15 || i == 23) configuration[i + ++j] = '.';
    }
    printf("  Configuration: %s\n", configuration);
    free(configuration);

    unsigned int subnets = (unsigned int)pow(2, network->prefix.length - network->class.prefix.length);
    unsigned int hosts = (unsigned int)pow(2, 32 - network->prefix.length) - 2;
    printf("  Subnets      : %u\n", subnets);
    printf("  Hosts/subnet : %u (usable)\n", hosts);
    printf("  Hosts        : %u (usable)\n", hosts * subnets);
    printf("  Subnet:\n");
    format_ipv4(network->ipv4 & network->prefix.mask, &network_address);
    format_ipv4((network->ipv4 & network->prefix.mask) + ((unsigned int)pow(2, 32 - network->prefix.length) - 1), &broadcast_address);
    printf("    Network    : %s\n", network_address);
    printf("    Broadcast  : %s\n", broadcast_address);
    free(network_address);
    free(broadcast_address);
}

int read_subnet_address(Network *network, char *address, char *error_msg)
{
    char addr[19];

    if (address != 0)
    {
        strncpy(addr, address, 18);
        addr[18] = '\0';
    }
    else if (fgets(addr, 19, stdin) == 0)
    {
        strcpy(error_msg, "Failed to read input");
        return 1;
    }

    uint8_t octets[4];
    uint8_t prefix;
    int matches = sscanf(addr, "%hhu.%hhu.%hhu.%hhu/%hhu",
        &octets[3], &octets[2], &octets[1], &octets[0], &prefix);

    if (matches < 4)
    {
        strcpy(error_msg, "Invalid IPv4 address");
        return 1;
    }

    network->ipv4 = pack_octets(octets);
    network->class = get_network_class(network->ipv4);

    if (network->class.character == 'D' || network->class.character == 'E')
    {
        sprintf(error_msg, "Class %c networks are not supported", network->class.character);
        return 1;
    }

    if (matches > 4 && (prefix < network->class.prefix.length || prefix > 31))
    {
        sprintf(error_msg, "Subnet prefix has to be within the bounds of the (%c) class (%u - %u)",
            network->class.character, network->class.prefix.length, 32);
        return 1;
    }

    prefix = matches > 4 ? prefix : network->class.prefix.length;
    network->prefix.length = prefix;
    network->prefix.mask = 0xFFFFFFFF << (32 - prefix);
    return 0;
}

int format_ipv4(uint32_t ipv4, char** address)
{
    uint8_t octet1 = ipv4 >> 24;
    uint8_t octet2 = (ipv4 & 0xFF0000) >> 16;
    uint8_t octet3 = (ipv4 & 0xFF00) >> 8;
    uint8_t octet4 = ipv4 & 0xFF;

    *address = (char*)calloc(16, sizeof(char));

    if (*address == 0) {
        return FAILED_ALLOCATION;
    }

    int bytes = snprintf(*address, 16, "%hhu.%hhu.%hhu.%hhu", octet1, octet2, octet3, octet4);
    return bytes > 0 ? 0 : FAILED_SCAN;
}

static uint32_t pack_octets(uint8_t *octets)
{
    return (octets[0] | (octets[1] << 8) | (octets[2] << 16) | (octets[3] << 24));
}

static NetworkClass get_network_class(uint32_t ipv4)
{
    if (ipv4 >> 31 == 0b0   ) return (NetworkClass) { 'A',  8, 0xFF000000 };
    if (ipv4 >> 30 == 0b10  ) return (NetworkClass) { 'B', 16, 0xFFFF0000 };
    if (ipv4 >> 29 == 0b110 ) return (NetworkClass) { 'C', 24, 0xFFFFFF00 };
    if (ipv4 >> 28 == 0b1110) return (NetworkClass) { 'D' };
    if (ipv4 >> 28 == 0b1111) return (NetworkClass) { 'E' };
    return (NetworkClass) { '\0', 0, 0 };
}
