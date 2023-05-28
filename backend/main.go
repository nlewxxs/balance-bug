package main

import (
	// "BalanceBugServer/backend/api"
	"net/http"

	api "BalanceBugServer/backend/api"

	"github.com/gin-gonic/contrib/cors"
	"github.com/gin-gonic/gin"
)

// Function called for index
func indexView(c *gin.Context) {
	c.Header("Access-Control-Allow-Origin", "*")
	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
	c.JSON(http.StatusOK, gin.H{"message": "Balance Bug API"})
}

// Setup Gin Routes
func SetupRoutes() *gin.Engine {
	// Use Gin as router
	router := gin.Default()
	config := cors.DefaultConfig()
	config.AllowAllOrigins = true
	router.Use(cors.New(config))

	// Set route for index
	router.GET("/", indexView)

	// Set routes for API
	// Update to POST, UPDATE, DELETE etc
	
	
	//SessionList Paths
	router.GET("/Session/Add", api.AddSession)
	router.GET("/Session/DisplayAll", api.DisplaySessionList)



	//Node Paths
	router.GET("/Nodes/CreateTable", api.CreateNodeTable)
	router.GET("/Nodes/DisplayAll", api.DisplayAllNodes)

	//Edge Paths
	router.Get("/Edges/Add", api.CreateEdgeTable)
	router.Get("/Edges/CreateTable", api.CreateEdgeTable)
	router.Get("/Edges/DisplayAll", api.DisplayAllEdges)
	
	/*
	SessionId.Edges
		- Foreign keys 
			- Node             - PRIMARY1
			- Connecting Node  - PRIMARY2
		- Distance
		- Angle
	*/

	// router.GET("/item/update/:id/:done", api.UpdateTodoItem)
	// router.GET("/item/delete/:id", api.DeleteTodoItem)

	// Set up Gin Server
	return router
}

// Main function
func main() {
	api.SetupMySQL()
	router := SetupRoutes()
	router.Run(":8081")
}
