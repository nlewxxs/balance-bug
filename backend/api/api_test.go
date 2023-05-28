package api

import (
	"encoding/json"
	"fmt"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"

	"github.com/stretchr/testify/assert"

	"github.com/gin-gonic/contrib/cors"
	"github.com/gin-gonic/gin"
)

var router *gin.Engine

// print database
func displayBotKey(c *gin.Context) {
	rows, err := db.Query("SELECT * FROM list")
	if err != nil {
		fmt.Println(err.Error())
		c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
	}

	// Get all rows and add into items
	SessionListOverall := make([]SessionListStruct, 0)

	if rows != nil {
		defer rows.Close()
		for rows.Next() {
			// Individual row processing
			SessionListRow := SessionListStruct{}
			if err := rows.Scan(&SessionListRow.TimeStamp, &SessionListRow.BugName, &SessionListRow.SessionId); err != nil {
				fmt.Println(err.Error())
				c.JSON(http.StatusInternalServerError, gin.H{"message": "error with DB"})
			}
			SessionListRow.SessionId = strings.TrimSpace(SessionListRow.SessionId)
			SessionListOverall = append(SessionListOverall, SessionListRow)
		}
	}
}

// Delete all elements
// from DB
func emptyTable() {
	db.Exec("DELETE from testdb.SessionList;")

	// Reset id counter
	//db.Exec("ALTER SEQUENCE list_id_seq RESTART WITH 1;")
}

// Setup Gin Routes
func SetupRoutes() *gin.Engine {
	// Use Gin as router
	router := gin.Default()
	config := cors.DefaultConfig()
	config.AllowAllOrigins = true
	router.Use(cors.New(config))

	// Set routes for API
	router.GET("/Session/DisplayAll", DisplaySessionList)
	router.GET("/Session/Create", CreateSession)
	// router.GET("/Botkey/update/:id/:done", UpdateTodoItem)
	// // router.GET("/item/delete/:id", DeleteTodoItem)

	// Set up Gin Server
	return router
}

// Perform Reuest
// and return response
func performRequest(r http.Handler, method, path string) *httptest.ResponseRecorder {
	req, _ := http.NewRequest(method, path, nil)
	w := httptest.NewRecorder()
	r.ServeHTTP(w, req)
	return w
}

// Init Test Function
func TestMain(t *testing.T) {
	SetupMySQL()
	router = SetupRoutes()
}

// Test for successfull GET
// response from /items
// with no elements
func TestEmptyDisplaySessionList(t *testing.T) {
	// clear db table
	emptyTable()

	// /items GET request and check 200 OK status code
	w := performRequest(router, "GET", "/Session/DisplayAll")
	
	//check for response from server, and that response is ok
	assert.Equal(t, http.StatusOK, w.Code)

	//obtain and process response
	var response []SessionListStruct
	err := json.Unmarshal([]byte(w.Body.String()), &response)
	value := response

	// No error in response
	assert.Nil(t, err)

	// Assert response is what is expected, in this case empty
	assert.Equal(t, []SessionListStruct{}, value)
}

// // Test for successfull create
// // response from /item/create
func TestBotKeyCreate(t *testing.T) {
	// Delete all elements
	// from DB
	emptyTable()

	// Expected body
	expected := SessionListStruct{
			TimeStamp: "2006-01-02 15:04:05",
			BugName: "2",
			SessionId: "",
	}

	// /item/create GET request and check 200 OK status code
	///Session/Create?TimeStamp=${dateTime}&BugName=${sessionName}
	w := performRequest(router, "GET", "/Session/Create?TimeStamp=2006-01-02 15:04:05&BugName=2")
	assert.Equal(t, http.StatusCreated, w.Code)

	// Obtain response
	var response SessionListStruct
	err := json.Unmarshal([]byte(w.Body.String()), &response)
	
	exists := false
	if ((len(response.TimeStamp) > 0) && (len(response.BugName) > 0)) {
		exists = true 
	}

	// No error in response
	assert.Nil(t, err)

	// Check if response exits
	assert.True(t, exists)

	// Assert response
	assert.Equal(t, expected.TimeStamp, response.TimeStamp)
	assert.Equal(t, expected.BugName, response.BugName)
}

// // Test for successfull creates
// // response for multiple items
// // from /item/create
// func TestItemsCreate(t *testing.T) {
// 	// Delete all elements
// 	// from DB
// 	emptyTable()

// 	// Expected body
// 	body := gin.H{
// 		"items": []ListItem{
// 			{
// 				Id:   "1",
// 				Item: "Test-API",
// 				Done: false,
// 			},
// 			{
// 				Id:   "2",
// 				Item: "Test-DB",
// 				Done: false,
// 			},
// 		},
// 	}

// 	// /item/create GET request and check 200 OK status code
// 	w1 := performRequest(router, "GET", "/item/create/Test-API")
// 	assert.Equal(t, http.StatusCreated, w1.Code)

// 	// /item/create GET request and check 200 OK status code
// 	w2 := performRequest(router, "GET", "/item/create/Test-DB")
// 	assert.Equal(t, http.StatusCreated, w2.Code)

// 	// /items GET request and check 200 OK status code
// 	w3 := performRequest(router, "GET", "/items")
// 	assert.Equal(t, http.StatusOK, w3.Code)

// 	// Obtain response
// 	var response map[string][]ListItem
// 	err := json.Unmarshal([]byte(w3.Body.String()), &response)
// 	value, exists := response["items"]

// 	// No error in response
// 	assert.Nil(t, err)

// 	// Check if response exits
// 	assert.True(t, exists)

// 	// Assert response
// 	assert.Equal(t, body["items"], value)
// }

// // Test for successfull delete
// // from /item/delete
// func TestItemDelete(t *testing.T) {
// 	// Delete all elements
// 	// from DB
// 	emptyTable()

// 	// Expected body
// 	body := gin.H{
// 		"items": []ListItem{
// 			{
// 				Id:   "2",
// 				Item: "Test-DB",
// 				Done: false,
// 			},
// 		},
// 	}

// 	// /item/create GET request and check 200 OK status code
// 	w1 := performRequest(router, "GET", "/item/create/Test-API")
// 	assert.Equal(t, http.StatusCreated, w1.Code)

// 	// /item/create GET request and check 200 OK status code
// 	w2 := performRequest(router, "GET", "/item/create/Test-DB")
// 	assert.Equal(t, http.StatusCreated, w2.Code)

// 	// /item/delete GET request and check 200 OK status code
// 	w3 := performRequest(router, "GET", "/item/delete/1")
// 	assert.Equal(t, http.StatusOK, w3.Code)

// 	// /items GET request and check 200 OK status code
// 	w4 := performRequest(router, "GET", "/items")
// 	assert.Equal(t, http.StatusOK, w4.Code)

// 	// Obtain response
// 	var response map[string][]ListItem
// 	err := json.Unmarshal([]byte(w4.Body.String()), &response)
// 	value, exists := response["items"]

// 	// No error in response
// 	assert.Nil(t, err)

// 	// Check if response exits
// 	assert.True(t, exists)

// 	// Assert response
// 	assert.Equal(t, body["items"], value)
// }

// // Test for unsuccessfull delete
// // for item that does not exist
// // from /item/delete
// func TestItemDeleteNotPresent(t *testing.T) {
// 	// Delete all elements
// 	// from DB
// 	emptyTable()

// 	// Expected body
// 	body := gin.H{
// 		"message": "not found",
// 	}

// 	// /item/delete GET request and check 404 Not Found status code
// 	w := performRequest(router, "GET", "/item/delete/15")
// 	assert.Equal(t, http.StatusNotFound, w.Code)

// 	// Obtain response
// 	var response map[string]string
// 	err := json.Unmarshal([]byte(w.Body.String()), &response)
// 	value, exists := response["message"]

// 	// No error in response
// 	assert.Nil(t, err)

// 	// Check if response exits
// 	assert.True(t, exists)

// 	// Assert response
// 	assert.Equal(t, body["message"], value)
// }

// // Test for successfull update
// // from /item/update
// func TestItemUpdate(t *testing.T) {
// 	// Delete all elements
// 	// from DB
// 	emptyTable()

// 	// Expected body
// 	body := gin.H{
// 		"items": []ListItem{
// 			{
// 				Id:   "1",
// 				Item: "Test-API",
// 				Done: true,
// 			},
// 		},
// 	}

// 	// /item/create GET request and check 200 OK status code
// 	w1 := performRequest(router, "GET", "/item/create/Test-API")
// 	assert.Equal(t, http.StatusCreated, w1.Code)

// 	// /item/update GET request and check 200 OK status code
// 	w2 := performRequest(router, "GET", "/item/update/1/true")
// 	assert.Equal(t, http.StatusOK, w2.Code)

// 	// /items GET request and check 200 OK status code
// 	w3 := performRequest(router, "GET", "/items")
// 	assert.Equal(t, http.StatusOK, w3.Code)

// 	// Obtain response
// 	var response map[string][]ListItem
// 	err := json.Unmarshal([]byte(w3.Body.String()), &response)
// 	value, exists := response["items"]

// 	// No error in response
// 	assert.Nil(t, err)

// 	// Check if response exits
// 	assert.True(t, exists)

// 	// Assert response
// 	assert.Equal(t, body["items"], value)
// }

// // Test for unsuccessfull update
// // for item that does not exist
// // from /item/update
// func TestItemUpdateNotPresent(t *testing.T) {
// 	// Delete all elements
// 	// from DB
// 	emptyTable()

// 	// Expected body
// 	body := gin.H{
// 		"message": "not found",
// 	}

// 	// /item/update GET request and check 404 Not Found status code
// 	w := performRequest(router, "GET", "/item/update/15/true")
// 	assert.Equal(t, http.StatusNotFound, w.Code)

// 	// Obtain response
// 	var response map[string]string
// 	err := json.Unmarshal([]byte(w.Body.String()), &response)
// 	value, exists := response["message"]

// 	// No error in response
// 	assert.Nil(t, err)

// 	// Check if response exits
// 	assert.True(t, exists)

// 	// Assert response
// 	assert.Equal(t, body["message"], value)
// }
