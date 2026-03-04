#include "network.h"

#include <stdio.h>

int main(int argc, char** args)
{
    Network network;
    int has_error = 0;
    char error_message[256];
    if (argc > 1)
    {
        has_error = read_subnet_address(&network, args[1], error_message);
    }
    else
    {
        printf("Type an IPv4 address in the field below. It can include a subnet prefix.\n");
        printf("IP: ");
        
        if (!(has_error = read_subnet_address(&network, 0, error_message)))
        {
            printf("\n");
        }
    }

    if (has_error)
    {
        fprintf(stderr, "Error: %s\n", error_message);
        return 1;
    }

    print_network_info(&network);
    return 0;
}
