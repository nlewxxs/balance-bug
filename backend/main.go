package main

import (
	// "BalanceBugServer/backend/api"
	"net/http"
	"time"

	"BalanceBugServer/backend/DatabaseApi"
	api "BalanceBugServer/backend/DatabaseApi"

	//"github.com/gin-gonic/contrib/cors"
	"github.com/gin-gonic/gin"
	"github.com/gin-contrib/cors"
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
	// config.AddAllowMethods = []string{"Get","POST","PUT","PATCH","DELETE","OPTIONS"}
	// router.Use(cors.New(config))
	router.Use(cors.New(cors.Config{
		AllowAllOrigins: true,
		AllowMethods: []string{"Get","POST","PUT","PATCH","DELETE","OPTIONS"},
		AllowHeaders: []string{"Origin"},
		ExposeHeaders: []string{"Content-Length"},
		AllowCredentials: true,
		MaxAge:12 * time.Hour,
	}))

	// Set route for index
	router.GET("/", indexView)

	// Set routes for API
	// Update to POST, UPDATE, DELETE etc

	//DatabaseApi Paths
	router.GET("/Session/Add", DatabaseApi.AddSession)
	router.PATCH("/Session/Ping", DatabaseApi.PingSession)
	router.GET("/Session/DisplayAll", DatabaseApi.DisplaySessionList)

	router.GET("/Nodes/Add", DatabaseApi.AddNode)
	router.GET("/Nodes/DisplayAll", DatabaseApi.DisplayAllNodes)
	router.GET("/Nodes/CreateTable", DatabaseApi.CreateNodeTable)

	router.GET("/Edges/Add", DatabaseApi.AddEdge)
	router.GET("/Edges/DisplayAll", DatabaseApi.DisplayAllEdges)
	router.GET("/Edges/CreateTable", DatabaseApi.CreateEdgeTable)

	router.GET("/BugInformation/Add", DatabaseApi.AddBugInformation)
	router.GET("/BugInformation/Ping", DatabaseApi.PingBugInformation)
	router.GET("/BugInformation/Online", DatabaseApi.OnlineBugInformation)
	router.GET("/BugInformation/DisplayAll", DatabaseApi.DisplayBugInformation)
	router.GET("/BugInformation/UpdateBugName", DatabaseApi.UpdateBugNameBugInformation)

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
