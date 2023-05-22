import React from 'react';
import { Link } from 'react-router-dom';
import SessionList from './SessionList';

function MainMenu() {
    return (
      <div className="selection">  
        <h1>Select Session</h1>        
        <SessionList />
        <button className="create_btn">
            <Link to="/Create">Create New Session</Link> 
        </button>
        <button className="back_btn">
            <Link to="/">Back to Menu</Link> 
        </button>
      </div>
    )
}

export default MainMenu; 
