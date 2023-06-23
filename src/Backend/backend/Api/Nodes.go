package Api

//imports
import (
	"fmt"
	"net/http"
	"time"

	_ "github.com/go-sql-driver/mysql"

	"database/sql"

	"github.com/gin-gonic/gin"
	_ "github.com/lib/pq"
)

//structs
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

//Create the table, this is REQUIRED before reading
func CreateNodeTable(c *gin.Context) {
	//input Query
	BugId := c.Query("BugId")

	//check if length of input query > 0 (if exists)
	if len(BugId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "enter a BugId"})
		return
	}

	//variables
	var BugName string
	var SessionId string

	//query db for BugId-BugName translation
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

	//query db for latest sessionid
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

	//create sql command to create the table
	SqlCommand := fmt.Sprintf("CREATE TABLE IF NOT EXISTS `testdb.%s_nodes` (`NodeId` char(100) NOT NULL, `XCoord` char(100) NOT NULL, `YCoord` char(100) NOT NULL, PRIMARY KEY (`NodeId`, `XCoord`, `YCoord`)) ENGINE=InnoDB;", SessionId)

	//execture the sql command
	_, err := db.Exec(SqlCommand)
	if err != nil {
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "could not create table"})
		return
	}

	//wait for init entry to be created
	duration := time.Second
 	time.Sleep(duration)

	//insert initial start node at (0,0) with id 0
	SqlCommand2 := fmt.Sprintf("INSERT INTO `testdb.%s_nodes` (`NodeId`, `XCoord`, `YCoord`) VALUES(0,0,0);", SessionId)

	//execute this command
	_, err2 := db.Exec(SqlCommand2)
	if err2 != nil {
		fmt.Println(err2.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB1"})
		return
	}

	// Return JSON
	c.Header("Access-Control-Allow-Origin", "*")
	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
	c.JSON(http.StatusCreated, gin.H{"message": "successfully created new table"})
}

//Display SessionIds
func DisplayAllNodes(c *gin.Context) {
	//input query
	SessionId := c.Query("SessionId")

	//check if query exists
	if len(SessionId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a valid SessionId!"})
		return
	}

	//query to fetch all inputs
	SqlQuery := fmt.Sprintf("SELECT * FROM `testdb.%s_nodes`", SessionId)
	rows, err := db.Query(SqlQuery)
	if err != nil {
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB, maybe the appropriate table hasn't been created yet"})
		return
	}

	//process the nodes into the format that the front-end wants to draw the graph
	NodeLists := make([]ProcessedNodeStruct, 0)

	//go through each row and process
	if rows != nil {
		defer rows.Close()
		for rows.Next() {
			//row processing
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
		NodeListRow.Id = "0"
		NodeListRow.Label = ""
		NodeListRow.X = "0"
		NodeListRow.Y = "0"

		NodeLists = append(NodeLists, NodeListRow)
	}

	// Return JSON object
	c.Header("Access-Control-Allow-Origin", "*")
	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
	c.JSON(http.StatusOK, &NodeLists)
}

//Add Node
func AddNode(c *gin.Context) {
	//variable
	var NodeNew NodeStruct

	//input queries
	BugId := c.Query("BugId")
	NodeNew.NodeId = c.Query("NodeId")
	NodeNew.XCoord = c.Query("XCoord")
	NodeNew.YCoord = c.Query("YCoord")

	//check input queries exist in request
	if len(NodeNew.NodeId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a NodeId"})
	} else if len(NodeNew.XCoord) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a XCoord"})
	} else if len(NodeNew.YCoord) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a YCoord"})
	} else if len(BugId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a BugId"})
	} else {
		var BugName string
		var SessionId string

		//query db for BugId-BugName translation
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

		//query db for latest created session
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

		//create command to insert the node
		SqlCommand := fmt.Sprintf("INSERT INTO `testdb.%s_nodes` (`NodeId`, `XCoord`, `YCoord`) VALUES(?,?,?);", SessionId)

		//query db to create the command
		req, err := db.Query(SqlCommand, NodeNew.NodeId, NodeNew.XCoord, NodeNew.YCoord)
		if err != nil {
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			return
		}
		req.Close()

		//log
		fmt.Println("created SessionList entry", NodeNew)

		//return JSON
		c.Header("Access-Control-Allow-Origin", "*")
		c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
		c.JSON(http.StatusCreated, &NodeNew)
	}
}
