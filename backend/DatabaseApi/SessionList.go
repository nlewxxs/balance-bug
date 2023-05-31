package DatabaseApi

import (
	"fmt"
	"log"
	"net/http"

	_ "github.com/go-sql-driver/mysql"

	"github.com/gin-gonic/gin"
	_ "github.com/lib/pq"

	"github.com/google/uuid"
)

type SessionListStruct struct {
	TimeStamp      	string  `json:"TimeStamp"`
	BugName         string  `json:"BugName"`
	SessionId 	string  `json:"SessionId"`
	SessionName     string  `json:"SessionName"`
}

// CRUD: Create Read Update Delete API Format
//DISPLAY SESSION IDs
func DisplaySessionList(c *gin.Context) {
	rows, err := db.Query("SELECT * FROM testdb.SessionList")
	if err != nil {
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
	}

	// Get all rows and add into SessionListStructs
	SessionLists := make([]SessionListStruct, 0)

	if rows != nil {
		defer rows.Close()
		for rows.Next() {
			// Individual row processing
			SessionListRow := SessionListStruct{}
			if err := rows.Scan(&SessionListRow.TimeStamp, &SessionListRow.BugName, &SessionListRow.SessionId, &SessionListRow.SessionName); err != nil {
				fmt.Println(err.Error())
				c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
				break
			}
			SessionLists = append(SessionLists, SessionListRow)
		}
	}

	// Return JSON object of all rows
	c.Header("Access-Control-Allow-Origin", "*")
	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
	c.JSON(http.StatusOK, &SessionLists)
}

func AddSession(c *gin.Context) {
	TimeStampNew := c.Query("TimeStamp")
	BugNameNew := c.Query("BugName")
	SessionIdNew := uuid.New().String()
	SessionNameNew := c.Query("SessionName")

	// Validate entry
	if len(TimeStampNew) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a SessionList.ConnTime"})
	} else if len(BugNameNew) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a SessionList.BugName"})
	} else if len(SessionIdNew) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a SessionList.SessionIdNew"})
	} else if len(SessionNameNew) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a SessionList.SessionName"})
	} else {
		// Create todo item
		var SessionListNew SessionListStruct

		SessionListNew.TimeStamp = TimeStampNew
		SessionListNew.BugName = BugNameNew
		SessionListNew.SessionId = SessionIdNew
		SessionListNew.SessionName = SessionNameNew

		// Insert item to DB
		_, err := db.Query("INSERT INTO testdb.SessionList(`Timestamp`, `BugName`, `SessionId`, `SessionName`) VALUES(?, ?, ?, ?);", SessionListNew.TimeStamp, SessionListNew.BugName, SessionListNew.SessionId, SessionListNew.SessionName)
		if err != nil {
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
		}

		// Log message
		log.Println("created SessionList entry", SessionListNew)

		// Return success response
		c.Header("Access-Control-Allow-Origin", "*")
		c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
		c.JSON(http.StatusCreated, &SessionListNew)
	}
}
