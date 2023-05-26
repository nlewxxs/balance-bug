import { useNavigate } from "react-router-dom";

const SessionList = (props) => {
  const sessions=props.sessions;

  const navigate = useNavigate();
  
  const handleClick = (name, id) => {
    navigate(`/Display/`+ name + "?=" + id);
  };
  
  console.log("Sessions", sessions);

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
          {sessions && 
          <tbody> 
            {sessions.map(session => (
              <tr className="session_row" key={session.SessionKey} onClick={() => handleClick(session.BugId, session.SessionKey)}>
                  <td>{ session.BugId }</td>
                  <td>{ session.SessionKey }</td>
                  <td>{ session.ConnTime }</td>
                </tr>
          ))}
          </tbody>}
          </table>
      </div>
    )
}

export default SessionList; 
