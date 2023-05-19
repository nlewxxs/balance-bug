package main

import (
	"fmt"
)

//upper case letters at start of name are exported (public) functions
//lower case letters at start of name are equivalent to private functions

// /////////////////////////////////////////////////////////////////////////////
// ///////////////////////      EDGE DATASTRUCTURE      ////////////////////////
// /////////////////////////////////////////////////////////////////////////////
type edge struct {
	target string
	weight int
}

// EDGE FUNCTIONS
func (edge *edge) PrintEdge() {
	fmt.Println(edge.target, ": ", edge.weight)
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////      MAZE DATASTRUCTURE      ////////////////////////
///////////////////////////////////////////////////////////////////////////////

type Maze struct {
	start string
	end   string
	m     map[string]edge
}

// access
func (maze Maze) GetStart() string {
	return maze.start
}

func (maze Maze) GetEnd() string {
	return maze.end
}

func (maze Maze) GetMap() map[string]edge {
	return maze.m
}

func (maze *Maze) ModifyStart(_start string) {
	maze.start = _start
}

func (maze *Maze) ModifyEnd(_end string) {
	maze.end = _end
}

func (maze *Maze) AddEdge(_startnode string, _endnode string, _weight int) {
	var new_edge = edge{_endnode, _weight}
	maze.m[_startnode] = new_edge
}

func (maze *Maze) PrintGraph() {
	for StartNode, EndNode := range maze.m {
		fmt.Print("Entry: ", StartNode, ", has edges: ")
		EndNode.PrintEdge()
	}
}

func (maze *Maze) NewMaze() {
	maze.start = ""
	maze.end = ""
	maze.m = make(map[string]edge)
}
