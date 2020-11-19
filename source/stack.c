#include "stack.h"

const char* sync_path = "/home/stanislav/Documents/MIPT/3rd_semester/Computer_technologies";

//stack_t* stack_init(int size)

//extern stack_t* stack_p;


void stack_print(stack_t* stack) {
    fprintf(stdout, "Stack adress: %p\n", stack);
    fprintf(stdout, "Stack max size: %d\n", stack->m_max_size);
    fprintf(stdout, "Stack cur size: %d\n", stack->m_cur_size);
    fprintf(stdout, "Stack memory beginning: %p\n", stack->m_memory_begin);
    fprintf(stdout, "Stack memory current position: %p\n", stack->m_memory_cur);
    fprintf(stdout, "Stack memory end: %p\n", stack->m_memory_end);
}

int set_sem_set(int key) {
    
    // Deletinig semaphore set


    int sem_id = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666 );

    if (sem_id > 0 && errno != EEXIST) { // set is new, this setting the initial value of semaphore to 1

        DBG(fprintf(stdout, "Creating new semaphore set\n"))

        sem_id = semget(key, 1, IPC_CREAT | 0666);
        if (sem_id == -1) {
            perror("Error in semget(): ");
        }

        int resop  = semctl(sem_id, 0, SETVAL, 1);
        DBG(fprintf(stdout, "Setting semaphore to 1\n"))
        if (resop == -1) {
            perror("Erorr in set_sem_set(): ");
        }
    } else {
        DBG(fprintf(stdout, "Attaching old semaphore set\n"))
        sem_id = semget(key, 1, 0666);
        if (sem_id == -1) {
            perror("Error in semget(): ");
        }
    }
    return sem_id;
}

void semdel(int key) {
    int sem_id = semget(key, 1, 0666);
    if (sem_id == -1) {
        perror("Erorr in semget(): ");
    }

    int resop = semctl(sem_id, 0, IPC_RMID);
    if (resop == -1) {
        perror("Error in semctl(): ");
    }
}

void shmdel(int key, int size) {

    int id = shmget(key, size * sizeof(char), 0);
    if (id == -1) {
        perror("Erorr in shmget(): ");
    }

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

    int memory_offset = sizeof(stack_t);

    int memory_size = size;
    int total_stack_size = memory_offset + memory_size * sizeof(void*);

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

        DBG(fprintf(stdout, "Stack memory offset: %d\n", memory_offset))
    
        stack->m_cur_size = 0;
        stack->m_max_size = size;
        stack->m_memory_begin = stack + memory_offset;  
        stack->m_memory_end = stack->m_memory_begin + sizeof(void*) * stack->m_max_size;
        stack->m_memory_cur = stack->m_memory_begin;


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

int mark_destruct(stack_t* stack) {
    assert(stack != NULL);
    DBG(fprintf(stdout, "Marking stack for destruction\n"))

    int key = ftok(sync_path, SYNC);
    if (key == -1) {
        perror("Error in ftok(): ");
    }

    int id = shmget(key, 0, 0);
    if (id == -1) {
        perror("Error in shmget: ");
    }

    int resop = shmctl(id, IPC_RMID, 0);
    if (resop == -1) {
        perror("Error in marking stack for destruction: ");
    }

    return resop;

}

int push(stack_t* stack, void* val) {
    int key = ftok(sync_path, SYNC);
    if (key == -1) {
        perror("Error in ftok(): ");
    }

    // creating or attaching semaphore set
    int sem_id = set_sem_set(key);
    struct sembuf semafor = {0, -1, 0};
    semafor.sem_flg = SEM_UNDO;

    // taking semaphore
    int resop = semop(sem_id, &semafor, 1);
    if (resop == -1) {
        perror("Error in semop(): ");
    }

    DBG(fprintf(stdout, "Copying value\n"))

    stack->m_memory_begin[stack->m_cur_size] = val;
    stack->m_cur_size += 1;
    // void* ret = memcpy(stack->m_memory_cur, &val, sizeof(void*));
    // assert(ret == stack->m_memory_cur);
    stack->m_memory_cur  = stack->m_memory_cur + sizeof(void*);

    //void* dest = get_space(stack);
    // releasing semaphore
    semafor.sem_op = 1;
    resop = semop(sem_id, &semafor, 1);
    if (resop == -1) {
        perror("Error in semop(): ");
    }

    return 0;
}
