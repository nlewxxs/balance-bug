import { Link } from 'react-router-dom';

function MainMenu() {
    return (
      <div className="menu_page">
        <h1 className="title">Main Menu</h1>
        <div className="btn_container">
            <div className="create_btn">
              <button className="btn">
               <Link to="/Create" className="btn_link">Create Session</Link> 
              </button>
            </div>
            <div className="load_btn">
              <button className="btn">
                <Link to={"/Load"} className="btn_link">Load Session</Link>
              </button>
            </div>
        </div>
      </div>
    )
}

export default MainMenu; 
