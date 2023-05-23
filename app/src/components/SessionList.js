import { useNavigate } from "react-router-dom";

const SessionList = (props) => {
  const sessions=props.sessions;

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
