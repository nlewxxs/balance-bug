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
              <th>Bug Name</th>
              <th>Session Name</th>
              <th>ID</th>
              <th>Date Added</th>
            </tr>
          </thead>
          {sessions && 
          <tbody> 
            {sessions
            .sort((a, b) => new Date(b.TimeStamp) - new Date(a.TimeStamp)) 
            .map(session => (
              <tr className="session_row" key={session.SessionId} onClick={() => handleClick(session.BugName, session.SessionId)}>
                  <td>{ session.SessionName }</td>
                  <td>{ session.BugName }</td>
                  <td>{ session.SessionId.substring(0,8) }</td>
                  <td className="last">{ session.TimeStamp }</td>
                </tr>
          ))}
          </tbody>}
          </table>
      </div>
    )
}

export default SessionList; 
