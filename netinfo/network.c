#include "network.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static uint32_t pack_octets(uint8_t *octets);
static void unpack_octets(uint32_t ipv4, uint8_t *octets);
static NetworkClass get_network_class(uint32_t ipv4);
static int build_prefix_configuration(uint8_t network_prefix, uint8_t subnet_prefix, char **configuration);
static void print_ipv4(char *prefix, uint32_t ipv4);

int print_network_info(Network* network)
{
    uint8_t octets[4];
    unpack_octets(network->ipv4, octets);
    printf("IPv4: %hhu.%hhu.%hhu.%hhu/%hhu\n", octets[3], octets[2], octets[1], octets[0], network->prefix.length);

    printf("  Class        : %c/%hhu\n", network->class.character, network->class.prefix.length);
    print_ipv4("  Network      : ", network->ipv4 & network->class.prefix.mask);
    print_ipv4("  Broadcast    : ", (network->ipv4 & network->class.prefix.mask) + ~network->class.prefix.mask);

    uint8_t subnet_mask[4];
    unpack_octets(network->prefix.mask, subnet_mask);
    printf("  Subnet mask  : %hhu.%hhu.%hhu.%hhu\n", subnet_mask[3], subnet_mask[2], subnet_mask[1], subnet_mask[0]);

    char *configuration;
    if (build_prefix_configuration(network->class.prefix.length, network->prefix.length, &configuration))
    {
        return FAILED_ALLOCATION;
    }

    printf("  Configuration: %s\n", configuration);
    free(configuration);

    unsigned int subnets = (unsigned int)pow(2, network->prefix.length - network->class.prefix.length);
    unsigned int hosts = (unsigned int)pow(2, 32 - network->prefix.length);
    printf("  Subnets      : %u\n", subnets);
    printf("  Hosts/subnet : %u (usable)\n", hosts - 2);
    printf("  Hosts        : %u (usable)\n", (hosts - 2) * subnets);
    printf("  Subnet:\n");
    print_ipv4("    Network    : ", network->ipv4 & network->prefix.mask);
    print_ipv4("    Broadcast  : ", (network->ipv4 & network->prefix.mask) + (hosts - 1));
    return 0;
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

static uint32_t pack_octets(uint8_t *octets)
{
    return (octets[0] | (octets[1] << 8) | (octets[2] << 16) | (octets[3] << 24));
}

static void unpack_octets(uint32_t ipv4, uint8_t *octets)
{
    octets[0] = ipv4 & 0xFF;
    octets[1] = (ipv4 & 0xFFFF) >> 8;
    octets[2] = (ipv4 & 0xFFFFFF) >> 16;
    octets[3] = ipv4 >> 24;
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

static int build_prefix_configuration(uint8_t network_prefix, uint8_t subnet_prefix, char **configuration)
{
    char *format = (char*)malloc(36 * sizeof(char));

    if (format == 0)
    {
        return FAILED_ALLOCATION;
    }

    for (int i = 0, j = 0; i < 32; i++)
    {
        format[i + j] = i < network_prefix ? 'n' : (i < subnet_prefix ? 's' : 'h');

        if (i == 7 || i == 15 || i == 23)
        {
            format[i + ++j] = '.';
        }
    }

    format[35] = '\0';
    *configuration = format;
    return 0;
}

static void print_ipv4(char *prefix, uint32_t ipv4)
{
    uint8_t octets[4];
    unpack_octets(ipv4, octets);
    printf("%s%hhu.%hhu.%hhu.%hhu\n", prefix, octets[3], octets[2], octets[1], octets[0]);
}
