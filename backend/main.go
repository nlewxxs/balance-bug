package main

import (
	// "BalanceBugServer/backend/api"
	"net/http"

	api "BalanceBugServer/backend/Api"
	// "BalanceBugServer/backend/api/Databaseapi"
	// // api "BalanceBugServer/backend/api"

	//"github.com/gin-gonic/contrib/cors"
	"github.com/gin-gonic/gin"
	"github.com/gin-contrib/cors"
)

// Function called for index
func indexView(c *gin.Context) {
	c.Header("Access-Control-Allow-Origin", "*")
	c.Header("Access-Control-Allow-Headers", "access-control-allow-origin, access-control-allow-headers")
	c.JSON(http.StatusOK, gin.H{"message": "Balance Bug api"})
}

// Setup Gin Routes
func SetupRoutes() *gin.Engine {
	// Use Gin as router
	router := gin.Default()
	config := cors.DefaultConfig()
	config.AllowAllOrigins = true
	config.AllowMethods = []string{"Get","POST","PUT","PATCH","DELETE","OPTIONS"}
	router.Use(cors.New(config))
	// router.Use(cors.New(cors.Config{
	// 	AllowAllOrigins: true,
	// 	AllowMethods: []string{"Get","POST","PUT","PATCH","DELETE","OPTIONS"},
	// 	AllowHeaders: []string{"Origin"},
	// 	ExposeHeaders: []string{"Content-Length"},
	// 	AllowCredentials: true,
	// 	MaxAge:12 * time.Hour,
	// }))

	// Set route for index
	router.GET("/", indexView)

	// Set routes for api
	// Update to POST, UPDATE, DELETE etc

	//api Paths
	router.GET("/Session/Add", api.AddSession)
	router.PATCH("/Session/Ping", api.PingSession)
	router.GET("/Session/DisplayAll", api.DisplaySessionList)

	router.GET("/Nodes/Add", api.AddNode)
	router.GET("/Nodes/DisplayAll", api.DisplayAllNodes)
	router.GET("/Nodes/CreateTable", api.CreateNodeTable)

	router.GET("/Edges/Add", api.AddEdge)
	router.GET("/Edges/DisplayAll", api.DisplayAllEdges)
	router.GET("/Edges/CreateTable", api.CreateEdgeTable)

	router.GET("/BugInformation/Add", api.AddBugInformation)
	router.GET("/BugInformation/Ping", api.PingBugInformation)
	router.GET("/BugInformation/Online", api.OnlineBugInformation)
	router.GET("/BugInformation/DisplayAll", api.DisplayBugInformation)
	router.GET("/BugInformation/UpdateBugName", api.UpdateBugNameBugInformation)

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
