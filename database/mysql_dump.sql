<<<<<<< HEAD
CREATE table BotKey (
    ConnTime TIMESTAMP NOT NULL,
    SessionKey char(100) NOT NULL,
    BugId char(100) NOT NULL,
    PRIMARY KEY (ConnTime, SessionKey, BugId)
) ENGINE=InnoDB;
=======
--USE docker-entrypoint-initdb.d;
CREATE table BotKey (
    ConnTime TIMESTAMP NOT NULL,
    SessionKey char(100) NOT NULL,
    BugId char(100) NOT NULL,
    PRIMARY KEY (ConnTime, SessionKey, BugId)
);
>>>>>>> b49d1e4ef260c43a6960a1dda7a60156ca7f1dd8
