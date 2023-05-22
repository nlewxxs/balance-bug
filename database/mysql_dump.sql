CREATE table BotKey (
    ConnTime TIMESTAMP NOT NULL,
    SessionKey char(100) NOT NULL,
    BugId char(100) NOT NULL,
    PRIMARY KEY (ConnTime, SessionKey, BugId)
) ENGINE=InnoDB;


INSERT INTO `testapp`.`BotKey` (`ConnTime`, `SessionKey`, `BugId`) VALUES (CURRENT_TIMESTAMP, 'test', 'test');
