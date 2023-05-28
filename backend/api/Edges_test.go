package api

import (
	"encoding/json"
	"net/http"
	"testing"

	"github.com/stretchr/testify/assert"

	"github.com/gin-gonic/gin"
)

// func emptySessionListTable(SessionId string) {
// 	SqlCommand := fmt.Sprintf("DELETE from testdb.SessionList_nodes;", SessionId)
// 	db.Exec(SqlCommand)
// }


func TestCreateEdgeTable(t *testing.T) {
	// Delete all elements
	// from DB
	//emptySessionListTable()

	// Expected body
	body := gin.H{
		"message": "successfully created new table",
	}

	// /items GET request and check 200 OK status code
	w := performRequest(router, "GET", "/Nodes/CreateTable?SessionId=Test123")
	assert.Equal(t, http.StatusOK, w.Code)

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