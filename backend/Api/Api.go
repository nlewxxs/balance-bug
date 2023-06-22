package Api

//imports
import (
	"database/sql"
	"log"
	"fmt"
	_ "github.com/go-sql-driver/mysql"
)

//db variables
var db *sql.DB
var err error

func SetupMySQL() {
	//open db
	db, err = sql.Open("mysql", "root:helloworld@tcp(172.17.0.1:3308)/testdb?timeout=5s")

	//check if db opened
	if err != nil {
		fmt.Println(err.Error())
	}

	if err = db.Ping(); err != nil {
		fmt.Println(err.Error())
	}

	//add log to check upon startup
	log.Println("connected to mysql")
}