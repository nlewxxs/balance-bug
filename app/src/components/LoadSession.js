import React, { useState, useEffect } from 'react';
import { Link } from 'react-router-dom';
import SessionList from './SessionList';
import {fetchData, usePoll} from './Utils.js';

function LoadSession() {
    const [sessions, setSessions] = useState(null);
    const [isLoading, setLoading] = useState(true);
    const [error, setError] = useState(null);

    useEffect( () => {
        setLoading(true);
        getSessions();
    }, [] );

//    usePoll( () => {
//        getSessions();  
//    }, 1000); 
    
    const url = "http://192.168.68.126:8081/Session/DisplayAll"
    const getSessions = () => {
        fetchData(url)
        .then(response => {
            console.log(response);
            setSessions(response.data);
            setError(response.error);
            if (!response.data && !response.error) {
                setError("No sessions are currently listed");
            }
            setLoading(response.isLoading);
            // setTimeout(() => {setLoading(response.isLoading); }, 300);
        })
        .catch(error => console.log(error));
    }

    return (
        <div>
        { isLoading ? (
            <div> Loading... </div>
        ) : (
        <div className="load_page">
          <h1 className="title">Select Session</h1>
          { (!sessions || error) ? (
                <table className="empty">{error}</table>
          ) : (
            <div>
            {<SessionList sessions={sessions} />}
            </div>
          )}
            <div className="btn_container">
              <div className="add_btn">
                  <button className="btn">
                      <Link to="/Add" className="btn_link">New Session</Link>
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

export default LoadSession;
