package DatabaseApi

import (
	"encoding/json"
	"net/http"
	"testing"

	"github.com/stretchr/testify/assert"

)

// Delete all elements
// from DB
func emptySessionListTable() {
	db.Exec("DELETE from testdb.SessionList;")

	// Reset id counter
	//db.Exec("ALTER SEQUENCE list_id_seq RESTART WITH 1;")
}

// Test for successfull GET
// response from /items
// with no elements
func TestDisplaySessionList(t *testing.T) {
	// clear db table
	emptySessionListTable()

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
func TestAddSession(t *testing.T) {
	// Delete all elements
	// from DB
	emptySessionListTable()

	// Expected body
	expected := SessionListStruct{
			TimeStamp: "2006-01-02 15:04:05",
			BugName: "2",
			SessionId: "",
	}

	// /item/create GET request and check 200 OK status code
	w := performRequest(router, "GET", "/Session/Add?TimeStamp=2006-01-02 15:04:05&BugName=2")
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
