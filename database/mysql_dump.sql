CREATE table BotKey (
    ConnTime TIMESTAMP NOT NULL,
    SessionKey char(100) NOT NULL,
    BugId char(100) NOT NULL,
    PRIMARY KEY (ConnTime, SessionKey, BugId)
) ENGINE=InnoDB;