#include "stack.h"

#define SYNC 777

//stack_t* stack;

//#define CLEAR

int main () {

    int stack_size = 10;
    char* sync_path = "/home/stanislav/Documents/MIPT/3rd_semester/Computer_technologies";

    int key = ftok(sync_path, SYNC);
    if (key == -1) {
        perror("Error in ftok(): ");
    }

    /* Deleting segment */

    #ifdef CLEAR
    fprintf(stdout, "Deleting segment\n");
    shmdel(key, stack_size);
    #else

    stack_t* stack = attach_stack(key, stack_size);
    
    int size = get_size(stack);
    DBG(fprintf(stdout, "Stack maximum size: %d\n", size));

    int cur_size = get_count(stack);
    DBG(fprintf(stdout, "Stack current size: %d\n", cur_size))

    DBG(fprintf(stdout, "Stack pointer before detaching: %p\n", stack))
    detach_stack(stack);
    //try to access stack
    DBG(fprintf(stdout, "Accessing stack\n"));
    //DBG(fprintf(stdout, "Max size: %d\n", stack->m_max_size));

    DBG(fprintf(stdout, "Stack pointer after detaching: %p\n", stack))

    while (1) {
        //waiting
    }

    #endif

}