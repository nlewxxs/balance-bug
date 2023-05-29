package DatabaseApi

import (
	"fmt"
	"log"
	"net/http"
	"strings"
	"time"

	_ "github.com/go-sql-driver/mysql"

	"github.com/gin-gonic/gin"
	_ "github.com/lib/pq"
)

type BugInformationStruct struct {
	BugName  string `json:"BugId"`
	LastSeen string `json:"LastSeen"`
}

// CRUD: Create Read Update Delete API Format
// DISPLAY SESSION IDs
func DisplayBugInformation(c *gin.Context) {
	rows, err := db.Query("SELECT * FROM testdb.BugInformation")
	if err != nil {
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
	}

	// Get all rows and add into SessionListStructs
	BugInformationList := make([]BugInformationStruct, 0)

	if rows != nil {
		defer rows.Close()
		for rows.Next() {
			// Individual row processing
			BugInformationRow := BugInformationStruct{}
			if err := rows.Scan(&BugInformationRow.BugName, &BugInformationRow.LastSeen); err != nil {
				fmt.Println(err.Error())
				c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			}
			BugInformationRow.BugName = strings.TrimSpace(BugInformationRow.BugName)
			BugInformationList = append(BugInformationList, BugInformationRow)
		}
	}

	// Return JSON object of all rows
	c.Header("Access-Control-Allow-Origin", "*")
	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
	c.JSON(http.StatusOK, &BugInformationList)
}

func AddBugInformation(c *gin.Context) {
	var BugInformationNew BugInformationStruct

	BugInformationNew.BugName = c.Query("BugName")

	t := time.Now()
	BugInformationNew.LastSeen = t.Format(time.RFC850)

	// Validate entry
	if len(BugInformationNew.BugName) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a BugName"})
	} else {
		// Insert item to DB
		_, err := db.Query("INSERT INTO testdb.BugInformation(`BugName`, `LastSeen`) VALUES(?, ?);", BugInformationNew.BugName, BugInformationNew.LastSeen)
		if err != nil {
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
		}

		// Log message
		log.Println("created SessionList entry", BugInformationNew)

		// Return success response
		c.Header("Access-Control-Allow-Origin", "*")
		c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
		c.JSON(http.StatusCreated, &BugInformationNew)
	}
}
