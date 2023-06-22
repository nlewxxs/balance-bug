package main

//imports
import (
	"net/http"

	api "BalanceBugServer/backend/Api"

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
	//setup router
	router := gin.Default()
	config := cors.DefaultConfig()

	//create custom config to allow all standard types of requests
	config.AllowAllOrigins = true
	config.AllowMethods = []string{"Get", "POST", "PUT", "PATCH", "DELETE", "OPTIONS"}
	router.Use(cors.New(config))

	// Set main route, used for checking connection
	router.GET("/", indexView)

	// Set routes for api
	//api Paths

	//Sessions
	router.GET("/Session/Add", api.AddSession)
	router.PATCH("/Session/Ping", api.PingSession)
	router.GET("/Session/DisplayAll", api.DisplaySessionList)
	router.GET("/Session/CheckNewSession", api.CheckNewSession)

	//Nodes
	router.PUT("/Nodes/Add", api.AddNode)
	router.GET("/Nodes/DisplayAll", api.DisplayAllNodes)
	router.PUT("/Nodes/CreateTable", api.CreateNodeTable)

	//Edges
	router.PUT("/Edges/Add", api.AddEdge)
	router.GET("/Edges/DisplayAll", api.DisplayAllEdges)
	router.PUT("/Edges/CreateTable", api.CreateEdgeTable)

	//BugInformation
	router.PATCH("/BugInformation/Ping", api.PingBugInformation)
	router.GET("/BugInformation/Online", api.OnlineBugInformation)
	router.GET("/BugInformation/DisplayAll", api.DisplayBugInformation)
	router.PATCH("/BugInformation/UpdateBugName", api.UpdateBugNameBugInformation)

	//Beacon System
	router.PATCH("/Beacon/TurnOn", api.TurnOn)
	router.PATCH("/Beacon/BeaconPing", api.BeaconPing)

	// Set up Gin Server
	return router
}

// Main function
func main() {
	//run the api
	api.SetupMySQL()
	router := SetupRoutes()
	router.Run(":8081")
}
