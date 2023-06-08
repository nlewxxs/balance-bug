package Api

import (
	"net/http"
	"net/http/httptest"
	"testing"


	"github.com/gin-gonic/contrib/cors"
	"github.com/gin-gonic/gin"
)

var router *gin.Engine


// Setup Gin Routes
func SetupRoutes() *gin.Engine {
	// Use Gin as router
	router := gin.Default()
	config := cors.DefaultConfig()
	config.AllowAllOrigins = true
	router.Use(cors.New(config))

	// Set routes for API
	router.GET("/Session/DisplayAll", DisplaySessionList)
	router.GET("/Session/Add", AddSession)

	router.GET("/Nodes/CreateTable", CreateNodeTable)
	router.GET("/Nodes/Add", AddNode)
	router.GET("/Nodes/DisplayAll", DisplayAllNodes)

	router.GET("/Edges/CreateTable", CreateEdgeTable)
	router.GET("/Edges/Add", AddEdge)
	router.GET("/Edges/DisplayAll", DisplayAllEdges)
	
	router.GET("/BugInformation/Add", AddBugInformation)
	router.GET("/BugInformation/Ping", PingBugInformation)
	router.GET("/BugInformation/UpdateBugName", UpdateBugNameBugInformation)
	router.GET("/BugInformation/Online", OnlineBugInformation)
	router.GET("/BugInformation/DisplayAll", DisplayBugInformation)

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

