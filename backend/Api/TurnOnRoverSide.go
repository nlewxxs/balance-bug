package Api

import (
	"fmt"
	"net/http"

	_ "github.com/go-sql-driver/mysql"

	"github.com/gin-gonic/gin"
	_ "github.com/lib/pq"
	"database/sql"
)

func TurnOn(c *gin.Context) {
	BugId := c.Query("BugId")

	if(len(BugId) == 0){
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "enter a BugId"})
	} else {
		var BugName 	string;
		var SessionId 	string;
		var OnExists 	string;
		var On 			string;
		var ChargeStatus		string;



		BugNameQuery := db.QueryRow("SELECT `BugName` FROM testdb.BugInformation WHERE `BugId`=?;", BugId);
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

		SessionIdQuery := db.QueryRow("SELECT `SessionId` FROM testdb.SessionList WHERE `BugName`=?;", BugName);
		switch err := SessionIdQuery.Scan(&SessionId); err {
		case sql.ErrNoRows:
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with SessionId-BugName Translation"})
			return
		case nil:
			fmt.Println(SessionId)
		default:
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
		}

		TurnOnExistsQuery := db.QueryRow("SELECT `SessionId` FROM testdb.BeaconOn WHERE `SessionId`=?;", SessionId);

		switch err := TurnOnExistsQuery.Scan(&OnExists); err {
		case sql.ErrNoRows:
			c.Header("Access-Control-Allow-Origin", "*")
			c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
			c.JSON(http.StatusOK, "0")
			return
		case nil:
			fmt.Println(OnExists)
			
		default:
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
		}

		OnQuery := db.QueryRow("SELECT `On` FROM testdb.BeaconOn WHERE SessionId=?;", SessionId);
		switch err := OnQuery.Scan(&On); err {
		case sql.ErrNoRows:
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with On query"})
			return
		case nil:
			fmt.Println(On)
			if (On == "1") {
				c.Header("Access-Control-Allow-Origin", "*")
				c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
				c.JSON(http.StatusOK, "2")
				return
			}
		default:
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
		}

		ChargeQuery := db.QueryRow("SELECT `ChargeStatus` FROM testdb.BeaconCharge WHERE `SessionId`=?;", SessionId);

		switch err := ChargeQuery.Scan(&ChargeStatus); err {
		case sql.ErrNoRows:
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with Charge Status query"})
			return
		case nil:
			fmt.Println(ChargeStatus)
			if (ChargeStatus == "0") {
				c.Header("Access-Control-Allow-Origin", "*")
				c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
				c.JSON(http.StatusOK, "1")
				return
			}
		default:
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
		}

		

		// Update DB
		_, err := db.Query("UPDATE testdb.BeaconOn SET `On`=?;", "1")
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
