package Api

//imports
import (
	"encoding/json"
	"net/http"
	"testing"

	"github.com/stretchr/testify/assert"

)

//empty SessionList Table
func emptySessionListTable() {
	db.Exec("DELETE from testdb.SessionList;")
}

//test for DisplaySession
func TestDisplaySessionList(t *testing.T) {
	//empty db table
	emptySessionListTable()

	//check for response from server, and that response is ok
	w := performRequest(router, "GET", "/Session/DisplayAll")
	assert.Equal(t, http.StatusOK, w.Code)

	//obtain + process response
	var response []SessionListStruct
	err := json.Unmarshal([]byte(w.Body.String()), &response)
	value := response

	//assert no error in response
	assert.Nil(t, err)

	//assert response equal to what is expected
	assert.Equal(t, []SessionListStruct{}, value)
}

//test successful add session
func TestAddSession(t *testing.T) {
	//empty SessionList
	emptySessionListTable()

	//expected
	expected := SessionListStruct{
			TimeStamp: "2006-01-02 15:04:05",
			BugName: "2",
			SessionId: "",
	}

	//check for response from server, and that response is ok
	w := performRequest(router, "GET", "/Session/Add?TimeStamp=2006-01-02 15:04:05&BugName=2")
	assert.Equal(t, http.StatusCreated, w.Code)

	//response
	var response SessionListStruct
	err := json.Unmarshal([]byte(w.Body.String()), &response)
	
	//check response exists
	exists := false
	if ((len(response.TimeStamp) > 0) && (len(response.BugName) > 0)) {
		exists = true 
	}

	//assert no error in response
	assert.Nil(t, err)

	//assert response exists
	assert.True(t, exists)

	//assert response is what is expected
	assert.Equal(t, expected.TimeStamp, response.TimeStamp)
	assert.Equal(t, expected.BugName, response.BugName)
}
