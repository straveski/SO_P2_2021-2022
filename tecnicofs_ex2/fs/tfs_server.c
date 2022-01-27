#include "operations.h"

#define S 1

typedef struct {
    int status;
    char pipe_name[40];
} session_id;

session_id session_id_array[S];

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return 1;
    }

    char *pipename = argv[1];
    printf("Starting TecnicoFS server with pipe called %s\n", pipename);

    /* TO DO */

    return 0;
}