package Api

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
		fmt.Println("Error: ", err)
		panic("Error with dB for Edges")
	}
}


func TestCreateEdgeTable(t *testing.T) {
	performRequest(router, "GET", "/Nodes/CreateTable?SessionId=EdgeTest")
	// Delete all elements
	// from DB
	//emptySessionListTable()

	// Expected body
	body := gin.H{
		"message": "successfully created new table",
	}

	// /items GET request and check 200 OK status code
	w := performRequest(router, "GET", "/Edges/CreateTable?SessionId=EdgeTest")
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
	emptyEdgeTable("EdgeTest")

	// /items GET request and check 200 OK status code
	w := performRequest(router, "GET", "/Edges/DisplayAll?SessionId=EdgeTest")
	
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
		NodeId: "Node3",
		EdgeNodeId: "Node4",
		Distance: "5",
		Angle: "30",
	}

	// /items GET request and check 200 OK status code
	

	emptyEdgeTable("EdgeTest")
	performRequest(router, "GET", "/Nodes/Add?SessionId=EdgeTest&NodeId=Node3&XCoord=10&YCoord=0")
	performRequest(router, "GET", "/Nodes/Add?SessionId=EdgeTest&NodeId=Node4&XCoord=20&YCoord=0")



	w := performRequest(router, "GET", "/Edges/Add?SessionId=EdgeTest&NodeId=Node3&EdgeNodeId=Node4&Distance=5&Angle=30")
	assert.Equal(t, http.StatusCreated, w.Code)

	// Obtain response
	var response EdgeStruct
	err := json.Unmarshal([]byte(w.Body.String()), &response)
	
	exists := false
	if ((len(response.NodeId) > 0) && (len(response.EdgeNodeId) > 0) && (len(response.Distance) > 0) && (len(response.Angle) > 0)) {
		exists = true 
	}

	// No error in response
	assert.Nil(t, err)

	// Check if response exits
	assert.True(t, exists)

	// Assert response
	assert.Equal(t, expected, response)
}