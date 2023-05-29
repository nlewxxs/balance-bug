package DatabaseApi

import (
	"encoding/json"
	"net/http"
	"testing"

	"github.com/stretchr/testify/assert"

)

// Delete all elements
// from DB
func emptyBugInformationTable() {
	db.Exec("DELETE from testdb.BugInformation;")
}

func TestDisplayBugInformation(t *testing.T) {
	// clear db table
	emptyBugInformationTable()

	// /items GET request and check 200 OK status code
	w := performRequest(router, "GET", "/BugInformation/DisplayAll")
	
	//check for response from server, and that response is ok
	assert.Equal(t, http.StatusOK, w.Code)

	//obtain and process response
	var response []BugInformationStruct
	err := json.Unmarshal([]byte(w.Body.String()), &response)

	// No error in response
	assert.Nil(t, err)

	// Assert response is what is expected, in this case empty
	assert.Equal(t, []BugInformationStruct{}, response)
}

// // Test for successfull create
// // response from /item/create
func TestAddBugInformation(t *testing.T) {
	// Delete all elements
	// from DB
	emptySessionListTable()

	// Expected body
	expected := BugInformationStruct{
			BugId: "TestId",
			BugName: "TestBug",
			LastSeen: "",
	}

	// /item/create GET request and check 200 OK status code
	w := performRequest(router, "GET", "/BugInformation/Add?BugId=TestId&BugName=TestBug")
	assert.Equal(t, http.StatusCreated, w.Code)

	// Obtain response
	var response BugInformationStruct
	err := json.Unmarshal([]byte(w.Body.String()), &response)
	
	exists := false
	if ((len(response.BugId) > 0) && (len(response.BugName) > 0) && (len(response.LastSeen) > 0)) {
		exists = true 
	}

	// No error in response
	assert.Nil(t, err)

	// Check if response exits
	assert.True(t, exists)

	// Assert response
	assert.Equal(t, expected.BugName, response.BugName)
	assert.Equal(t, expected.BugId, response.BugId)
}


func TestPingBugInformation(t *testing.T) {
	// Delete all elements
	// from DB
	//emptySessionListTable()

	// Expected body
	expected := BugInformationStruct{
			BugId: "TestId",
			BugName: "TestBug",
			LastSeen: "",
	}

	// /item/create GET request and check 200 OK status code
	w := performRequest(router, "GET", "/BugInformation/Ping?BugId=TestId")
	assert.Equal(t, http.StatusOK, w.Code)

	// Obtain response
	var response BugInformationStruct
	err := json.Unmarshal([]byte(w.Body.String()), &response)
	
	exists := false
	if ((len(response.BugId) > 0) && (len(response.LastSeen) > 0)) {
		exists = true 
	}

	// No error in response
	assert.Nil(t, err)

	// Check if response exits
	assert.True(t, exists)

	// Assert response
	assert.Equal(t, expected.BugId, response.BugId)
}
