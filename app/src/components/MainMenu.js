import { Link } from 'react-router-dom';

function MainMenu() {
    return (
      <div className="Menu">
        <h1>Main Menu</h1>
        <button className="create_btn">
           <Link to="/Create">Create New Session</Link> 
        </button>
        <button className="load_btn">
           <Link to="/Load">Load Previous Session</Link> 
        </button>
      </div>
    )
}

export default MainMenu; 
