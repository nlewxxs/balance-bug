package api

import (
	"fmt"
	"net/http"
	"strings"

	_ "github.com/go-sql-driver/mysql"

	"github.com/gin-gonic/gin"
	_ "github.com/lib/pq"

)

type NodeStruct struct {
	NodeId    	  		string  `json:"NodeId"`
	XCoord      		string  `json:"XCoord"`
	YCoord 				string  `json:"YCoord"`
}

//Create the table, this is REQUIRED before reading

func CreateNodeTable (c *gin.Context) {
	SessionId := c.Query("SessionId")

	SqlCommand := fmt.Sprintf("CREATE TABLE IF NOT EXISTS testdb.%s_nodes (`NodeId` char(100) NOT NULL, `XCoord` char(100) NOT NULL, `YCoord` char(100) NOT NULL, PRIMARY KEY (`NodeId`, `XCoord`, `YCoord`)) ENGINE=InnoDB;", SessionId)
	_, err := db.Exec(SqlCommand)
    if err != nil {
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "could not create table"})
    }
	// Return JSON object of all rows
	c.Header("Access-Control-Allow-Origin", "*")
	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
	c.JSON(http.StatusOK, gin.H{"message": "successfully created new table"})
}


// CRUD: Create Read Update Delete API Format
//DISPLAY SESSION IDs
func DisplayAllNodes(c *gin.Context) {
	SessionId := c.Query("SessionId")

	SqlQuery := fmt.Sprintf("SELECT * FROM testdb.%s_nodes", SessionId)
	rows, err := db.Query(SqlQuery)
	if err != nil {
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB, maybe the appropriate table hasn't been created yet"})
	}

	// Get all rows and add into SessionListStructs
	NodeLists := make([]NodeStruct, 0)

	if rows != nil {
		defer rows.Close()
		for rows.Next() {
			// Individual row processing
			NodeListRow := NodeStruct{}
			if err := rows.Scan(&NodeListRow.NodeId, &NodeListRow.XCoord, &NodeListRow.YCoord); err != nil {
				fmt.Println(err.Error())
				c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			}
			NodeListRow.NodeId = strings.TrimSpace(NodeListRow.NodeId)
			NodeLists = append(NodeLists, NodeListRow)
		}
	}

	// Return JSON object of all rows
	c.Header("Access-Control-Allow-Origin", "*")
	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
	c.JSON(http.StatusOK, &NodeLists)
}

// func CreateSession(c *gin.Context) {
// 	TimeStampNew := c.Query("TimeStamp")
// 	BugNameNew := c.Query("BugName")
// 	SessionIdNew := uuid.New().String()

// 	// Validate entry
// 	if len(TimeStampNew) == 0 {
// 		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a SessionList.ConnTime"})
// 	} else if len(BugNameNew) == 0 {
// 		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a SessionList.bugname"})
// 	} else if len(SessionIdNew) == 0 {
// 		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a SessionList.SessionIdNew"})
// 	} else {
// 		// Create todo item
// 		var SessionListNew SessionListStruct

// 		SessionListNew.TimeStamp = TimeStampNew
// 		SessionListNew.BugName = BugNameNew
// 		SessionListNew.SessionId = SessionIdNew

// 		// Insert item to DB
// 		_, err := db.Query("INSERT INTO testdb.SessionList(`Timestamp`, `BugName`, `SessionId`) VALUES(?, ?, ?);", SessionListNew.TimeStamp, SessionListNew.BugName, SessionListNew.SessionId)
// 		if err != nil {
// 			fmt.Println(err.Error())
// 			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
// 		}

// 		// Log message
// 		log.Println("created SessionList entry", SessionListNew)

// 		// Return success response
// 		c.Header("Access-Control-Allow-Origin", "*")
// 		c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
// 		c.JSON(http.StatusCreated, &SessionListNew)
// 	}
// }
