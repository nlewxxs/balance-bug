import { useState } from "react";
import { useNavigate } from "react-router-dom";

const SessionList = (/*{ sessions }*/) => {
  const [sessions, setSessions] = useState([
    {name: "bug1", id: "123", date: "2023-05-10 18:00:00.00"},
    {name: "bug2", id: "403", date: "2023-05-11 18:00:00.00"},
    {name: "bug2", id: "403", date: "2023-05-11 18:00:00.00"},
    {name: "bug3", id: "101", date: "2023-05-12 18:00:00.00"}
  ]);
  const navigate = useNavigate();
  
  const handleClick = (id) => {

    navigate(`/Display/`+id);
  };

  return (
      <div className="session_list">
        <table>
          <thead>
            <tr className="session_col_names">
              <th>Name</th>
              <th>ID</th>
              <th>Date Created</th>
            </tr>
          </thead>
          <tbody> 
            {sessions.map(session => (
              <tr className="session_row" key={session.id} onClick={() => handleClick(session.id)}>
                  <td>{ session.name }</td>
                  <td>{ session.id }</td>
                  <td>{ session.date }</td>
                </tr>
          ))}
          </tbody>
          </table>
      </div>
    )
}

export default SessionList; 
