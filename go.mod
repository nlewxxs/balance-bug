module main

require golang.org/x/sys v0.0.0-20201015000850-e3ed0017c211 // indirect

require (
	//maze_pkg v1.0.0
	github.com/julienschmidt/httprouter v1.3.0
	github.com/kardianos/service v1.2.0

)

//replace maze_pkg v1.0.0 => ./include/maze
go 1.20
