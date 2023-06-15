package Api

import (
	"fmt"
	"net/http"

	_ "github.com/go-sql-driver/mysql"

	"github.com/gin-gonic/gin"
	_ "github.com/lib/pq"
	"database/sql"

	"time"
)

func BeaconPing(c *gin.Context) {
	ChargeStatus := c.Query("ChargeStatus")

	var SessionId 	string;
	var OnExists 	string;



	SessionIdQuery := db.QueryRow("SELECT `SessionId` FROM testdb.SessionList ORDER BY `TimeStamp` DESC LIMIT 1;");
	switch err := SessionIdQuery.Scan(&SessionId); err {
	case sql.ErrNoRows:
		c.JSON(http.StatusInternalServerError, gin.H{"message": "No Session Exists"})
		return
	case nil:
		fmt.Println(SessionId)
	default:
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
	}
	
	TurnOnQuery := db.QueryRow("SELECT `On` FROM testdb.BeaconOn WHERE `SessionId`=?;", SessionId);
	

	switch err := TurnOnQuery.Scan(&OnExists); err {
	case sql.ErrNoRows:
		if(len(ChargeStatus) == 0){
			c.JSON(http.StatusInternalServerError, gin.H{"message": "Not Initialised, ChargeStatus is required when hooking to a new session"})
		}
		//c.JSON(http.StatusInternalServerError, gin.H{"message": "error with TurnOn query"})
		createOnEntry, errCreateOnEntry := db.Query("INSERT INTO testdb.BeaconOn (`SessionId`, `On`) VALUES(?,?);", SessionId, "0")
		if (errCreateOnEntry != nil) {
			fmt.Println(err.Error())
			fmt.Println(createOnEntry)
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			return
		}

		
		createChargeEntry, errCreateChargeEntry := db.Query("INSERT INTO testdb.BeaconCharge (`SessionId`, `ChargeStatus`) VALUES(?,?);", SessionId, ChargeStatus)
		if (errCreateChargeEntry != nil) {
			fmt.Println(err.Error())
			fmt.Println(createChargeEntry)
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			return
		}

		time.Sleep(100 * time.Millisecond)
	case nil:
		fmt.Println(OnExists)

		if(len(ChargeStatus) != 0){
			ChargeSet, errChargeSet := db.Query("UPDATE testdb.BeaconCharge SET `ChargeStatus`=? WHERE SessionId=?;", ChargeStatus, SessionId);
			
			if (errChargeSet != nil) {
				fmt.Println(err.Error())
				fmt.Println(errChargeSet)
				fmt.Println(ChargeSet)
				c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
				return
			}
		}

		// OnSet, errOnSet := db.Query("UPDATE testdb.BeaconOn SET `On`=0 WHERE SessionId=?;", SessionId);
			
		// if (errOnSet != nil) {
		// 	fmt.Println(err.Error())
		// 	fmt.Println(errOnSet)
		// 	fmt.Println(OnSet)
		// 	c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
		// 	return
		// }

		c.Header("Access-Control-Allow-Origin", "*")
		c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
		c.JSON(http.StatusOK, &OnExists)		
		return
	default:
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
	}
}
