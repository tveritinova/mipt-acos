#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/sem.h>

char title(char a)
{
    return a - 'a' + 'A';
}

char no_title(char a)
{
    return a - 'A' + 'a';
}

int process_string(char* addr2, int str_addr, char* addr3, int fin_addr, int str_cnt)
{
    int counter_string = 0;
    int counter_res = fin_addr;
    int char_key = 0;
    char stream = addr2[str_addr];
    while (stream != '\n')
    {
        if ((stream >= 'A') && (stream <= 'Z'))
        {
            addr3[counter_res] = no_title(stream);
            addr3[counter_res + 1] = no_title(stream);
            counter_res += 2;
            char_key = 1;
        }
        if ((stream >= 'a') && (stream <= 'z'))
        {
            addr3[counter_res] = title(stream);
            ++counter_res;
            char_key = 1;
        }
        if (char_key == 0)
        {
            addr3[counter_res] = stream;
            ++counter_res;
        }
        char_key = 0;
        ++counter_string;
        stream = addr2[str_addr + counter_string];
    }
    //addr3[counter_res] = 0;
    //printf("\nLOOOOOOOOK!!!!!    %s\n", &addr3[sizeof(int) * (str_cnt * 2 + 1)]);
    return counter_res;
}

int string_count(int fd)
{
    int res = 0;
    char stream;
    int er_read = read(fd, &stream, 1);
    while (er_read == 1)
    {
        if (stream == '\n')
        {
            ++res;
        }
        er_read = read(fd, &stream, 1);
    }
    return res;
}

void process_with_shm1(int shm1, int shm2, int shm2_size)
{
    int er_read;
    int sem = semget(IPC_PRIVATE, 2, IPC_CREAT | 0660);
    if (sem == -1)
    {
        perror("semget");
    }
    int* addr1 = shmat(shm1, 0, 0);
    if (addr1 == -1)
    {
        perror("shmat 1");
    }
    addr1[0] = sem;
    
    char* addr2 = shmat(shm2, 0, 0);
    addr2[shm2_size - 1] = '\n';
    
    
    int counter_file = 0;
    int counter_string = 0;
    int start_number = 0;
    int key = 1;
    char c;
    while(1)
    {
        c = addr2[counter_file];
        if (counter_file >= shm2_size - 1)
        {
            break;
        }
        if (key == 1)
        {
            addr1[1 + counter_string] = counter_file;
            ++counter_string;
            key = 0;
        }
        if (c == '\n')
        {
            key = 1;
        }
        ++counter_file;
    }
    
    /*for (int i = 1; i < counter_string + 1; i++)
    {
        printf("%d\n", addr1[i]);
    }*/
    shmdt(addr2);
    
    shmdt(addr1);
}

void process_with_shm2(int shm2, char* file_name, int file_size)
{
    char* addr2 = shmat(shm2, 0, 0);
    if (addr2 == -1)
    {
        perror("shmat");
    }
    int fd = open(file_name, O_RDONLY);
    int er_read = read(fd, addr2, file_size);
    if (er_read == -1)
    {
        perror("read");
    }
    shmdt(addr2);
    close(fd);
}

void process_with_shm3(int shm3, int str_cnt)
{
    char* addr3 = shmat(shm3, 0, 0);
    if (addr3 == -1)
    {
        perror("shmat in process shm3");
    }
    ((int*) addr3)[0] = (str_cnt * 2 + 1) * sizeof(int);
    shmdt(addr3);
}

void process_child(int shm1, int shm2, int shm3, int str_cnt)
{
    static struct sembuf lock_1[2];
    lock_1[0].sem_num = 0;
    lock_1[0].sem_op = 0;
    lock_1[0].sem_flg = 0;
    
    lock_1[1].sem_num = 0;
    lock_1[1].sem_op = 1;
    lock_1[1].sem_flg = 0;
    
    static struct sembuf lock_2[2];
    lock_2[0].sem_num = 1;
    lock_2[0].sem_op = 0;
    lock_2[0].sem_flg = 0;
    
    lock_2[1].sem_num = 1;
    lock_2[1].sem_op = 1;
    lock_2[1].sem_flg = 0;
    
    static struct sembuf unlock_1[1];
    unlock_1[0].sem_num = 0;
    unlock_1[0].sem_op = -1;
    unlock_1[0].sem_flg = 0;
    
    static struct sembuf unlock_2[1];
    unlock_2[0].sem_num = 1;
    unlock_2[0].sem_op = -1;
    unlock_2[0].sem_flg = 0;
    
    
    int* addr1;
    char *addr2, *addr3;
    int sem, er_semop, i, key, counter_header, next_empty, str_addr;
    counter_header = 1;
    
    addr1 = shmat(shm1, 0, 0);
    if (addr1 == -1)
    {
        perror("shmat shm1");
    }
    
    addr2 = shmat(shm2, 0, 0);
    if (addr2 == -1)
    {
        perror("shmat shm2");
    }
    
    addr3 = shmat(shm3, 0, 0);
    if (addr3 == -1)
    {
        perror("shmat shm3");
    }
    sem = addr1[0];
    
    while(1)
    {
        er_semop = semop(sem, lock_1, 2);
        if (er_semop == -1)
        {
            perror("semop");
        }
        i = 1;
        key = 0;
        while((i <= str_cnt) && (key == 0))
        {
            if (addr1[i] != -1)
            {
                str_addr = addr1[i];
                addr1[i] = -1;
                key = 1;
            }
            ++i;
        }
        er_semop = semop(sem, unlock_1, 1);
        if (er_semop == -1)
        {
            perror("semop");
        }
        if (key == 0)
        {
            shmdt(addr1);
            shmdt(addr2);
            shmdt(addr3);
            break;
        }
        
        er_semop = semop(sem, lock_2, 2);
        if (er_semop == -1)
        {
            perror("semop");
        }
        next_empty = process_string(addr2, str_addr, addr3, ((int*) addr3)[0], str_cnt);
        ((int*) addr3)[counter_header] = ((int*) addr3)[0];
        ((int*) addr3)[counter_header + 1] = next_empty - 1;
        ((int*) addr3)[0] = next_empty;
        counter_header += 2;
        er_semop = semop(sem, unlock_2, 1);
        if (er_semop == -1)
        {
            perror("semop");
        }
    }
}

void process_parent(int shm3, int str_cnt)
{
    char* addr3 = shmat(shm3, 0, 0);
    if (addr3 == -1)
    {
        perror("shmat in pr parent");
    }
    for (int i = 0; i < str_cnt * 2 ; i += 2)
    {
        //printf("%d %d\n", ((int*) addr3)[1 + i], ((int*) addr3)[1 + i + 1]);
        for (int j = ((int*) addr3)[1 + i]; j <= ((int*) addr3)[1 + i + 1]; j++)
        {
            printf("%c", addr3[j]);
        }
        printf("\n");
    }
    shmdt(addr3);
}

int main(int argc, char** argv)
{
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1)
    {
        perror("open");
    }
    struct stat stat1;
    fstat(fd, &stat1);
    int str_cnt = string_count(fd);
    close(fd);
    
    int shm2 = shmget(IPC_PRIVATE, sizeof(char) * (stat1.st_size + 1) + 1, IPC_CREAT | 0660);
    if (shm2 == -1)
    {
        perror("shmget 2");
    }
    process_with_shm2(shm2, argv[1], stat1.st_size);
    
    int shm1 = shmget(IPC_PRIVATE, sizeof(int) * (str_cnt + 1), IPC_CREAT | 0660);
    if (shm1 == -1)
    {
        perror("shmget 1");
    }
    process_with_shm1(shm1, shm2, sizeof(char) * (stat1.st_size + 1) + 1);
    
    
    int shm3 = shmget(IPC_PRIVATE, stat1.st_size * sizeof(char) + (str_cnt * 2 + 1) * sizeof(int) , IPC_CREAT | 0660);
    if (shm3 == -1)
    {
        perror("shmget 3");
    }
    process_with_shm3(shm3, str_cnt);
    
    
    int pr_number = atoi(argv[2]);
    int pr;
    char stream;
    for (int i = 0; i < pr_number; i++)
    {
        pr = fork();
        if (pr == 0)
        {
            //printf("in child\n\n");
            process_child(shm1, shm2, shm3, str_cnt);
            _exit(EXIT_SUCCESS);
        }
    }
    
    for (int i = 0; i < pr_number; ++i)
    {
        wait(0);
    }
    
    process_parent(shm3, str_cnt);
    
    
    close(fd);
    shmctl(shm1, IPC_RMID, 0);
    shmctl(shm2, IPC_RMID, 0);
    shmctl(shm3, IPC_RMID, 0);
    return 0;
}
