#ifndef __DATABASE_ADT_H__
#define __DATABASE_ADT_H__

#include "../queue/queueADT.h"

#define FALSE   0
#define TRUE    !FALSE

/** Timeout values on busy or lock conditions   **/
/** if you observe lock errors you might try to **/
/** increase the values.     
/** SQLTM_COUNT*SQLTM_TIME micro seg timeout **/
#define SQLTM_COUNT       500
#define SQLTM_TIME        1000


typedef struct databaseCDT *databaseADT;

typedef enum { DB_SUCCESS = 0, DB_INVALID_ARG, DB_NO_MATCH, DB_NO_MEMORY,
            DB_INTERNAL_ERROR, DB_ACCESS_DENIED, DB_ALREADY_EXISTS } DB_ERR;


/**
 * Creates a new database instance.
 *
 * @param[out]  db      Pointer to the newly created database instance.
 * @param[in]   dbFile  Path to the database file.
 * @param[in]   errLog  The stream to which to output error logs.
 *                      If NULL error log is disabled.
 *
 * @return      DB_SUCCESS if the operation succeded, an appropiate error
 *              code otherwise.
*/
DB_ERR NewDatabaseADT( databaseADT *db, const char *dbFile, FILE *errLog );

/**
 * Destroys a database instance.
 *
 * @param[in]   db      Database instance to be destroyed.
*/
void FreeDatabaseADT( databaseADT db );

/**
 * Creates all database tables if they don't exist already.
 *
 * @param[in]   db      The database into which to attempt to
 *                      create the tables.
 * @param[in]   schema  Path to the db schema.
 *
 * @return      DB_SUCCESS if the operation succeded, an appropiate error
 *              code otherwise.
*/
DB_ERR DBBuildDatabase( databaseADT db, const char *schema );

/**
 * Adds a user to the db.
 *
 * @param[in]   db   Pointer to the newly created database instance.
 * @param[in]   user char *.
 * @param[in]   mail char *.
 * @param[in]   pass char *.
 *
 * @return      DB_SUCCESS if the operation succeded, an appropiate error
 *              code otherwise.
*/
DB_ERR DBaddUser(databaseADT db, const char *user, const char *password,
        const char *mail);

/**
 * Gets the user list.
 *
 * @param[in]   db          Pointer to the newly created database instance.
 * @param[out]  queue       QueueADT.
 *
 * @return      DB_SUCCESS if the operation succeded, an appropiate error
 *              code otherwise.
*/
DB_ERR DBgetUserQueue(databaseADT db, queueADT queue);

#endif
