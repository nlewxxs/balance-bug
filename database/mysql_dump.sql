CREATE table SessionList (
    `TimeStamp` TIMESTAMP NOT NULL,
    `BugName` char(100) NOT NULL,
    `SessionId` char(100) NOT NULL,
    PRIMARY KEY (`TimeStamp`, `BugName`, `SessionId`)
) ENGINE=InnoDB;