import { useParams } from "react-router-dom";
import { Link } from "react-router-dom";

function DisplayGraph() {
    const { id } = useParams();

    // make api call

    return (
      <div className="display_page">
        <h1 className="title">GRAPH FILLER</h1>
        <div className="graph">
            <p> {id} </p>
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
      
    )
}

export default DisplayGraph; 
