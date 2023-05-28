package api

import (
	"encoding/json"
	"net/http"
	"testing"
	"fmt"

	"github.com/stretchr/testify/assert"

	"github.com/gin-gonic/gin"
)

// func emptySessionListTable(SessionId string) {
// 	SqlCommand := fmt.Sprintf("DELETE from testdb.SessionList_nodes;", SessionId)
// 	db.Exec(SqlCommand)
// }

func emptyEdgeTable(SessionId string) {
	SqlCommand := fmt.Sprintf("DELETE from testdb.%s_edges;", SessionId)
	_, err = db.Exec(SqlCommand)
	if err != nil{
		panic("Error with dB")
	}
}


func TestCreateEdgeTable(t *testing.T) {
	// Delete all elements
	// from DB
	//emptySessionListTable()

	// Expected body
	body := gin.H{
		"message": "successfully created new table",
	}

	// /items GET request and check 200 OK status code
	w := performRequest(router, "GET", "/Edges/CreateTable?SessionId=Test123")
	assert.Equal(t, http.StatusCreated, w.Code)

	// Obtain response
	var response map[string]string
	err := json.Unmarshal([]byte(w.Body.String()), &response)
	value, exists := response["message"]

	// No error in response
	assert.Nil(t, err)

	// Check if response exits
	assert.True(t, exists)

	// Assert response
	assert.Equal(t, body["message"], value)
}



func TestDisplayAllEdges(t *testing.T) {
	// clear db table
	emptyNodeTable("Test123")

	// /items GET request and check 200 OK status code
	w := performRequest(router, "GET", "/Edges/DisplayAll?SessionId=Test123")
	
	//check for response from server, and that response is ok
	assert.Equal(t, http.StatusOK, w.Code)

	//obtain and process response
	var response []EdgeStruct
	err := json.Unmarshal([]byte(w.Body.String()), &response)

	// No error in response
	assert.Nil(t, err)

	// Assert response is what is expected, in this case empty
	assert.Equal(t, []EdgeStruct{}, response)
}

func TestAddEdge(t *testing.T) {
	// Delete all elements
	// from DB
	//emptySessionListTable()

	// Expected body
	expected := EdgeStruct{
		NodeId: "Node1",
		EdgeNodeId: "Node2",
		Distance: "5",
		Angle: "30",
	}

	emptyNodeTable("Test123")

	// /items GET request and check 200 OK status code
	performRequest(router, "GET", "/Nodes/Add?SessionId=Test123&NodeId=Node2&XCoord=20&YCoord=0")
	w := performRequest(router, "GET", "/Edges/Add?SessionId=Test123&NodeId=Node1&EdgeNodeId=Node2&Distance=5&Angle=30")
	assert.Equal(t, http.StatusCreated, w.Code)

	// Obtain response
	var response NodeStruct
	err := json.Unmarshal([]byte(w.Body.String()), &response)
	
	exists := false
	if ((len(response.NodeId) > 0) && (len(response.XCoord) > 0) && (len(response.YCoord) > 0)) {
		exists = true 
	}

	// No error in response
	assert.Nil(t, err)

	// Check if response exits
	assert.True(t, exists)

	// Assert response
	assert.Equal(t, expected, response)
}