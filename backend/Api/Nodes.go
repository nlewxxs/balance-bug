package Api

import (
	"fmt"
	"net/http"

	_ "github.com/go-sql-driver/mysql"

	"database/sql"

	"github.com/gin-gonic/gin"
	_ "github.com/lib/pq"
)

type NodeStruct struct {
	NodeId string `json:"NodeId"`
	XCoord string `json:"XCoord"`
	YCoord string `json:"YCoord"`
}

type ProcessedNodeStruct struct {
	Id    string `json:"id"`
	Label string `json:"label"`
	X     string `json:"x"`
	Y     string `json:"y"`
}

// Create the table, this is REQUIRED before reading
func CreateNodeTable(c *gin.Context) {
	BugId := c.Query("BugId")

	if len(BugId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "enter a BugId"})
		return
	}

	var BugName string
	var SessionId string

	BugNameQuery := db.QueryRow("SELECT `BugName` FROM testdb.BugInformation WHERE `BugId`=?;", BugId)
	switch err := BugNameQuery.Scan(&BugName); err {
	case sql.ErrNoRows:
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with BugId-BugName Translation"})
		return
	case nil:
		fmt.Println(BugName)
	default:
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
		return
	}

	SessionIdQuery := db.QueryRow("SELECT `SessionId` FROM testdb.SessionList ORDER BY `TimeStamp` DESC LIMIT 1;")
	switch err := SessionIdQuery.Scan(&SessionId); err {
	case sql.ErrNoRows:
		c.JSON(http.StatusInternalServerError, gin.H{"message": "No Session Exists"})
		return
	case nil:
		fmt.Println(SessionId)
	default:
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
		return
	}

	SqlCommand := fmt.Sprintf("CREATE TABLE IF NOT EXISTS `testdb.%s_nodes` (`NodeId` char(100) NOT NULL, `XCoord` char(100) NOT NULL, `YCoord` char(100) NOT NULL, PRIMARY KEY (`NodeId`, `XCoord`, `YCoord`)) ENGINE=InnoDB;", SessionId)

	_, err := db.Exec(SqlCommand)
	if err != nil {
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "could not create table"})
		return
	}
	// Return JSON object of all rows
	c.Header("Access-Control-Allow-Origin", "*")
	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
	c.JSON(http.StatusCreated, gin.H{"message": "successfully created new table"})
}

// CRUD: Create Read Update Delete API Format
// DISPLAY SESSION IDs
func DisplayAllNodes(c *gin.Context) {
	SessionId := c.Query("SessionId")

	if len(SessionId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a valid SessionId!"})
		return
	}

	SqlQuery := fmt.Sprintf("SELECT * FROM `testdb.%s_nodes`", SessionId)
	rows, err := db.Query(SqlQuery)
	if err != nil {
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB, maybe the appropriate table hasn't been created yet"})
		return
	}

	// Get all rows and add into SessionListStructs
	NodeLists := make([]ProcessedNodeStruct, 0)

	if rows != nil {
		defer rows.Close()
		for rows.Next() {
			// Individual row processing
			NodeListRow := ProcessedNodeStruct{}
			NodeListRow.Label = ""
			if err := rows.Scan(&NodeListRow.Id, &NodeListRow.X, &NodeListRow.Y); err != nil {
				fmt.Println(err.Error())
				c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
				break
			}

			NodeLists = append(NodeLists, NodeListRow)
		}
	} else {
		NodeListRow := ProcessedNodeStruct{}
		NodeListRow.Id = "start"
		NodeListRow.Label = ""
		NodeListRow.X = "0"
		NodeListRow.Y = "0"

		NodeLists = append(NodeLists, NodeListRow)
	}

	// Return JSON object of all rows
	c.Header("Access-Control-Allow-Origin", "*")
	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
	c.JSON(http.StatusOK, &NodeLists)
}

func AddNode(c *gin.Context) {
	var NodeNew NodeStruct

	BugId := c.Query("BugId")
	NodeNew.NodeId = c.Query("NodeId")
	NodeNew.XCoord = c.Query("XCoord")
	NodeNew.YCoord = c.Query("YCoord")

	// Validate entry
	if len(NodeNew.NodeId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a NodeId"})
	} else if len(NodeNew.XCoord) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a XCoord"})
	} else if len(NodeNew.YCoord) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a YCoord"})
	} else if len(BugId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a BugId"})
	} else {
		// Insert item to DB
		var BugName string
		var SessionId string

		BugNameQuery := db.QueryRow("SELECT `BugName` FROM testdb.BugInformation WHERE `BugId`=?;", BugId)
		switch err := BugNameQuery.Scan(&BugName); err {
		case sql.ErrNoRows:
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with BugId-BugName Translation"})
			return
		case nil:
			fmt.Println(BugName)
		default:
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			return
		}

		SessionIdQuery := db.QueryRow("SELECT `SessionId` FROM testdb.SessionList ORDER BY `TimeStamp` DESC LIMIT 1;")
		switch err := SessionIdQuery.Scan(&SessionId); err {
		case sql.ErrNoRows:
			c.JSON(http.StatusInternalServerError, gin.H{"message": "No Session Exists"})
			return
		case nil:
			fmt.Println(SessionId)
		default:
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			return
		}
		SqlCommand := fmt.Sprintf("INSERT INTO `testdb.%s_nodes` (`NodeId`, `XCoord`, `YCoord`) VALUES(?,?,?);", SessionId)

		req, err := db.Query(SqlCommand, NodeNew.NodeId, NodeNew.XCoord, NodeNew.YCoord)
		if err != nil {
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			return
		}
		req.Close()

		// Log message
		fmt.Println("created SessionList entry", NodeNew)

		// Return success response
		c.Header("Access-Control-Allow-Origin", "*")
		c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
		c.JSON(http.StatusCreated, &NodeNew)
	}
}
