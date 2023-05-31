CREATE table SessionList (
    `TimeStamp`   TIMESTAMP NOT NULL,
    `BugName`     char(100) NOT NULL,
    `SessionId`   char(100) NOT NULL,
    `SessionName` char(100) NOT NULL,
    PRIMARY KEY (`SessionId`)
) ENGINE=InnoDB;

CREATE table BugInformation (
    `BugId`    char(100) NOT NULL,
    `BugName`  char(100) NOT NULL,
    `LastSeen` TIMESTAMP NOT NULL,
    PRIMARY KEY (`BugId`)
) ENGINE=InnoDB;
