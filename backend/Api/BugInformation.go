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

)

//structs
type BugInformationStruct struct {
	BugId    string `json:"BugId"`
	BugName  string `json:"BugName"`
	LastSeen string `json:"LastSeen"`
}

type BugNameIdStruct struct {
	BugName string `json:"BugName"`
	BugId   string `json:"BugId"`
}

//display bug information
func DisplayBugInformation(c *gin.Context) {
	//make query
	rows, err := db.Query("SELECT * FROM testdb.BugInformation")
	if err != nil {
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
	}

	//Get all rows, add into the BugInformationList Struct
	BugInformationList := make([]BugInformationStruct, 0)

	//go through each row
	if rows != nil {
		defer rows.Close()
		for rows.Next() {
			// Individual row processing
			BugInformationRow := BugInformationStruct{}
			if err := rows.Scan(&BugInformationRow.BugId, &BugInformationRow.BugName, &BugInformationRow.LastSeen); err != nil {
				fmt.Println(err.Error())
				c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
				break
			}
			//append information
			BugInformationList = append(BugInformationList, BugInformationRow)
		}
	}

	//return JSON of all rows
	c.Header("Access-Control-Allow-Origin", "*")
	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
	c.JSON(http.StatusOK, &BugInformationList)
}

//Ping
func PingBugInformation(c *gin.Context) {
	//variables
	var BugInformationNew BugInformationStruct

	//input query
	BugInformationNew.BugId = c.Query("BugId")

	//get time and format timestamp
	t := time.Now() //.In(location)
	BugInformationNew.LastSeen = t.Format("2006-01-02 15:04:05")

	//check Query was inputted
	if len(BugInformationNew.BugId) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a BugId"})
	} else {
		//Query bugname
		var BugName string
		BugNameQuery := db.QueryRow("SELECT `BugName` FROM testdb.BugInformation WHERE `BugId`=?;", BugInformationNew.BugId)
		switch err := BugNameQuery.Scan(&BugName); err {
		case sql.ErrNoRows:
			BugInformationNew.BugName = BugInformationNew.BugId

			//insert
			req, err := db.Query("INSERT INTO testdb.BugInformation(`BugId`, `BugName`, `LastSeen`) VALUES(?, ?, ?);", BugInformationNew.BugId, BugInformationNew.BugName, BugInformationNew.LastSeen)
			if err != nil {
				fmt.Println(err.Error())
				c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			}
			req.Close();
		case nil:
			fmt.Println(BugName)

			//update field
			req, err := db.Query("UPDATE testdb.BugInformation SET `LastSeen`=? WHERE BugId=?;", BugInformationNew.LastSeen, BugInformationNew.BugId)
			if err != nil {
				fmt.Println(err.Error())
				c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			}
			req.Close();

			//log
			log.Println("updated BugInformation entry", BugInformationNew)
		default:
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			return
		}

		//return JSON
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
		req, err := db.Query("UPDATE testdb.BugInformation SET `BugName`=? WHERE BugId=?;", BugInformationNew.BugName, BugInformationNew.BugId)
		if err != nil {
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
		}
		req.Close()

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

	if len(TimeoutNew) > 0 {
		TimeoutExtracted, err := strconv.ParseFloat(TimeoutNew, 32)
		if err != nil {
			c.JSON(http.StatusNotAcceptable, gin.H{"message": "invalid timeout format, please enter a valid float"})
			return
		}
		Timeout = TimeoutExtracted
	}

	//location, err := time.LoadLocation("Europe/London")

	// if err != nil {
	// 	c.JSON(http.StatusInternalServerError, gin.H{"error": "Failed to load time zone"})
	// 	return
	// }

	t := time.Now() //.In(location)
	CurrentTime := t.Format("2006-01-02 15:04:05")
	fmt.Println(CurrentTime, Timeout)

	// read from DB
	rows, err := db.Query(`SELECT BugName, BugId
	                       FROM testdb.BugInformation 
			       WHERE TIMESTAMPDIFF(SECOND, LastSeen, ?) < ?`,
		CurrentTime, Timeout)
	if err != nil {
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
		return
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
				return
			}
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
