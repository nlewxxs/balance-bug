package api

import (
	"fmt"
	"net/http"
	"strings"

	_ "github.com/go-sql-driver/mysql"

	"github.com/gin-gonic/gin"
	_ "github.com/lib/pq"

)


// CHANGE LATER
type EdgeStruct struct {
	EdgeId   string `json:"EdgeId"`/* MIGHT NOT NEED */
//  REPLACE EdgeId with {NodeId,EdgeNodeId} if you want?
	NodeId  string `json:"NodeId"`
	//ForeighKEY
	EdgeNodeId  string `json:"EdgeNodeId"` 
	//ForeighKEY
	Distance  string `json:"Distance"`
	Angle  string `json:"Angle"`

	//PRIMARYKEY (NodeId, EdgeNodeId)
}
/*

JHSBFKDSNKNDNFKDN
 PLEASE CHANGE THE STRUCT LATER

 */

//Create the table, this is REQUIRED before reading
func CreateEdgeTable (c *gin.Context) {
	SessionId := c.Query("SessionId")

	SqlCommand := fmt.Sprintf("CREATE TABLE IF NOT EXISTS testdb.%s_edges (`NodeId` char(100) NOT NULL, `XCoord` char(100) NOT NULL, `YCoord` char(100) NOT NULL, PRIMARY KEY (`NodeId`, `XCoord`, `YCoord`)) ENGINE=InnoDB;", SessionId)
	
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

			///////////////////////////////////////
			////////// E D G E   I D //////////////
			///////////////////////////////////////
			if err := rows.Scan(&EdgeListRow.EdgeId, &EdgeListRow.NodeId, &EdgeListRow.EdgeNodeId, &EdgeListRow.Distance, &EdgeListRow.Angle ); err != nil {
				fmt.Println(err.Error())
				c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			}
			///////////////////////////////////////
			////////// E D G E   I D //////////////
			///////////////////////////////////////
			EdgeRows.EdgeId = strings.TrimSpace(EdgeListRow.EdgeId)
			EdgeLists = append(EdgeLists, EdgeListRow)
		}
	}

	// Return JSON object of all rows
	c.Header("Access-Control-Allow-Origin", "*")
	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
	c.JSON(http.StatusOK, &EdgeLists)
}




