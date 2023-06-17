package main

import (
	// "BalanceBugServer/backend/api"
	"net/http"

	api "BalanceBugServer/backend/Api"
	// "BalanceBugServer/backend/api/Databaseapi"
	// // api "BalanceBugServer/backend/api"

	//"github.com/gin-gonic/contrib/cors"
	"github.com/gin-contrib/cors"
	"github.com/gin-gonic/gin"
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
	// gin.SetMode(gin.releaseMode)
	router := gin.Default()
	config := cors.DefaultConfig()
	config.AllowAllOrigins = true
	config.AllowMethods = []string{"Get", "POST", "PUT", "PATCH", "DELETE", "OPTIONS"}
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
	//api Paths
	router.PUT("/Session/Add", api.AddSession)
	router.PATCH("/Session/Ping", api.PingSession)
	router.GET("/Session/DisplayAll", api.DisplaySessionList)

	router.PUT("/Nodes/Add", api.AddNode)
	router.GET("/Nodes/DisplayAll", api.DisplayAllNodes)
	router.PUT("/Nodes/CreateTable", api.CreateNodeTable)

	router.PUT("/Edges/Add", api.AddEdge)
	router.GET("/Edges/DisplayAll", api.DisplayAllEdges)
	router.PUT("/Edges/CreateTable", api.CreateEdgeTable)

	router.PATCH("/BugInformation/Ping", api.PingBugInformation)
	router.GET("/BugInformation/Online", api.OnlineBugInformation)
	router.GET("/BugInformation/DisplayAll", api.DisplayBugInformation)
	router.PATCH("/BugInformation/UpdateBugName", api.UpdateBugNameBugInformation)

	router.PATCH("/Beacon/TurnOn", api.TurnOn)
	router.PATCH("/Beacon/BeaconPing", api.BeaconPing)

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
