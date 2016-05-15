#include <stdio.h>
#include <unistd.h>

char* heap_begin = NULL;

typedef struct list list;
struct list
{
    int size;
    char flag;
    list* prev;
    list* next;
};

void print_list()
{
    list* help = (list*) heap_begin;
    if (help == NULL)
    {
        printf("no memory allocated\n");
    }
    while (help != NULL)
    {
        printf("%p  %d   %d\n", ((char*) help) + sizeof(list), help->size, help->flag);
        help = help->next;
    }
    printf("\n");
}

void* my_malloc(int size)
{
    if (heap_begin == NULL)
    {
        heap_begin = sbrk(1<<20);
        list a;
        a.prev = NULL;
        a.next = NULL;
        a.size = (1<<20) - sizeof(list);
        a.flag = 0;
        list* heap_begin_new = (list*) heap_begin;
        *heap_begin_new = a;
    }
    list* stream = (list*) heap_begin;
    list* stream_copy = stream;
    int key = 0;
    while ((key == 0) && (stream_copy != NULL))
    {
        if ((stream_copy->size == size) && (stream_copy->flag == 0))
        {
            key = 1;
        }
        stream_copy = stream_copy->next;
    }
    if (key == 1)
    {
        stream_copy = stream_copy->prev;
        stream_copy->flag = 1;
        print_list();
        return ((char*) stream_copy) + sizeof(list);
    }
    else
    {
        while ((((stream->size < size + sizeof(list)) && (stream->flag == 0)) && (stream->next != NULL)) || (stream->flag == 1))
        {
            stream = stream->next;
        }
        if (size >= stream->size + sizeof(list))
        {
            sbrk(1<<20);
        }
        char* last = (char*) stream;
        last += sizeof(list) + size;
        list* new_last = (list*) last;
        list new;
        new.prev = stream;
        new.next = stream->next;
        new.flag = 0;
        new.size = stream->size - size - sizeof(list);
        *new_last = new;
        stream->flag = 1;
        stream->size = size;
        stream->next = new_last;
        print_list();
        return (char*) stream + sizeof(list);
    }
}

void my_free(void* address)
{
    list* addr = (list*) ((char*) address - sizeof(list));
    addr->flag = 0;
    if ((addr->next != NULL) && (addr->next->flag == 0))
    {
        addr->size += addr->next->size + sizeof(list);
        addr->next = addr->next->next;
        if (addr->next != NULL)
        {
            addr->next->prev = addr;
        }
    }
    if ((addr->prev != NULL) && (addr->prev->flag == 0))
    {
        addr->prev->size += addr->size + sizeof(list);
        addr->prev->next = addr->next;
        if (addr->next != NULL)
        {
            addr->next->prev = addr->prev;
        }
    }
    print_list();
}

//void* my_realloc(void* a, int size)

int main()
{
    void *a1 = my_malloc(10);
    void *a2 = my_malloc(100);
    void *a3 = my_malloc(400);
    //void *a4 = my_malloc(12);
    my_free(a1);
    my_free(a3);
    my_free(a2);
    return 0;
}