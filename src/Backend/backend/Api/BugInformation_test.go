package Api

//imports
import (
	"encoding/json"
	"net/http"
	"testing"

	"github.com/stretchr/testify/assert"

)

//empty db
func emptyBugInformationTable() {
	db.Exec("DELETE from testdb.BugInformation;")
}

//test displaying the bug information
func TestDisplayBugInformation(t *testing.T) {
	// clear db table
	emptyBugInformationTable()

	//items GET request and check 200 OK status code
	w := performRequest(router, "GET", "/BugInformation/DisplayAll")
	
	//check for response from server, and that response is ok
	assert.Equal(t, http.StatusOK, w.Code)

	//obtain and process response
	var response []BugInformationStruct
	err := json.Unmarshal([]byte(w.Body.String()), &response)

	//no error in response
	assert.Nil(t, err)

	//assert response with expected response
	assert.Equal(t, []BugInformationStruct{}, response)
}

//test successful create
func TestAddBugInformation(t *testing.T) {
	// clear db table
	emptySessionListTable()

	//expected
	expected := BugInformationStruct{
			BugId: "TestId",
			BugName: "TestBug",
			LastSeen: "",
	}

	//check for response from server, and that response is ok
	w := performRequest(router, "GET", "/BugInformation/Add?BugId=TestId&BugName=TestBug")
	assert.Equal(t, http.StatusCreated, w.Code)

	//response
	var response BugInformationStruct
	err := json.Unmarshal([]byte(w.Body.String()), &response)
	
	//check if response is valid
	exists := false
	if ((len(response.BugId) > 0) && (len(response.BugName) > 0) && (len(response.LastSeen) > 0)) {
		exists = true 
	}

	//no error in response
	assert.Nil(t, err)

	//assert if response exists
	assert.True(t, exists)

	//assert response
	assert.Equal(t, expected.BugName, response.BugName)
	assert.Equal(t, expected.BugId, response.BugId)
}


func TestPingBugInformation(t *testing.T) {
	//expected
	expected := BugInformationStruct{
			BugId: "TestId",
			BugName: "TestBug",
			LastSeen: "",
	}

	//check for response from server, and that response is ok
	w := performRequest(router, "GET", "/BugInformation/Ping?BugId=TestId")
	assert.Equal(t, http.StatusOK, w.Code)

	//response
	var response BugInformationStruct
	err := json.Unmarshal([]byte(w.Body.String()), &response)
	
	//check if response is valid
	exists := false
	if ((len(response.BugId) > 0) && (len(response.LastSeen) > 0)) {
		exists = true 
	}

	//assert no error
	assert.Nil(t, err)

	//assert exists
	assert.True(t, exists)

	//assert response as expected
	assert.Equal(t, expected.BugId, response.BugId)
}


func TestUpdateBugNameBugInformation(t *testing.T) {
	//expected
	expected := BugInformationStruct{
			BugId: "TestId",
			BugName: "ChangedBugName",
			LastSeen: "",
	}

	//check for response from server, and that response is ok
	w := performRequest(router, "GET", "/BugInformation/UpdateBugName?BugId=TestId&BugName=ChangedBugName")
	assert.Equal(t, http.StatusOK, w.Code)

	//response
	var response BugInformationStruct
	err := json.Unmarshal([]byte(w.Body.String()), &response)
	
	exists := false
	if ((len(response.BugId) > 0) && (len(response.BugName) > 0)) {
		exists = true 
	}

	//assert no error
	assert.Nil(t, err)

	//assert exists
	assert.True(t, exists)

	//assert response is expected
	assert.Equal(t, expected.BugId, response.BugId)
	assert.Equal(t, expected.BugName, response.BugName)
}


func TestOnlineBugInformation(t *testing.T) {
	//assert expected
	emptyBugInformationTable()
	
	//perform request
	performRequest(router, "GET", "/BugInformation/Add?BugId=OnlineTest")

	//expected response
	expected := "OnlineTest"

	//check for response from server, and that response is ok
	w := performRequest(router, "GET", "/BugInformation/Online?Timeout=100.0")
	assert.Equal(t, http.StatusOK, w.Code)

	//response
	var response []string
	err := json.Unmarshal([]byte(w.Body.String()), &response)
	
	//check response exists
	exists := false
	if (len(response) > 0) {
		exists = true 
	}

	//assert no error
	assert.Nil(t, err)

	//assert exists
	assert.True(t, exists)

	//assert response is expected
	assert.Equal(t, expected, response[0])
}