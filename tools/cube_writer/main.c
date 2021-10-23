#include "helper.h"

int main()
{
    if (init_cube())
        exit(1);

    while (1)
    {
        printf("Idle main thread...\n");
        usleep(5000000);
    }

    exit(0);
}
