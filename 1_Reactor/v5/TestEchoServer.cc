#include "EchoServer.h"

int main(int argc, char **argv)
{
    EchoServer es(4, 10, "127.0.0.1", 8888);
    es.start();
    return 0;
}

