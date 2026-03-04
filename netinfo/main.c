#include "network.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char** args)
{
    char address[19];
    if (argc == 1)
    {
        printf("Type an IPv4 address in the field below. It can include a subnet prefix.\n");
        printf("IP: ");
        if (fgets(address, 19, stdin) == 0)
        {
            printf("Error reading input.\n");
            return 1;
        }
        printf("\n");
    }
    else
    {
        strncpy(address, args[1], 18);
    }

    int err;
    Network network;
    if (err = parse_subnet_address(address, &network))
    {
        if (err == INVALID_SUBNET)
        {
            printf("Invalid subnet: Network prefix cannot be less than class prefix\n");
            return 0;
        }
        return err;
    }

    print_network_info(&network);
    return 0;
}
