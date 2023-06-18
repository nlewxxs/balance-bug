package Api

import (
	"fmt"
	"log"
	"net/http"
	"strconv"
	"time"

	_ "github.com/go-sql-driver/mysql"

	"github.com/gin-gonic/gin"
	_ "github.com/lib/pq"

	"database/sql"

	"github.com/google/uuid"
)

type SessionListStruct struct {
	TimeStamp   string `json:"TimeStamp"`
	BugName     string `json:"BugName"`
	SessionId   string `json:"SessionId"`
	SessionName string `json:"SessionName"`
}

// CRUD: Create Read Update Delete API Format
// DISPLAY SESSION IDs
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
	BugNameNew := c.Query("BugName")
	SessionIdNew := uuid.New().String()
	SessionNameNew := c.Query("SessionName")

	//location, err := time.LoadLocation("Europe/London")

	// if err != nil {
	// 	c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to load time zone"})
	// 	return
	// }

	t := time.Now() //.In(location)
	TimeStampNew := t.Format("2006-01-02 15:04:05")

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

func CheckNewSession(c *gin.Context) {
	BugId := c.Query("BugId")
	timeDiff := c.Query("TimeDiff")

	TimeDiffExtracted, err := strconv.ParseFloat(timeDiff, 32)
	if err != nil {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "invalid TimeDiff format, please enter a valid float"})
		return
	}

	t := time.Now() //.In(location)
	CurrentTime := t.Format("2006-01-02 15:04:05")

	//location, err := time.LoadLocation("Europe/London")

	// if err != nil {
	// 	c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to load time zone"})
	// 	return
	// }

	// Validate entry
	if len(CurrentTime) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a SessionList.ConnTime"})
	} else if len(BugId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a SessionList.BugName"})
	} else {
		// Create todo item
		var SessionListNew SessionListStruct

		SessionListNew.TimeStamp = CurrentTime

		BugNameQuery := db.QueryRow("SELECT `BugName` FROM `BugInformation` WHERE `BugId`=?;", BugId)
		switch err := BugNameQuery.Scan(&SessionListNew.BugName); err {
		case sql.ErrNoRows:
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with BugId-BugName Translation"})
			return
		case nil:
			fmt.Println(SessionListNew.BugName)
		default:
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			return
		}

		SessionIdQuery := db.QueryRow("SELECT `SessionId` FROM testdb.SessionList WHERE TIMESTAMPDIFF(SECOND, TimeStamp, ?) < ? ORDER BY `TimeStamp` DESC LIMIT 1;", CurrentTime, TimeDiffExtracted)
		switch err := SessionIdQuery.Scan(&SessionListNew.SessionId); err {
		case sql.ErrNoRows:
			log.Println("Did not find matching SessionList")

			// Return success response
			c.Header("Access-Control-Allow-Origin", "*")
			c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
			c.JSON(http.StatusOK, "0")
			return
		case nil:
			log.Println("Found matching SessionList entry")

			// Return success response
			c.Header("Access-Control-Allow-Origin", "*")
			c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
			c.JSON(http.StatusOK, "1")
		default:
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			return
		}
	}
}

func PingSession(c *gin.Context) {
	var SessionListNew SessionListStruct

	SessionListNew.SessionId = c.Query("SessionId")

	//location, err := time.LoadLocation("Europe/London")

	// if err != nil {
	// 	c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to load time zone"})
	// 	return
	// }

	t := time.Now() //.In(location)
	SessionListNew.TimeStamp = t.Format("2006-01-02 15:04:05")

	// Validate entry
	if len(SessionListNew.SessionId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a SessionId"})
	} else {
		// Insert item to DB
		_, err := db.Query("UPDATE testdb.SessionList SET `TimeStamp`=? WHERE SessionId=?;", SessionListNew.TimeStamp, SessionListNew.SessionId)
		if err != nil {
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			return
		}

		// Log message
		log.Println("updated SessionList entry", SessionListNew)

		// Return success response
		c.Header("Access-Control-Allow-Origin", "*")
		c.Header("Access-Control-Allow-Headers", "Content-Type, access-control-allow-origin, access-control-allow-headers")
		c.JSON(http.StatusOK, &SessionListNew)
	}
}
