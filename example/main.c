#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/databaseADT.h"
#include "../queue/queueADT.h"

/* Restrictions for users */
#define USER_NAME_MAX_LEN 50
#define USER_PASS_MAX_LEN 50
#define USER_MAIL_MAX_LEN 50

typedef struct user_t
{
    char name[USER_NAME_MAX_LEN+1];
    char pass[USER_PASS_MAX_LEN+1];
    char mail[USER_MAIL_MAX_LEN+1];
} user_t;

/*Static functions prototypes for user's queue*/
static void *cpyUserQ(void *ptr);
static void freeUserQ(void *ptr);

/*Queries*/
void listUsers(databaseADT db, const char *name);
void addUser(databaseADT db, const char *user, const char *password,
        const char *email, const char *name);

int testDB(const char *name);

int main(void)
{
    pid_t pid;
    databaseADT db = NULL;
    char *path = "./database.db";
    char *schema = "./schema.sql";
    FILE *errLog = NULL;

    if ( (errLog = fopen("error.log", "w")) == NULL )
    {
        fprintf(stderr, "error.log couldn't be opened\n");
        return 1;
    }

    if ( (NewDatabaseADT(&db, path, errLog)) != DB_SUCCESS )
    {
        fprintf(stderr, "NewDatabaseADT failed\n");
        return 1;
    }

    if ( (DBBuildDatabase(db, schema)) != DB_SUCCESS)
    {
        fprintf(stderr, "DBBuildDatabase failed\n");
        return 1;
    }

    fclose(errLog);
    FreeDatabaseADT(db);

    switch(pid = fork())
    {
        case -1:
            printf("Fork error\n");
            return -1;

        case 0:
            testDB("CHILD");
            break;

        default:
            testDB("PADRE");
            break;
    }

    return 1;
}

int
testDB(const char *name)
{
    databaseADT db = NULL;
    char *path = "./database.db";
    char *schema = "./schema.sql";
    FILE *errLog = NULL;
    int i;

    if ( (errLog = fopen("error.log", "w")) == NULL )
    {
        fprintf(stderr, "%s error.log couldn't be opened\n", name);
        return 1;
    }

    if ( (NewDatabaseADT(&db, path, errLog)) != DB_SUCCESS )
    {
        fprintf(stderr, "%s NewDatabaseADT failed\n", name);
        return 1;
    }

    for(i=0; i<50; i++)
    {
        char tmp[100];
        sprintf(tmp, "%s%d", name, i);
        printf("%s\n", tmp);
        addUser(db, tmp, "pass", "e@mail.com", name);
    }

    listUsers(db, name);

    fclose(errLog);
    FreeDatabaseADT(db);
    return 0;
}

void
addUser(databaseADT db, const char *user, const char *password,
        const char *email, const char *name)
{
    int ret;

    ret = DBaddUser(db, user, password, email);

    switch (ret)
    {
            case DB_SUCCESS:
                    printf("%s inserted correctly! @ %s\n", user, name);
                    break;

            case DB_ALREADY_EXISTS:
                    printf("%s already exists!@ %s\n", user, name);
                    break;

            default:
                    printf("Database error!@ %s\n", name);
                    break;
    }
    return;
}

void
listUsers(databaseADT db, const char *name)
{
    int ret;
    queueADT queue;
    int i = 1;
    user_t *uq;

    if ((queue=newQueue(cpyUserQ, freeUserQ)) == NULL)
        return;

    ret = DBgetUserQueue(db, queue);

    switch (ret)
    {
            case DB_SUCCESS:
                break;

            default:
                printf("Database error\n");
                break;

    }

    while ((uq = dequeue(queue)) != NULL )
    {
        printf("Leyendo %d - user: %s, pass: %s, mail: %s @%s\n",
                i, uq->name, uq->pass, uq->mail, name);
        free(uq);
        i++;
        putchar('\n');
    }

    freeQueue(queue);
    return;
}

void *
cpyUserQ(void *ptr)
{
    user_t *uq;

    if ((uq = malloc(sizeof(user_t))) == NULL)
        return NULL;

    memcpy(uq, ptr, sizeof(user_t));

    return (void *)uq;
}

void
freeUserQ(void *ptr)
{
    if (ptr != NULL)
        free(ptr);

    return;
}
