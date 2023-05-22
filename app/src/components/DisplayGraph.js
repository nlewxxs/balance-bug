import { useParams } from "react-router-dom";
import { Link } from "react-router-dom";

function DisplayGraph() {
    const { id } = useParams();
    return (
      <div className="GraphPage">
        <h1>GRAPH FILLER</h1>
        <p> {id} </p>
        <button className="load_btn">
          <Link to="/Load">Change Session</Link> 
        </button>
        <button className="back_btn">
           <Link to="/">Back to Menu</Link> 
        </button>
      </div>
      
    )
}

export default DisplayGraph; 
