package Api

import (
	"database/sql"
	"log"
	"fmt"
	_ "github.com/go-sql-driver/mysql"


)

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