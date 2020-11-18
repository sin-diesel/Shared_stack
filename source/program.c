#include "stack.h"

#define SYNC 777

int main () {

    int stack_size = 10;
    char* sync_path = "/home/stanislav/Documents/MIPT/3rd_semester/Computer_technologies";

    int key = ftok(sync_path, SYNC);
    if (key == -1) {
        perror("Error in ftok(): ");
    }

    /* Deleting segment */

    stack_t* stack = attach_stack(key, stack_size);
    while (1) {
        
    }
    //shmdel(key, stack_size);
}