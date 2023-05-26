CREATE table SessionList (
    `TimeStamp` TIMESTAMP,
    `BugName` char(100),
    `SessionId` char(100),
    PRIMARY KEY (`TimeStamp`, `BugName`, `SessionId`)
) ENGINE=InnoDB;