package Api

//imports
import (
	"encoding/json"
	"net/http"
	"testing"
	"fmt"

	"github.com/stretchr/testify/assert"

	"github.com/gin-gonic/gin"
)

//empty the node table
func emptyNodeTable(SessionId string) {
	SqlCommand := fmt.Sprintf("DELETE from testdb.%s_nodes;", SessionId)
	_, err = db.Exec(SqlCommand)
	if err != nil{
		panic("Error with dB for Nodes")
	}
}

//create the node table
func TestCreateNodeTable(t *testing.T) {
	//expected
	body := gin.H{
		"message": "successfully created new table",
	}

	//check for response from server, and that response is ok
	w := performRequest(router, "GET", "/Nodes/CreateTable?SessionId=Test123")
	assert.Equal(t, http.StatusCreated, w.Code)

	//response
	var response map[string]string
	err := json.Unmarshal([]byte(w.Body.String()), &response)
	value, exists := response["message"]

	//assert no error
	assert.Nil(t, err)

	//assert if response exists
	assert.True(t, exists)

	//assert that the response is equal to that of the expected
	assert.Equal(t, body["message"], value)
}

func TestDisplayAllNodes(t *testing.T) {
	//empty node table
	emptyNodeTable("Test123")

	//check for response from server, and that response is ok
	w := performRequest(router, "GET", "/Nodes/DisplayAll?SessionId=Test123")
	assert.Equal(t, http.StatusOK, w.Code)

	//obtain and process response
	var response []NodeStruct
	err := json.Unmarshal([]byte(w.Body.String()), &response)

	//assert no error
	assert.Nil(t, err)

	//assert expected response
	assert.Equal(t, []NodeStruct{}, response)
}

func TestAddNode(t *testing.T) {
	//expected
	expected := NodeStruct{
		NodeId: "Node1",
		XCoord: "0",
		YCoord: "0",
	}

	//empty the nodetable
	emptyNodeTable("Test123")

	//check for response from server, and that response is ok
	w := performRequest(router, "GET", "/Nodes/Add?SessionId=Test123&NodeId=Node1&XCoord=0&YCoord=0")
	assert.Equal(t, http.StatusCreated, w.Code)

	//response
	var response NodeStruct
	err := json.Unmarshal([]byte(w.Body.String()), &response)
	
	//check if response exists
	exists := false
	if ((len(response.NodeId) > 0) && (len(response.XCoord) > 0) && (len(response.YCoord) > 0)) {
		exists = true 
	}

	//assert no error
	assert.Nil(t, err)

	//assert response exists
	assert.True(t, exists)

	//assert equal to expected response
	assert.Equal(t, expected, response)
}


