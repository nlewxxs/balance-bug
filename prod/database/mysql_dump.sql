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

CREATE table BeaconCharge (
    `SessionId`    char(100) NOT NULL,
    `ChargeStatus`  char(100) NOT NULL,
    PRIMARY KEY (`SessionId`)
) ENGINE=InnoDB;

CREATE table BeaconOn (
    `SessionId`    char(100) NOT NULL,
    `On`  char(100) NOT NULL,
    PRIMARY KEY (`SessionId`)
) ENGINE=InnoDB;

CREATE table BeaconConnected (
    `SessionId`    char(100) NOT NULL,
    `LastSeen` TIMESTAMP NOT NULL,
    PRIMARY KEY (`SessionId`)
) ENGINE=InnoDB;
