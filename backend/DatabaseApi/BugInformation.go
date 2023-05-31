package DatabaseApi

import (
	"fmt"
	"log"
	"net/http"
	"strings"
	"time"
	"strconv"

	_ "github.com/go-sql-driver/mysql"

	"github.com/gin-gonic/gin"
	_ "github.com/lib/pq"
)

type BugInformationStruct struct {
	BugId    string `json:"BugId"`
	BugName  string `json:"BugName"`
	LastSeen string `json:"LastSeen"`
}

type BugNameIdStruct struct {
	BugId    string `json:"BugId"`
	BugName  string `json:"BugName"`
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
			if err := rows.Scan(&BugInformationRow.BugId,&BugInformationRow.BugName, &BugInformationRow.LastSeen); err != nil {
				fmt.Println(err.Error())
				c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
				break
			}
			BugInformationRow.BugId = strings.TrimSpace(BugInformationRow.BugId)
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

	BugInformationNew.BugId = c.Query("BugId")
	BugInformationNew.BugName = c.Query("BugName")

	t := time.Now()
	BugInformationNew.LastSeen = t.Format("2006-01-02 15:04:05")

	// Validate entry
	if len(BugInformationNew.BugId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a BugId"})
	} else {
		
		if len(BugInformationNew.BugName) == 0 {
			BugInformationNew.BugName = BugInformationNew.BugId
		}

		// Insert item to DB
		_, err := db.Query("INSERT INTO testdb.BugInformation(`BugId`, `BugName`, `LastSeen`) VALUES(?, ?, ?);", BugInformationNew.BugId, BugInformationNew.BugName, BugInformationNew.LastSeen)
		if err != nil {
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
		}

		// Log message
		log.Println("created BugInformation entry", BugInformationNew)

		// Return success response
		c.Header("Access-Control-Allow-Origin", "*")
		c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
		c.JSON(http.StatusCreated, &BugInformationNew)
	}
}

func PingBugInformation(c *gin.Context) {
	var BugInformationNew BugInformationStruct

	BugInformationNew.BugId = c.Query("BugId")

	t := time.Now()
	BugInformationNew.LastSeen = t.Format("2006-01-02 15:04:05")

	// Validate entry
	if len(BugInformationNew.BugId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a BugId"})
	} else {
		// Insert item to DB
		_, err := db.Query("UPDATE testdb.BugInformation SET `LastSeen`=? WHERE BugId=?;", BugInformationNew.LastSeen, BugInformationNew.BugId)
		if err != nil {
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
		}

		// Log message
		log.Println("updated BugInformation entry", BugInformationNew)

		// Return success response
		c.Header("Access-Control-Allow-Origin", "*")
		c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
		c.JSON(http.StatusOK, &BugInformationNew)
	}
}

func UpdateBugNameBugInformation(c *gin.Context) {
	var BugInformationNew BugInformationStruct

	BugInformationNew.BugId = c.Query("BugId")
	BugInformationNew.BugName = c.Query("BugName")

	// Validate entry
	if len(BugInformationNew.BugId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a BugId"})
	} else if len(BugInformationNew.BugName) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a BugName"})
	} else {
		// Insert item to DB
		_, err := db.Query("UPDATE testdb.BugInformation SET `BugName`=? WHERE BugId=?;", BugInformationNew.BugName, BugInformationNew.BugId)
		if err != nil {
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
		}

		// Log message
		log.Println("updated BugInformation entry", BugInformationNew)

		// Return success response
		c.Header("Access-Control-Allow-Origin", "*")
		c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
		c.JSON(http.StatusOK, &BugInformationNew)
	}
}


func OnlineBugInformation(c *gin.Context) {

	TimeoutNew := c.Query("Timeout")
	Timeout := 5.0
	
	if (len(TimeoutNew) > 0) {
		TimeoutExtracted, err := strconv.ParseFloat(TimeoutNew, 32)
		if err != nil {
			c.JSON(http.StatusNotAcceptable, gin.H{"message" : "invalid timeout format, please enter a valid float"})
		}
		Timeout = TimeoutExtracted
	}

	t := time.Now()
	CurrentTime := t.Format("2006-01-02 15:04:05")
	fmt.Println(CurrentTime, Timeout)

	// read from DB
	rows, err := db.Query(`SELECT BugId, BugName
	                       FROM testdb.BugInformation 
			       WHERE TIMESTAMPDIFF(SECOND, LastSeen, ?) < ?`,
			       CurrentTime, Timeout)
	if err != nil {
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
	}

	// Get all rows and add into SessionListStructs
	BugInformationList := make([]BugNameIdStruct, 0)

	if rows != nil {
		defer rows.Close()
		for rows.Next() {
			// Individual row processing
			BugInformationRow := BugNameIdStruct{}
			if err := rows.Scan(&BugInformationRow.BugName, &BugInformationRow.BugId); err != nil {
				fmt.Println(err.Error())
				c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
				break
			}
			BugInformationRow.BugId = strings.TrimSpace(BugInformationRow.BugId)
			// BugInformationRow.BugName = strings.TrimSpace(BugInformationRow.BugName)
			BugInformationList = append(BugInformationList, BugInformationRow)
		}
	}


	// Log message
	log.Println("Retrieved Online Bugs")

	// Return success response
	c.Header("Access-Control-Allow-Origin", "*")
	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
	c.JSON(http.StatusOK, &BugInformationList)
}