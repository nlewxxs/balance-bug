package api

import (
	"database/sql"
	"fmt"
	"log"
	"net/http"
	"strings"

	_ "github.com/go-sql-driver/mysql"

	"github.com/gin-gonic/gin"
	_ "github.com/lib/pq"
)

type BotKey struct {
	ConnTime         string `json:"ConnTime"`
	BugId      		string  `json:"done"`
	SessionKey 		string  `json:"SessionKey"`

}

var db *sql.DB
var err error

func SetupMySQL() {
	// for local testing only:

	db, err = sql.Open("mysql", "root:helloworld@tcp(172.17.0.1:3308)/testdb?timeout=5s")

	if err != nil {
		fmt.Println(err.Error())
	}

	if err = db.Ping(); err != nil {
		fmt.Println(err.Error())
	}

	log.Println("connected to mysql")
}

// CRUD: Create Read Update Delete API Format
func DisplayBotKey(c *gin.Context) {
	rows, err := db.Query("SELECT * FROM testdb.BotKey")
	if err != nil {
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
	}

	// Get all rows and add into BotKeys
	BotKeys := make([]BotKey, 0)

	if rows != nil {
		defer rows.Close()
		for rows.Next() {
			// Individual row processing
			item := BotKey{}
			if err := rows.Scan(&item.ConnTime, &item.BugId, &item.SessionKey); err != nil {
				fmt.Println(err.Error())
				c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			}
			item.SessionKey = strings.TrimSpace(item.SessionKey)
			BotKeys = append(BotKeys, item)
		}
	}

	// Return JSON object of all rows
	c.Header("Access-Control-Allow-Origin", "*")
	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
	c.JSON(http.StatusOK, gin.H{"BotKeys": BotKeys})
}

func CreateBotEntry(c *gin.Context) {
	TimeStampNew := c.Param("ConnTime")
	BugIdNew := c.Param("BugId")
	SessionKeyNew := c.Param("SessionKey")

	// Validate entry
	if len(TimeStampNew) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a BotKey.ConnTime"})
	} else if len(BugIdNew) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a BotKey.BugId"})
	} else if len(SessionKeyNew) == 0 {
		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter a BotKey.SessionKey"})
	} else {
		// Create todo item
		var BotKeyList BotKey

		BotKeyList.ConnTime = TimeStampNew
		BotKeyList.BugId = BugIdNew
		BotKeyList.SessionKey = SessionKeyNew

		// Insert item to DB
		_, err := db.Query("INSERT INTO testdb.BotKey(ConnTime, SessionKey, BugId) VALUES(?, ?, ?);", BotKeyList.ConnTime, BotKeyList.BugId, BotKeyList.SessionKey)
		if err != nil {
			fmt.Println(err.Error())
			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
		}

		// Log message
		log.Println("created BugKey entry", BotKeyList)

		// Return success response
		c.Header("Access-Control-Allow-Origin", "*")
		c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
		c.JSON(http.StatusCreated, gin.H{"BotKeyCreation": &BotKeyList})
	}
}
