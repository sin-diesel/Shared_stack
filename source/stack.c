#include "stack.h"

//stack_t* stack_init(int size)

//extern stack_t* stack_p;

void shmdel(int key, int size) {
    int id = shmget(key, size * sizeof(char), IPC_CREAT);
    int resop = shmctl (id , IPC_RMID , NULL);
    if (resop == -1) {
        perror("Error in shmctl(): ");
    }
}

stack_t* attach_stack(key_t key, int size) {

    assert(size > 0);
    assert(key != -1);
    int resop = 0;
    stack_t* stack = NULL;

    int memory_size = size;
    int total_stack_size = memory_size + 
                           sizeof(stack->m_cur_size) + 
                           sizeof(stack->m_max_size) + 
                           sizeof(stack->m_memory);
    DBG(fprintf(stdout, "Total stack size: %d bytes\n", total_stack_size))

    int id = shmget(key, total_stack_size * sizeof(char), IPC_CREAT | IPC_EXCL | 0666);

    if (id == -1) {
        perror("Possible error in shmget(): ");

        fprintf(stdout, "Stack exists, attaching the stack\n");

        id = shmget(key, total_stack_size * sizeof(char), 0666);
        if (id == -1) {
            perror("Error in shmget: ");
        }

        stack = shmat(id, NULL, 0);
        if (stack == (void*) -1) {
            perror("Error in shmat(): ");
            return NULL;
        }

        return stack;
    } else {
        fprintf(stdout, "No stack exists, creating new stack\n");
        
        id = shmget(key, total_stack_size * sizeof(char), IPC_CREAT | 0666);
        if (id == -1) {
            perror("Error in shmget(): ");
        }
        stack = shmat(id, NULL, 0);
        if (stack == (void*) -1) {
            perror("Error in shmat(): ");
            return NULL;
        }

        stack->m_cur_size = 0;
        stack->m_max_size = size;

        int memory_offset = sizeof(stack) + 
                            sizeof(stack->m_cur_size) +
                            sizeof(stack->m_max_size) + 
                            sizeof(stack->m_memory);

        DBG(fprintf(stdout, "Stack memory offset: %d\n", memory_offset))
        stack->m_memory = stack + memory_offset;

        return stack;
    }


}

int get_size(stack_t* stack) {
    assert(stack != NULL);
    int size = stack->m_max_size;

    return size;
}

int get_count(stack_t* stack) {
    assert(stack != NULL);
    int count = stack->m_cur_size;
    
    return count;
}


int detach_stack(stack_t* stack) {
    int resop = 0;
    resop = shmdt(stack);
    if (resop == -1) {
        perror("Error in detaching stack with shmdt(): ");
    }

    //stack = NULL;

    //stack = POISON;

    return resop;
}
