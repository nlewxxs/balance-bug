package Api

//imports
import (
	"fmt"
	"net/http"

	_ "github.com/go-sql-driver/mysql"

	"database/sql"

	"github.com/gin-gonic/gin"
	_ "github.com/lib/pq"
)

//Structs
type EdgeStruct struct {
	NodeId     string `json:"NodeId"`
	EdgeNodeId string `json:"EdgeNodeId"`
	Distance   string `json:"Distance"`
	Angle      string `json:"Angle"`
}

type ProcessedEdgeStruct struct {
	From  string `json:"from"`
	To    string `json:"to"`
	Label string `json:"label"`
}


// Create the table, this is REQUIRED before reading
func CreateEdgeTable(c *gin.Context) {
	//query
	BugId := c.Query("BugId")

	//check if query was inputted
	if len(BugId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "enter a BugId"})
		return
	}

	//variables used
	var BugName string
	var SessionId string

	//query db for BugId-BugName Translation
	BugNameQuery := db.QueryRow("SELECT `BugName` FROM `BugInformation` WHERE `BugId`=?;", BugId)
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

	//query if latest session created exists
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

	//create the table
	SqlCommand := fmt.Sprintf("CREATE TABLE IF NOT EXISTS `testdb.%s_edges` (`NodeId` char(100) NOT NULL, `EdgeNodeId` char(100) NOT NULL, `Distance` char(100), `Angle` char(100), PRIMARY KEY (`NodeId`, `EdgeNodeId`), FOREIGN KEY (`NodeId`) REFERENCES `testdb.%s_nodes`(`NodeId`), FOREIGN KEY (`EdgeNodeId`) REFERENCES `testdb.%s_nodes`(`NodeId`)) ENGINE=InnoDB;", SessionId, SessionId, SessionId)

	req, err := db.Query(SqlCommand)
	if err != nil {
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "could not create table"})
		return
	}
	req.Close()

	// return JSON
	c.Header("Access-Control-Allow-Origin", "*")
	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
	c.JSON(http.StatusCreated, gin.H{"message": "successfully created new table"})
}

//Display Edges
func DisplayAllEdges(c *gin.Context) {
	//query
	SessionId := c.Query("SessionId")

	//check if query was inputted
	if len(SessionId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a valid SessionId!"})
		return
	}
	
	//query all items in the table associated to that of the input
	SqlQuery := fmt.Sprintf("SELECT * FROM `testdb.%s_edges`", SessionId)
	rows, err := db.Query(SqlQuery)
	if err != nil {
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB, maybe the appropriate table hasn't been created yet"})
	}

	//get all rows and make into ProcessedEdgeStruct, to directly feed into the graph viewer
	EdgeLists := make([]ProcessedEdgeStruct, 0)

	//process the rows
	if rows != nil {
		defer rows.Close()
		for rows.Next() {
			// Individual row processing
			EdgeListRow := ProcessedEdgeStruct{}
			edgeListRowDiscard := EdgeStruct{}
			if err := rows.Scan(&EdgeListRow.From, &EdgeListRow.To, &EdgeListRow.Label, &edgeListRowDiscard.Angle); err != nil {
				fmt.Println(err.Error())
				c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
				break
			}
			EdgeLists = append(EdgeLists, EdgeListRow)
		}
	}

	//return JSON of the rows
	c.Header("Access-Control-Allow-Origin", "*")
	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
	c.JSON(http.StatusOK, &EdgeLists)
}

//Add the Edge
func AddEdge(c *gin.Context) {
	//variables
	var EdgeNew EdgeStruct
	var BugName string
	var SessionId string

	//input Queries
	BugId := c.Query("BugId")
	EdgeNew.NodeId = c.Query("NodeId")
	EdgeNew.EdgeNodeId = c.Query("EdgeNodeId")
	EdgeNew.Distance = c.Query("Distance")
	EdgeNew.Angle = c.Query("Angle")

	//check the input queries were inputted
	if len(EdgeNew.NodeId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a NodeId"})
	} else if len(EdgeNew.EdgeNodeId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a EdgeNodeId"})
	} else if len(EdgeNew.Distance) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a Distance"})
	} else if len(EdgeNew.Angle) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a Angle"})
	} else if len(BugId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a BugId"})
	} else {
		//query db for BugId-BugName Translation
		BugNameQuery := db.QueryRow("SELECT `BugName` FROM `BugInformation` WHERE `BugId`=?;", BugId)
		switch err := BugNameQuery.Scan(&BugName); err {
		case sql.ErrNoRows:
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with BugId-BugName Translation"})
			return
		case nil:
			fmt.Println(BugName)
		default:
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB1"})
			return
		}

		//query db for latest session
		SessionIdQuery := db.QueryRow("SELECT `SessionId` FROM testdb.SessionList ORDER BY `TimeStamp` DESC LIMIT 1;")
		switch err := SessionIdQuery.Scan(&SessionId); err {
		case sql.ErrNoRows:
			c.JSON(http.StatusInternalServerError, gin.H{"message": "No Session Exists"})
			return
		case nil:
			fmt.Println(SessionId)
		default:
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB2"})
			return
		}

		//insert entry into the db
		SqlCommand := fmt.Sprintf("INSERT INTO `testdb.%s_edges` (`NodeId`, `EdgeNodeId`, `Distance`, `Angle`) VALUES((SELECT NodeId FROM `testdb.%s_nodes` WHERE NodeId='%s'),(SELECT NodeId FROM `testdb.%s_nodes` WHERE NodeId='%s'),%s,%s);", SessionId, SessionId, EdgeNew.NodeId, SessionId, EdgeNew.EdgeNodeId, EdgeNew.Distance, EdgeNew.Angle)

		//do query
		req, err := db.Query(SqlCommand)
		if err != nil {
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB3"})
			return
		}
		req.Close()

		//log
		fmt.Println("created SessionList entry", EdgeNew)

		//return Edge created (can be used for checking of correct entry if required)
		c.Header("Access-Control-Allow-Origin", "*")
		c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
		c.JSON(http.StatusCreated, &EdgeNew)
	}
}
