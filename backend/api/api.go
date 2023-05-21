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
	Id         string `json:"ConnTime"`
	SessionKey string `json:"SessionKey"`
	BugId      string `json:"done"`
}

var db *sql.DB
var err error

func SetupMySQL() {
	// db, err = sql.Open("postgres", "postgres://postgres:password@postgres/todo?sslmode=disable")

	// when running locally
	db, err = sql.Open("mysql", "mysql://mysql:password@tcp(127.0.0.1:3306)/todo?sslmode=disable")

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
	rows, err := db.Query("SELECT * FROM list")
	if err != nil {
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
	}

	// Get all rows and add into items
	items := make([]BotKey, 0)

	if rows != nil {
		defer rows.Close()
		for rows.Next() {
			// Individual row processing
			item := BotKey{}
			if err := rows.Scan(&item.Id, &item.SessionKey, &item.BugId); err != nil {
				fmt.Println(err.Error())
				c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			}
			item.SessionKey = strings.TrimSpace(item.SessionKey)
			items = append(items, item)
		}
	}

	// Return JSON object of all rows
	c.Header("Access-Control-Allow-Origin", "*")
	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
	c.JSON(http.StatusOK, gin.H{"items": items})
}

// func CreateBotEntry(c *gin.Context) {
// 	item := c.Param("item")

// 	// Validate item
// 	if len(item) == 0 {
// 		c.JSON(http.StatusNotAcceptable, gin.H{"message": "please enter an item"})
// 	} else {
// 		// Create todo item
// 		var TodoItem ListItem

// 		TodoItem.SessionKey = item
// 		TodoItem.BugId = "false"

// 		// Insert item to DB
// 		_, err := db.Query("INSERT INTO list(item, done) VALUES($1, $2);", TodoItem.Item, TodoItem.Done)
// 		if err != nil {
// 			fmt.Println(err.Error())
// 			c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})

// 		}

// 		// Log message
// 		log.Println("created todo item", item)

// 		// Return success response
// 		c.Header("Access-Control-Allow-Origin", "*")
// 		c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
// 		c.JSON(http.StatusCreated, gin.H{"items": &TodoItem})
// 	}
// }
