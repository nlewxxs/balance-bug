<<<<<<< HEAD
CREATE table BotKey (
    ConnTime TIMESTAMP NOT NULL,
    SessionKey char(100) NOT NULL,
    BugId char(100) NOT NULL,
    PRIMARY KEY (ConnTime, SessionKey, BugId)
) ENGINE=InnoDB;


INSERT INTO `testapp`.`BotKey` (`ConnTime`, `SessionKey`, `BugId`) VALUES (CURRENT_TIMESTAMP, 'test', 'test');
=======
CREATE table BotKey (
    ConnTime TIMESTAMP NOT NULL,
    SessionKey char(100) NOT NULL,
    BugId char(100) NOT NULL,
    PRIMARY KEY (ConnTime, SessionKey, BugId)
) ENGINE=InnoDB;
>>>>>>> 09a6bf6768097befca043e3c6a6ac6c6f165db27
