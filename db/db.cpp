#include <iostream>
#include <db.h>
#include "util/fileutil.h"
#include "util/logdef.h"
#include "db.h"

DBManager::DBManager() {}
DBManager::~DBManager()
{
    if (_env) {
        //mdb_env_close(_env);
        _env = nullptr;
    }
}

int DBManager::init(const std::string &basepath)
{
    _basepath = basepath;
    if (File::IsDirectory(basepath)) {
        return 0;
    }
    else if (File::IsFile(basepath)) {
        LOGE("directory not exists");
    }
    else {
        if (!File::CreateDir(basepath)) {
            LOGE("create directory failed, %s", strerror(errno));
            return -1;
        }
    }
    return 0;
}

int DBManager::open(const DBOption &opt)
{

    std::string dbpath = _basepath + "/" + opt.dbname + ".db";

    LOGI("bdb version:%s\n%s\ndbpath:%s", db_version(0, 0, 0), db_full_version(0, 0, 0, 0, 0), dbpath.c_str());

    DB *dbp;
    DBC *dbcp;
    DBT key, val;
    DB_BTREE_STAT *statp;
    FILE *fp;
    db_recno_t recno;
    size_t len;
    int cnt, ret;
    char *p, *t, buf[1024], rbuf[1024];
    const char *progname = "ex_btrec"; /* Program name. */

    DB_ENV *dbenv;
    if ((ret = db_env_create(&dbenv, 0)) != 0) {
                fprintf(stderr, "%s: %s\n", progname, db_strerror(ret));
                return (1);
        }
        dbenv->set_errfile(dbenv, stderr);
        dbenv->set_errpfx(dbenv, progname);
        if ((ret = dbenv->set_cachesize(dbenv, 0, 64 * 1024, 0)) != 0) {
                dbenv->err(dbenv, ret, "set_cachesize");
                dbenv->close(dbenv, 0);
                return (1);
        }

        (void)dbenv->set_data_dir(dbenv, _basepath.c_str());

        if ((ret = dbenv->open(dbenv, _basepath.c_str(), DB_CREATE | DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN, 0644)) != 0) {
                dbenv->err(dbenv, ret, "environment open: %s", _basepath.c_str());
                dbenv->close(dbenv, 0);
                return (1);
        }

    /* Create and initialize database object, open the database. */
    if ((ret = db_create(&dbp, nullptr, 0)) != 0) {
        fprintf(stderr, "%s: db_create: %s\n", progname, db_strerror(ret));
        return (1);
    }
    dbp->set_errfile(dbp, stderr);
    dbp->set_errpfx(dbp, progname); /* 1K page sizes. */
    if ((ret = dbp->set_pagesize(dbp, 1024)) != 0) {
        dbp->err(dbp, ret, "set_pagesize");
        return (1);
    } /* Record numbers. */
    if ((ret = dbp->set_flags(dbp, DB_RECNUM)) != 0) {
        dbp->err(dbp, ret, "set_flags: DB_RECNUM");
        return (1);
    }
    if ((ret = dbp->open(dbp, NULL, opt.dbname.c_str(), NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
        dbp->err(dbp, ret, "open: %s", opt.dbname.c_str());
        return (1);
    }

    /*
     * Insert records into the database, where the key is the word
     * preceded by its record number, and the data is the same, but
     * in reverse order.
     */
    memset(&key, 0, sizeof(DBT));
    memset(&val, 0, sizeof(DBT));
    for (cnt = 1; cnt <= 100; ++cnt) {
        std::string k = std::string("k-") + std::to_string(cnt);
        std::string v = std::string("v-") + std::to_string(cnt * 32);

        key.size = k.size();
        key.data = (void *)(k.data());
        val.size = v.size();
        val.data = (void *)(v.data());

        if ((ret = dbp->put(dbp, NULL, &key, &val, DB_NOOVERWRITE)) != 0) {
            dbp->err(dbp, ret, "DB->put");
            if (ret != DB_KEYEXIST) goto err1;
        }
    }

    /* Print out the number of records in the database. */
    if ((ret = dbp->stat(dbp, NULL, &statp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->stat");
        goto err1;
    }
    printf("%s: database contains %lu records\n", progname, (u_long)statp->bt_ndata);
    free(statp);

    /* Acquire a cursor for the database. */
    if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
        dbp->err(dbp, ret, "DB->cursor");
        goto err1;
    }

    /*
     * Prompt the user for a record number, then retrieve and display
     * that record.
     */
    for (;;) {
        /* Get a record number. */
        // printf("recno #> ");
        recno = 10;

        /*
         * Reset the key each time, the dbp->get() routine returns
         * the key and data pair, not just the key!
         */
        //	key.data = &recno;
        //		key.size = sizeof(recno);
        //		if ((ret = dbcp->get(dbcp, &key, &val, DB_SET_RECNO)) != 0)
        //			goto get_err;

        //	/* Display the key and data. */
        //{
        //       std::string k((const char*)(key.data), key.size);
        //       std::string v((const char*)(val.data), val.size);
        //       LOGI("key: '%s', value: '%s'", k.c_str(), v.c_str());
        //}
        /* Move the cursor a record forward. */
        if ((ret = dbcp->get(dbcp, &key, &val, DB_NEXT)) != 0) {
            dbp->err(dbp, ret, "DBcursor->get");
            // if (ret != DB_NOTFOUND && ret != DB_KEYEMPTY)
            goto err2;
        }

        /* Display the key and data. */
        {
            std::string k((const char *)(key.data), key.size);
            std::string v((const char *)(val.data), val.size);
            LOGI("key: '%s', value: '%s'", k.c_str(), v.c_str());
        }

        /*
         * Retrieve the record number for the following record into
         * local memory.
         */
        val.data = &recno;
        val.size = sizeof(recno);
        val.ulen = sizeof(recno);
        val.flags |= DB_DBT_USERMEM;
        //	if ((ret = dbcp->get(dbcp, &key, &val, DB_GET_RECNO)) != 0) {
        // get_err:		dbp->err(dbp, ret, "DBcursor->get");
        //			if (ret != DB_NOTFOUND && ret != DB_KEYEMPTY)
        //				goto err2;
        //		} else
        //			printf("retrieved recno: %lu\n", (u_long)recno);

        /* Reset the data DBT. */
        memset(&val, 0, sizeof(val));
    }

    if ((ret = dbcp->close(dbcp)) != 0) {
        dbp->err(dbp, ret, "DBcursor->close");
        goto err1;
    }
    if ((ret = dbp->close(dbp, 0)) != 0) {
        fprintf(stderr, "%s: DB->close: %s\n", progname, db_strerror(ret));
        return (1);
    }

    return (0);

err2:
    (void)dbcp->close(dbcp);
err1:
    (void)dbp->close(dbp, 0);
    return (ret);

    //   return -1;
}

int DBManager::open(const std::string &dbname, bool readonly)
{
    DBOption opt;
    opt.dbname   = dbname;
    opt.readonly = readonly;
    return open(opt);
}

int DBManager::close(const std::string &dbname) { return -1; }

int DBManager::close_all() { return -1; }

// TableReader DBManager::get_table_reader(const std::string &dbname, const std::string &tblname){}
// TableWriter DBManager::get_table_writer(const std::string &dbname, const std::string &tblname){}
