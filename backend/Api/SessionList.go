package Api

//imports
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

//structs
type SessionListStruct struct {
	TimeStamp   string `json:"TimeStamp"`
	BugName     string `json:"BugName"`
	SessionId   string `json:"SessionId"`
	SessionName string `json:"SessionName"`
}

//Display Session Ids
func DisplaySessionList(c *gin.Context) {
	rows, err := db.Query("SELECT * FROM testdb.SessionList")
	if err != nil {
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
	}

	//make sessionlist for output
	SessionLists := make([]SessionListStruct, 0)

	//process the rows
	if rows != nil {
		defer rows.Close()
		for rows.Next() {
			//process each row individually
			SessionListRow := SessionListStruct{}
			if err := rows.Scan(&SessionListRow.TimeStamp, &SessionListRow.BugName, &SessionListRow.SessionId, &SessionListRow.SessionName); err != nil {
				fmt.Println(err.Error())
				c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
				break
			}
			SessionLists = append(SessionLists, SessionListRow)
		}
	}

	//return JSON
	c.Header("Access-Control-Allow-Origin", "*")
	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
	c.JSON(http.StatusOK, &SessionLists)
}

//add session
func AddSession(c *gin.Context) {
	//input queries
	BugNameNew := c.Query("BugName")
	SessionIdNew := uuid.New().String()
	SessionNameNew := c.Query("SessionName")

	//create timestamp
	t := time.Now() //.In(location)
	TimeStampNew := t.Format("2006-01-02 15:04:05")

	//check input query exists
	if len(TimeStampNew) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a SessionList.ConnTime"})
	} else if len(BugNameNew) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a SessionList.BugName"})
	} else if len(SessionIdNew) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a SessionList.SessionIdNew"})
	} else if len(SessionNameNew) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a SessionList.SessionName"})
	} else {
		var SessionListNew SessionListStruct

		SessionListNew.TimeStamp = TimeStampNew
		SessionListNew.BugName = BugNameNew
		SessionListNew.SessionId = SessionIdNew
		SessionListNew.SessionName = SessionNameNew

		//query db to insert
		req, err := db.Query("INSERT INTO testdb.SessionList(`Timestamp`, `BugName`, `SessionId`, `SessionName`) VALUES(?, ?, ?, ?);", SessionListNew.TimeStamp, SessionListNew.BugName, SessionListNew.SessionId, SessionListNew.SessionName)
		if err != nil {
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
		}
		req.Close()

		//log
		log.Println("created SessionList entry", SessionListNew)

		//return JSON
		c.Header("Access-Control-Allow-Origin", "*")
		c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
		c.JSON(http.StatusCreated, &SessionListNew)
	}
}

//CheckNewSession
func CheckNewSession(c *gin.Context) {
	//input queries
	BugId := c.Query("BugId")
	timeDiff := c.Query("TimeDiff")

	//timedifference to respond to
	TimeDiffExtracted, err := strconv.ParseFloat(timeDiff, 32)
	if err != nil {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "invalid TimeDiff format, please enter a valid float"})
		return
	}

	//create current timestamp
	t := time.Now() //.In(location)
	CurrentTime := t.Format("2006-01-02 15:04:05")

	//check entry is valid
	if len(CurrentTime) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a SessionList.ConnTime"})
	} else if len(BugId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a SessionList.BugName"})
	} else {
		var SessionListNew SessionListStruct

		SessionListNew.TimeStamp = CurrentTime

		//query db for BugId-BugName Translation
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

		//Check if matching SessionId exists with timestamp difference specified
		SessionIdQuery := db.QueryRow("SELECT `SessionId` FROM testdb.SessionList WHERE TIMESTAMPDIFF(SECOND, TimeStamp, ?) < ? ORDER BY `TimeStamp` DESC LIMIT 1;", CurrentTime, TimeDiffExtracted)
		switch err := SessionIdQuery.Scan(&SessionListNew.SessionId); err {
		case sql.ErrNoRows:
			//no matching SessionList
			log.Println("Did not find matching SessionList")

			//respond "0"
			c.Header("Access-Control-Allow-Origin", "*")
			c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
			c.JSON(http.StatusOK, "0")
			return
		case nil:
			//Create SessionList entry
			log.Println("Found matching SessionList entry")

			//respond "1"
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

//Ping Session
func PingSession(c *gin.Context) {
	var SessionListNew SessionListStruct

	//input querys
	SessionListNew.SessionId = c.Query("SessionId")

	//create appropriate timestamp
	t := time.Now() //.In(location)
	SessionListNew.TimeStamp = t.Format("2006-01-02 15:04:05")

	//check entry is correct
	if len(SessionListNew.SessionId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a SessionId"})
	} else {
		//query to update the database
		req, err := db.Query("UPDATE testdb.SessionList SET `TimeStamp`=? WHERE SessionId=?;", SessionListNew.TimeStamp, SessionListNew.SessionId)
		if err != nil {
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			return
		}
		req.Close()

		//log
		log.Println("updated SessionList entry", SessionListNew)

		//return JSON
		c.Header("Access-Control-Allow-Origin", "*")
		c.Header("Access-Control-Allow-Headers", "Content-Type, access-control-allow-origin, access-control-allow-headers")
		c.JSON(http.StatusOK, &SessionListNew)
	}
}
