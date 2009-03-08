CREATE TABLE users
(
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    user        VARCHAR(50),
    password    BLOB,
    email       VARCHAR(50),
    UNIQUE(user)
);
