/**
*   @file databaseADT.c
*   Database ADT
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../sqlite/sqlite3.h"
#include "../include/databaseADT.h"

/* Restrictions for users */
#define USER_NAME_MAX_LEN 50
#define USER_PASS_MAX_LEN 50
#define USER_MAIL_MAX_LEN 50

typedef struct databaseCDT
{
    sqlite3 *dbHandle;
    char *dbFile;
    FILE *logFile;
} databaseCDT;

typedef struct blobBindings
{
    void *data;
    int size;
    int param;
} blobBindings;

typedef struct user_t
{
    char name[USER_NAME_MAX_LEN+1];
    char pass[USER_PASS_MAX_LEN+1];
    char mail[USER_MAIL_MAX_LEN+1];
} user_t;

/**
 * Writes text to errFile.
 *
 * @param[in]   errFile Error file to write to.
 * @param[in]   text    Text to write.
*/
static void
logError (FILE *errFile, const char *text, ...)
{
    va_list ap;

    if (errFile == NULL || text == NULL)
        return;

    va_start( ap, text );

    vfprintf( errFile, text, ap );
    fprintf( errFile, "\n" );

    va_end( ap );

    return;
}

   /******************************************************/
   /** StepSql:                                         **/
   /** This encapsulates sqlite step call to handle     **/
   /** timeout condition.                               **/
   /******************************************************/
int StepSql(databaseADT db, sqlite3_stmt *statement);

   /******************************************************/
   /** PrepareSql:                                      **/
   /** This encapsulates sqlite prepare call to handle  **/
   /** timeout condition.                               **/
   /******************************************************/
int PrepareSql(databaseADT db, char *SqlStr, int queryLen,
            sqlite3_stmt **statement, const char **tail);

/**
 * Retrieves a sanitized copy of the given string.
 *
 * @param[in]   db      The database to be used.
 * @param[in]   str     The string to be sanitized.
 *
 * @return      Sanitized copy of the string provided.
 *
 * @remarks     The caller is responsible to free the returned string.
*/
static char *EscapeString( databaseADT db, const char *str );

/**
 * Executes the given query.
 *
 * @param[in]           db              The database instance.
 * @param[in][out]      statement       The statement structure to be filled
 *                                      when preparing the query.
 * @param[in]           sql             The SQL query to perform. Should have
 *                                      a "%s" in each place an argument should
 *                                      be placed.
 * @param[in]           bindingCount    Number of elements in the binsing array.
 * @param[in]           bindings        Allows to bind blobs. All other types
 *                                      can be isnerted into the query itself.
 * @param[in]           args            Number of arguments to the sql query.
 *
 * @return      An error code if the argument validation failed or the value
 *              returned by SQLite when executing the query.
 *
 * @remarks     All variable arguments passed MUST be char*. They will be
 *              validated against ilegal characters such as \' and \".
*/
static int QueryExecute( databaseADT db, sqlite3_stmt **statement,
                        const char* sql, int bindingCount,
                        blobBindings* bindings, int args, ... );

static long DBSize(databaseADT db);

static long
DBSize(databaseADT db)
{
    long size;
    struct stat st;

    stat(db->dbFile, &st);
    size = st.st_size;

    return size;
}

DB_ERR DBBuildDatabase( databaseADT db, const char *schema )
{
    char sqlStr[1024] = {0};
    char line[LINE_MAX];
    FILE *fp = NULL;
    int ret;

    if ( db == NULL || schema == NULL)
            return DB_INVALID_ARG;

    if (DBSize(db) > 0)
        return DB_ALREADY_EXISTS;

    if ( (fp = fopen(schema, "r")) == NULL)
        return DB_INVALID_ARG;

    /*Read the schema*/
    while (fgets(line, LINE_MAX, fp) != NULL)
    {
        strcat(sqlStr, line);

        /* execute if ; found*/
        if ( (strchr(line,';')) != NULL )
        {
            ret = sqlite3_exec( db->dbHandle, sqlStr, NULL, NULL, NULL );

            if ( ret != SQLITE_OK )
            {
                logError( db->logFile, "Error in BuildDatabase - "
                        "Can't create user table : %s",
                        sqlite3_errmsg( db->dbHandle ) );

                return DB_INTERNAL_ERROR;
            }
            sqlStr[0] = '\0';
        }

    }
    fclose(fp);
    return DB_SUCCESS;
}

DB_ERR
NewDatabaseADT( databaseADT *db, const char *dbFile, FILE *errLog )
{
    int ret;

    if ( db == NULL || dbFile == NULL || errLog == NULL )
        return DB_INVALID_ARG;

    if ( ( *db = ( databaseADT ) malloc( sizeof( databaseCDT ) ) ) == NULL)
        return DB_NO_MEMORY;

    ( *db )->logFile = errLog;
    ( *db )->dbFile = strdup(dbFile);

    /* Open the database file */
    ret = sqlite3_open( dbFile, &( ( *db )->dbHandle ) );

    if ( ret )
    {
        logError( (*db)->logFile, "Error in NewDatabaseADT - "
                        "Can't open database : %s",
                        sqlite3_errmsg( ( *db )->dbHandle ) );

        return DB_INTERNAL_ERROR;
    }

    return DB_SUCCESS;
}

void
FreeDatabaseADT( databaseADT db )
{
    if ( db == NULL )
        return;

    sqlite3_close(db->dbHandle);
    free(db->dbFile);
    free(db);
}

DB_ERR
DBaddUser(databaseADT db, const char *user, const char *password,
          const char *mail)
{
    sqlite3_stmt *statement;
    int ret;
    char *userN, *mailN;
    blobBindings binding;
    char *sqlSelect = "INSERT INTO users VALUES (NULL, '%s', ? , '%s')";

    if (db == NULL || user == NULL || password == NULL || mail == NULL)
        return DB_INVALID_ARG;

    /*TODO: In the next lines inside the if it should free
            malloced strings in this function */
    if ( ( userN = EscapeString( db, user ) ) == NULL )
        return DB_NO_MEMORY;

    if ( ( mailN = EscapeString( db, mail ) ) == NULL )
        return DB_NO_MEMORY;

    binding.param=1;
    binding.data= (void *)password;
    binding.size= 128;


    ret = QueryExecute(db, &statement, sqlSelect, 1, &binding, 2, userN, mailN);

    free(userN);
    free(mailN);

    switch (ret)
    {
        case SQLITE_DONE:
            sqlite3_finalize( statement );
            return DB_SUCCESS;

        case SQLITE_CONSTRAINT:
            sqlite3_finalize( statement );
            return DB_ALREADY_EXISTS;

        default:
            sqlite3_finalize(statement);
            return DB_INTERNAL_ERROR;
    }
}

DB_ERR
DBgetUserQueue(databaseADT db, queueADT queue)
{
    sqlite3_stmt *statement;
    int ret;
    user_t uq;
    char *sqlSelect = "SELECT user, password, email FROM users";

    if ( db == NULL || queue == NULL )
        return DB_INVALID_ARG;

    ret = QueryExecute( db, &statement, sqlSelect, 0, NULL, 0 );

    while ( ret == SQLITE_ROW )
    {
        strncpy(uq.name, (char *) sqlite3_column_text(statement, 0),
                USER_NAME_MAX_LEN);
        uq.name[USER_NAME_MAX_LEN-1] = 0;

        strncpy(uq.pass, (char *) sqlite3_column_text(statement, 1),
                USER_PASS_MAX_LEN);
        uq.mail[USER_PASS_MAX_LEN-1] = 0;

        strncpy(uq.mail, (char *) sqlite3_column_text(statement, 2),
                USER_MAIL_MAX_LEN);
        uq.mail[USER_MAIL_MAX_LEN-1] = 0;


        if ((enqueue(queue, &uq)) == 1)
            ret = sqlite3_step( statement );
        else
        {
            sqlite3_finalize( statement );
            return DB_INTERNAL_ERROR;
        }
    }

    sqlite3_finalize( statement );

    if ( ret != SQLITE_DONE )
        return DB_INTERNAL_ERROR;

    return DB_SUCCESS;
}

static int
QueryExecute( databaseADT db, sqlite3_stmt **statement, const char *sql,
                int bindingCount, blobBindings* bindings, int args, ... )
{

    char *query, *tail;
    int queryLen = 0;
    int i;
    va_list ap, apCopy;
    int retCode;

    /* Get the list of arguments */
    va_start( ap, args );
    va_copy( apCopy, ap );

    for ( i = 0; i < args; i++ )
        queryLen += strlen( va_arg( ap, char* ) );

    va_end( ap );

    /* Add the query length to args', and substract 2 for each arg (%s) + NULL */
    queryLen += strlen( sql ) - 2 * args + 1;

    /* Reserve space for query */
    if ( ( query = (char*) calloc( queryLen, sizeof( char ) ) ) == NULL )
    {
        logError( db->logFile, "Not enough memory building query in QueryExecute." );
        return DB_NO_MEMORY;
    }

    /* Build query according to received data */
    if ( vsprintf( query, sql, apCopy ) != queryLen - 1 )
    {
        va_end( apCopy );
        free( query );
        logError( db->logFile, "Query length doesn't match with expected value." );
        return DB_INVALID_ARG;
    }

    va_end( apCopy );

    /* Prepare for execution and retrieve values */
    PrepareSql(db, query, queryLen, statement, (const char**)&tail);


    /* If there are any bindings, do them! */
    if ( bindings != NULL && bindingCount > 0 )
    {
        for ( i = 0; i < bindingCount; i++ )
        {
            sqlite3_bind_blob( *statement, bindings[i].param,
                            bindings[i].data, bindings[i].size,
                                SQLITE_TRANSIENT );
        }
    }

//     retCode = sqlite3_step( *statement );
    retCode = StepSql(db, *statement);

    /* If an error occured, log it */
    if ( retCode != SQLITE_DONE && retCode != SQLITE_ROW )
            logError( db->logFile, "Error executing query: \"%s\""
                    " - The error message is: %s", query,
                    sqlite3_errmsg( db->dbHandle ) );

    free( query );
    return retCode;
}


int
PrepareSql(databaseADT db, char *SqlStr, int queryLen,
            sqlite3_stmt **statement, const char **tail)
{
    int rc;
    int n = 0;

    do
    {
        rc = sqlite3_prepare_v2( db->dbHandle, SqlStr, queryLen,
                                statement, tail);

        if( (rc == SQLITE_BUSY) || (rc == SQLITE_LOCKED) )
        {
            n++;
            usleep(SQLTM_TIME);
        }
    }while((n < SQLTM_COUNT) && ((rc == SQLITE_BUSY) || (rc == SQLITE_LOCKED)));

    if( rc != SQLITE_OK)
    {
        logError(db->logFile, "SqlPrepare-Error-H(%d): (%d) %s \n",
                db->dbHandle, rc, sqlite3_errmsg(db->dbHandle));
    }

    return rc;
}

   /******************************************************/
   /** StepSql:                                         **/
   /** This encapsulates sqlite step call to handle     **/
   /** timeout condition.                               **/
   /******************************************************/
int
StepSql(databaseADT db, sqlite3_stmt *statement)
{
    int rc, n = 0;

    do
    {
        rc = sqlite3_step(statement);

        if( rc == SQLITE_LOCKED )
        {
            /** Note: This will return SQLITE_LOCKED as well... **/
            rc = sqlite3_reset(statement);
            n++;
            usleep(SQLTM_TIME);
        }
        else
        {
            if( (rc == SQLITE_BUSY) )
            {
                usleep(SQLTM_TIME);
                n++;
            }
        }
    }while((n < SQLTM_COUNT) && ((rc == SQLITE_BUSY) || (rc == SQLITE_LOCKED)));

    if( n == SQLTM_COUNT )
    {
        logError(db->logFile, "SqlStep Timeout on handle: %d (rc = %d)\n",
                db->dbHandle, rc);
    }

    if( n > 2 )
    {
        logError(db->logFile, "SqlStep tries on handle %d: %d\n",
                db->dbHandle, n);
    }

    if( rc == SQLITE_MISUSE )
    {
        logError(db->logFile, "sqlite3_step missuse ?? on handle %d\n",
                db->dbHandle);
    }

    return rc;
}

/** This generates an exclusive transaction. Used to block all other **/
/** from writing operations.                                         **/
// int
// BeginTrans(databaseADT db, int handle)
// {
//     int rc;
//     sqlite3_stmt *bt_stmt = NULL;
// 
//     if( db == NULL )
//     {
//         logError(db->logFile, "Sql: BeginTrans: No DB connection!\n");
//         return 0;
//     }
// PrepareSql(databaseADT db, char *SqlStr, int queryLen,
//             sqlite3_stmt **statement, const char **tail)
//     if( !PrepareSql(db, "BEGIN EXCLUSIVE TRANSACTION;", &bt_stmt, , handle) )
//     {
//         EnterCriticalSection(&stderr_lock);
//         fprintf(stderr, "Begin Transaction error on handle: %d\n", handle);
//         LeaveCriticalSection(&stderr_lock);
// 
//         return 0;
//     }
// 
//     rc = StepSql(bt_stmt, handle);
// 
//     sqlite3_finalize(bt_stmt);
// 
//     if( rc != SQLITE_DONE )
//     {
//         EnterCriticalSection(&stderr_lock);
//         fprintf(stderr, "BeginTrans Timeout/Error on handle:  %d, Errorcode = %d \n", handle, rc);
//         LeaveCriticalSection(&stderr_lock);
//         return 0;
//     }
// 
//     return 1;
// }

   /** This ends the exclusive transaction...    **/
// int EndTrans(sqlite3 *DB, int handle)
// {
//     int         rc;
//     sqlite3_stmt    *bt_stmt;
// 
//     if( DB == NULL )
//     {
//         fprintf(stderr, "Sql: EndTrans: No DB connection!\n");
//         return(0);
//     }
// 
//     if( !PrepareSql(DB, &bt_stmt, "COMMIT;", handle) )
//     {
//         EnterCriticalSection(&stderr_lock);
//         fprintf(stderr, "EndTransaction prepare failed/timeout on handle %d\n", handle);
//         LeaveCriticalSection(&stderr_lock);
//         return(0);
//     }
// 
//     rc = StepSql(bt_stmt, handle);
// 
//     sqlite3_finalize(bt_stmt);
// 
//     if( rc != SQLITE_DONE )
//     {
//         EnterCriticalSection(&stderr_lock);
//         fprintf(stderr, "EndTrans Step Timeout on handle %d (code = %d) \n", handle, rc);
//         LeaveCriticalSection(&stderr_lock);
//         return(0);
//     }
// 
//     return(1);
// }

static char *EscapeString( databaseADT db, const char *str )
{

    char *outstr;
    int i, j = 0, need = 0;

    for ( i = 0; i < strlen( str ); i++ )
            if ( str[i] == '\'' || str[i] == '%' )
                    need++;

    if ((outstr = (char *)calloc(sizeof(char), strlen(str) + need + 1)) == NULL)
    {
            logError( db->logFile, "Not enough memory in EscapeString." );
            return NULL;
    }

    for ( i = 0; i < strlen( str ); i++ )
    {
            /* Prevent both, injection in the SQL, and
                     corruption when using sprintf */
            if ( str[i] == '\'' || str[i] == '%' )
                    outstr[j++] = str[i];

            outstr[j++] = str[i];
    }

    return outstr;
}
