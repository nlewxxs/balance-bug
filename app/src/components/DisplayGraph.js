import React, { useState, useEffect } from 'react';
import { useParams, useLocation } from "react-router-dom";
import { Link } from "react-router-dom";
import {fetchData, usePoll} from './Utils.js';
import Graph from 'react-graph-vis';

// function generateGraph(nodesIn, edgesIn) {
//   const graph = {
//     nodes: nodesIn,
//     edges: edgesIn,
//   };
  
//   return graph ;
// };


function DisplayGraph() {
    const [nodes, setNodes] = useState(null);
    const [edges, setEdges] = useState(null);
    const [nodesError, setNodesError] = useState(null);
    const [edgesError, setEdgesError] = useState(null);
    const {id} = useParams();
    const sessionIdString = id.toString();
    // const location = useLocation();
    const [isLoading, setLoading] = useState(true); 
    // const [graph, setGraph] = useState(null);
    




    const urlNodes = "http://90.196.3.86:8081/Nodes/DisplayAll?SessionId=" + sessionIdString;

    // const processNodes = () => {
    //   setProcessedNodesJson([{ id: 1, label: '', x:0, y:0 },
    //     { id: 2, label: '', x:10, y: 15},]);
    // }

    // const processEdges = () => {
    //   setProcessedEdgesJson([
    //     { from: 1, to: 2 },
    //   ]);
    // }

    

    const getNodes = () => {
      fetchData(urlNodes)
      .then(response => {
          console.log(response);
          setNodes(response.data);
          setNodesError(response.error);
          if (!response.data && !response.error) {
              setNodesError("No sessions are currently listed");
          }
          // setLoading(response.isLoading);
          // setTimeout(() => {setLoading(response.isLoading); }, 300);
      })
      .catch(error => console.log(error));
    }

    const urlEdges = "http://90.196.3.86:8081/Edges/DisplayAll?SessionId=" + sessionIdString;

    const getEdges = () => {
      fetchData(urlEdges)
      .then(response => {
          console.log(response);
          setEdges(response.data);
          setEdgesError(response.error);
          if (!response.data && !response.error) {
              setEdgesError("No sessions are currently listed");
          }
          // setLoading(response.isLoading);
          // setTimeout(() => {setLoading(response.isLoading); }, 300);
      })
      .catch(error => console.log(error));
    }
    useEffect( () => {
        setNodes([{ id: 0, label: '', x:0, y: 0},]);
        setEdges([]);
        // getNodes();
        // getEdges();
        // setGraph(generateGraph(nodes, edges));
    }, [] );
    
    usePoll( () => {
      getNodes();
      getEdges(); 
      // setGraph(generateGraph(nodes, edges));
    }, 1000); 

    // const [searchParams, setSearchParams] = useSearchParams();
    // const {name} = searchParams.get("id")

    // make api call
    useEffect( () => { 
        setTimeout(() => { setLoading(false); }, 300); 
    }, []);

    // const processNodes = () => {
    //   setProcessedNodesJson([]);
    // }
    // for (let i = 0; i < Object.keys(setNodes).length; i++) {
      
    // }

    // const processEdges = () => {
    //   setProcessedEdgesJson([
    //     { from: 1, to: 2 },
    //   ]);
    // }

    const options = {
      layout: {
        hierarchical: false,
      },
      edges: {
        color: '#000000',
      },
      physics: false,
      interaction: {
        selectable: false, // Disable node and edge selection
        dragNodes: false, // Disable node dragging
        dragView: false, // Disable panning and zooming
        zoomView: false, // Disable zooming
      },
    };
  
    const events = {};

    const graph = {
      nodes: nodes,
      edges: edges,
    };
    
    return (
      <div>
      {isLoading ? ( <div> Loading... </div>
      )  : (
      <div className="display_page">
        <h1 className="title">GRAPH FILLER</h1>
        <div className="graph">
          <Graph graph={graph} options={options} events={events}/>
            {/* Display Graph Info */}
        </div>
        <div className="btn_container">
            <div className="load_btn">
              <button className="btn">
                <Link to={"/Load"} className="btn_link">Load Session</Link>
              </button>
            </div>
            <div className="back_btn">
              <button className="btn">
                <Link to="/" className="btn_link">Back to Menu</Link> 
              </button>
            </div>
        </div>
        </div>
      )}
      </div>
      
    )
}

export default DisplayGraph; 
