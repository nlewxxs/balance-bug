package Api

//imports
import (
	"fmt"
	"net/http"

	_ "github.com/go-sql-driver/mysql"

	"github.com/gin-gonic/gin"
	_ "github.com/lib/pq"
	"database/sql"

	"time"
)

//ping beacon
func BeaconPing(c *gin.Context) {
	//queries
	ChargeStatus := c.Query("ChargeStatus")

	//variables to set
	var SessionId 	string;
	var OnExists 	string;

	//select latest session to latch onto
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
		return
	}
	
	//Check if On Entry on SessionId exists
	TurnOnQuery := db.QueryRow("SELECT `On` FROM testdb.BeaconOn WHERE `SessionId`=?;", SessionId);

	switch err := TurnOnQuery.Scan(&OnExists); err {
	case sql.ErrNoRows:
		if(len(ChargeStatus) == 0){
			c.JSON(http.StatusInternalServerError, gin.H{"message": "Not Initialised, ChargeStatus is required when hooking to a new session"})
		}

		//case doesn't exist, add this row
		createOnEntry, errCreateOnEntry := db.Query("INSERT INTO testdb.BeaconOn (`SessionId`, `On`) VALUES(?,?);", SessionId, "0")
		if (errCreateOnEntry != nil) {
			fmt.Println(err.Error())
			fmt.Println(createOnEntry)
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			return
		}

		//initialise this row
		createChargeEntry, errCreateChargeEntry := db.Query("INSERT INTO testdb.BeaconCharge (`SessionId`, `ChargeStatus`) VALUES(?,?);", SessionId, ChargeStatus)
		if (errCreateChargeEntry != nil) {
			fmt.Println(err.Error())
			fmt.Println(createChargeEntry)
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			return
		}

		//wait for this request to be complete
		time.Sleep(100 * time.Millisecond)
	case nil:
		fmt.Println(OnExists)

		//if an entry is parsed, update this entry
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

		c.Header("Access-Control-Allow-Origin", "*")
		c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
		c.JSON(http.StatusOK, &OnExists)		
		return
	default:
		//error catching and return this error
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
		return
	}
}
