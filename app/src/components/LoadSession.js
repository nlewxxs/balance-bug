import React, { useState ,useEffect } from 'react';
import { Link } from 'react-router-dom';
import SessionList from './SessionList';
import fetchData from './FetchData.js';

function LoadSession() {
    const [sessions, setSessions] = useState(null);
    const [isLoading, setLoading] = useState(true);
//    {name: "bug1", id: "123", date: "2023-05-10 18:00:00.00"},
//    {name: "bug2", id: "403", date: "2023-05-11 18:00:00.00"},
//    {name: "bug3", id: "101", date: "2023-05-12 18:00:00.00"}
//    ]);

    const url = "http://localhost:8081/Session/DisplayAll"
    useEffect( () => {
        setLoading(true);
        fetchData(url)
        .then(response => {
            setSessions(response.data);
            setTimeout(() => {setLoading(response.isLoading); }, 300);
        })
        .catch(error => console.log(error));
    }, [] );

    return (
    <div>
    { isLoading ? (
        <div> Loading... </div>
    ) : (
    <div className="selection">
      <h1>Select Session</h1>
      {<SessionList sessions={sessions} />}
      <button className="create_btn">
          <Link to="/Create">Create New Session</Link>
      </button>
      <button className="back_btn">
          <Link to="/">Back to Menu</Link>
      </button>
    </div>
    )}
    </div>
    )
}

export default LoadSession;
