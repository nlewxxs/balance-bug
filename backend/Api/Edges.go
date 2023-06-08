package Api

import (
	"fmt"
	"net/http"

	_ "github.com/go-sql-driver/mysql"

	"github.com/gin-gonic/gin"
	_ "github.com/lib/pq"

)


// CHANGE LATER
type EdgeStruct struct {
	NodeId  string `json:"NodeId"`
	EdgeNodeId  string `json:"EdgeNodeId"` 
	Distance  string `json:"Distance"`
	Angle  string `json:"Angle"`
}

// CREATE TABLE IF NOT EXISTS testdb.test_edges4
// (
// `EdgeNodeId` char(100) NOT NULL, 
// `NodeId` char(100) NOT NULL, 
// `Distance` char(100), 
// `Angle` char(100), 
// PRIMARY KEY (`NodeId`, `EdgeNodeId`),
// FOREIGN KEY (`EdgeNodeId`) REFERENCES example(`NodeId`),
// FOREIGN KEY (`NodeId`) REFERENCES example(`NodeId`)
// )
// ENGINE=InnoDB;

//Create the table, this is REQUIRED before reading
func CreateEdgeTable (c *gin.Context) {
	SessionId := c.Query("SessionId")

	if len(SessionId) == 0{
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "enter a SessionId"})
	}

	SqlCommand := fmt.Sprintf("CREATE TABLE IF NOT EXISTS testdb.%s_edges (`NodeId` char(100) NOT NULL, `EdgeNodeId` char(100) NOT NULL, `Distance` char(100), `Angle` char(100), PRIMARY KEY (`NodeId`, `EdgeNodeId`), FOREIGN KEY (`NodeId`) REFERENCES testdb.%s_nodes(`NodeId`), FOREIGN KEY (`EdgeNodeId`) REFERENCES testdb.%s_nodes(`NodeId`)) ENGINE=InnoDB;", SessionId,SessionId,SessionId)
	
	_, err := db.Query(SqlCommand)
    if err != nil {
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "could not create table"})
    }
	// Return JSON object of all rows
	c.Header("Access-Control-Allow-Origin", "*")
	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
	c.JSON(http.StatusCreated, gin.H{"message": "successfully created new table"})
}



// CRUD: Create Read Update Delete API Format
//DISPLAY SESSION IDs
func DisplayAllEdges(c *gin.Context) {
	SessionId := c.Query("SessionId")
	
	if len(SessionId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a valid SessionId!"})
	}

	SqlQuery := fmt.Sprintf("SELECT * FROM testdb.%s_edges", SessionId)
	rows, err := db.Query(SqlQuery)
	if err != nil {
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB, maybe the appropriate table hasn't been created yet"})
	}

	// Get all rows and add into SessionListStructs
	EdgeLists := make([]EdgeStruct, 0)

	if rows != nil {
		defer rows.Close()
		for rows.Next() {
			// Individual row processing
			EdgeListRow := EdgeStruct{}
			if err := rows.Scan(&EdgeListRow.NodeId, &EdgeListRow.EdgeNodeId, &EdgeListRow.Distance, &EdgeListRow.Angle); err != nil {
				fmt.Println(err.Error())
				c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
				break
			}
			EdgeLists = append(EdgeLists, EdgeListRow)
		}
	}

	// Return JSON object of all rows
	c.Header("Access-Control-Allow-Origin", "*")
	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
	c.JSON(http.StatusOK, &EdgeLists)
}

func AddEdge(c *gin.Context) {
	var EdgeNew EdgeStruct

	SessionId := c.Query("SessionId")
	EdgeNew.NodeId = c.Query("NodeId")
	EdgeNew.EdgeNodeId = c.Query("EdgeNodeId")
	EdgeNew.Distance = c.Query("Distance")
	EdgeNew.Angle = c.Query("Angle")

	// Validate entry
	if len(EdgeNew.NodeId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a NodeId"})
	} else if len(EdgeNew.EdgeNodeId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a EdgeNodeId"})
	} else if len(EdgeNew.Distance) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a Distance"})
	} else if len(EdgeNew.Angle) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a Angle"})
	} else if len(SessionId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a SessionId"})
	}else {
		// Insert item to DB
		// fmt.Println("SessionID: ", SessionId)
		//TODO: ADD FOREIGN KEY FOR SYMBOLIC LINK
		//NEST A REQUEST, LUCKILY READ HAVE VERY LOW LATENCY

		
		SqlCommand := fmt.Sprintf("INSERT INTO testdb.%s_edges (`NodeId`, `EdgeNodeId`, `Distance`, `Angle`) VALUES((SELECT NodeId FROM testdb.%s_nodes WHERE NodeId='%s'),(SELECT NodeId FROM testdb.%s_nodes WHERE NodeId='%s'),%s,%s);", SessionId, SessionId, EdgeNew.NodeId, SessionId, EdgeNew.EdgeNodeId, EdgeNew.Distance, EdgeNew.Angle)


		// fmt.Println("SQL Command: ", SqlCommand)

		_, err := db.Query(SqlCommand)
		if err != nil {
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
		}

		// Log message
		fmt.Println("created SessionList entry", EdgeNew)

		// Return success response
		c.Header("Access-Control-Allow-Origin", "*")
		c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
		c.JSON(http.StatusCreated, &EdgeNew)
	}
}




