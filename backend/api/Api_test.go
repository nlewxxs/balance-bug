package api

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
	router.GET("/Session/Create", CreateSession)

	router.GET("/Nodes/CreateTable", CreateNodeTable)
	router.GET("/Nodes/DisplayAll", DisplayAllNodes)
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

