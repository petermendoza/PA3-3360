#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define NTHREADS 5
#define FAMILYNAME "RINCON"

static pthread_mutex_t bsem;
static int members;
static pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

void *access_house(void *family_void_ptr)
{
    pthread_mutex_lock(&bsem);
    char fam[20];
    strcpy(fam, (char *)family_void_ptr);
    printf("%s member arrives to the house\n", fam);
    if (strcmp(fam, FAMILYNAME) != 0)
        pthread_cond_wait(&empty, &bsem);
    members++;
    printf("%s member inside the house\n", fam);
    pthread_mutex_unlock(&bsem);

    sleep(5);

    pthread_mutex_lock(&bsem);
    printf("%s member leaving the house\n", fam);
    members--;
    if (strcmp(fam, FAMILYNAME) == 0 && members == 0)
        pthread_cond_broadcast(&empty);
    pthread_mutex_unlock(&bsem);
    return NULL;
}

struct test {
public:
int *test;
};
void funcTest(test arg) {
    arg.test++;
}
int main()
{
    int mainTest, ptrTest;
    test testStruct;
    testStruct.test = &ptrTest;
    return 0;
}