package Api

import (
	"fmt"
	"net/http"

	_ "github.com/go-sql-driver/mysql"

	"github.com/gin-gonic/gin"
	_ "github.com/lib/pq"
	"sql"

)


// CRUD: Create Read Update Delete API Format
// //DISPLAY SESSION IDs
// func DisplayAllNodes(c *gin.Context) {
// 	SessionId := c.Query("SessionId")
	
// 	if len(SessionId) == 0 {
// 		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a valid SessionId!"})
// 	}

// 	SqlQuery := fmt.Sprintf("SELECT * FROM testdb.%s_nodes", SessionId)
// 	rows, err := db.QueryRow(SqlQuery)
// 	if err != nil {
// 		fmt.Println(err.Error())
// 		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB, maybe the appropriate table hasn't been created yet"})
// 	}

// 	// Get all rows and add into SessionListStructs
// 	NodeLists := make([]NodeStruct, 0)

// 	if rows != nil {
// 		defer rows.Close()
// 		for rows.Next() {
// 			// Individual row processing
// 			NodeListRow := NodeStruct{}
// 			if err := rows.Scan(&NodeListRow.NodeId, &NodeListRow.XCoord, &NodeListRow.YCoord); err != nil {
// 				fmt.Println(err.Error())
// 				c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
// 				break
// 			}
// 			NodeLists = append(NodeLists, NodeListRow)
// 		}
// 	}

// 	// Return JSON object of all rows
// 	c.Header("Access-Control-Allow-Origin", "*")
// 	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
// 	c.JSON(http.StatusOK, &NodeLists)
// }

func TurnOn(c *gin.Context) {
	//var BeaconOnNew BeaconOnStruct
	BugId := c.Query("BugId")

// 	row := db.QueryRow(sqlStatement, 3)
// switch err := row.Scan(&id, &email); err {
// case sql.ErrNoRows:
//   fmt.Println("No rows were returned!")
// case nil:
//   fmt.Println(id, email)
// default:
//   panic(err)
// }


	if(len(BugId) == 0){
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "enter a BugId"})
	} else {
		var BugName string;


		BugNameQuery := db.QueryRow("SELECT `BugName` FROM testdb.BugInformation WHERE BugId=?;", BugId);
		switch err := BugNameQuery.Scan(&BugName); err {
		case sql.ErrNoRows:
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with BugId-BugName Translation"})
			return
		case nil:
			fmt.Println(BugName)
		default:
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
		}
	
		

		SessionIdQuery, errSessionId := db.QueryRow("SELECT `SessionId` FROM testdb.SessionList WHERE BugName=?;", BugNameQuery);
		if (errSessionId != nil) {
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			return
		} else if (len(SessionIdQuery) == 0) {
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with BugName-SessionId Translation"})
			return
		}




		TurnOnExistsQuery, errTurnOnExists := db.QueryRow("SELECT `SessionId` FROM testdb.BeaconOn WHERE SessionId=?;", SessionIdQuery);
		if (errSessionId != nil) {
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			return
		} else if (len(TurnOnExistsQuery) == 0) {
			c.Header("Access-Control-Allow-Origin", "*")
			c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
			c.JSON(http.StatusOK, "0")
			return
		}

		OnQuery, errOn := db.QueryRow("SELECT `On` FROM testdb.BeaconOn WHERE SessionId=?;", SessionIdQuery);
		if (errOn != nil) {
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			return
		}

		if (OnQuery == "1") {
			c.Header("Access-Control-Allow-Origin", "*")
			c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
			c.JSON(http.StatusOK, "2")
			return
		}
		

		ChargeQuery, errOn := db.QueryRow("SELECT `Charged` FROM testdb.BeaconCharge WHERE SessionId=?;", SessionIdQuery);
		if (errOn != nil) {
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			return
		}

		if (ChargeQuery == "0") {
			c.Header("Access-Control-Allow-Origin", "*")
			c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
			c.JSON(http.StatusOK, "1")
			return
		}

		// Update DB
		_, err := db.QueryRow("UPDATE testdb.Charge SET `Charged`=?;", "1")
		if (err != nil) {
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			return
		}

		// Return success response
		c.Header("Access-Control-Allow-Origin", "*")
		c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
		c.JSON(http.StatusOK, "3")
	}
}
