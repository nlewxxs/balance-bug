package maze

import (
    "fmt"
)


///////////////////////////////////////////////////////////////////////////////
/////////////////////////      EDGE DATASTRUCTURE      ////////////////////////
///////////////////////////////////////////////////////////////////////////////
type Edge struct {
	target			string
	weight			int
}

//EDGE FUNCTIONS
func (edge *Edge) print_edge() {
	fmt.Println(edge.target, ": ", edge.weight)
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////      MAZE DATASTRUCTURE      ////////////////////////
///////////////////////////////////////////////////////////////////////////////

type Maze struct{
	start			string
	end				string
	m 				map[string] Edge
}

//access
func (maze Maze) get_start() string {
	return maze.start
}

func (maze Maze) get_end() string{
	return maze.end
}

func (maze Maze) get_map() map[string] Edge{
	return maze.m
}

func (maze *Maze) modify_start(start string) {
	maze.start = start
}

func (maze *Maze) modify_end(end string) {
	maze.end = end
}

func (maze *Maze) add_edge(start_node string, end_node string, weight int) {
	var new_edge = Edge{end_node, weight}
	maze.m[start_node] = new_edge
}

func (maze *Maze) print_graph() {
	for start_node, end_node := range maze.m{
		fmt.Print("Entry: ", start_node, ", has edges: ")
		end_node.print_edge()
	}
}

func (maze *Maze) new_maze() {
	maze.start = ""
	maze.end = ""
	maze.m = make(map[string] Edge)
}
