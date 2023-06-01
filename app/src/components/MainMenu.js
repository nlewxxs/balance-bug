import { useState } from 'react';
import { Link } from 'react-router-dom';
import msk from '../Imperial-masked.png';
import imp from '../Imperial.png';
import { useNavigate } from "react-router-dom";



function MainMenu() {
    const [isHover, setHover] = useState(false);
    const navigate = useNavigate();

    const handleHover = () => {
        setHover(!isHover);
    };

    const handleClick = () => {
        window.open("https://www.imperial.ac.uk/students/academic-support/graduate-school/students/masters/professional-development/plagiarism-online/");
    };

    const img = isHover ? msk : imp;
    return (
      <div className="menu_page">
        <h1 className="title">Main Menu</h1>
        <div className="img_container">
          <img src={img}
            onMouseEnter={handleHover}
            onMouseLeave={handleHover}
            onClick={handleClick}
            className="img" alt="imperial"/>
        </div>
        <div className="btn_container">
            <div className="add_btn">
              <button className="btn">
               <Link to="/Add" className="btn_link">New Session</Link>
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
