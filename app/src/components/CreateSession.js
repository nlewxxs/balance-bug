import React, { useState } from "react";
import { Link, useNavigate } from "react-router-dom";
import fetchData from './FetchData';

function CreateSession() {
    const [errorMsgs, setErrorMsgs] = useState({ err: "", msg: ""});
    const [sessionName, setSessionName] = useState("");
    const [isLoading, setLoading] = useState(false);
    const navigate = useNavigate();
    const errors = {
        taken: "Name is currently in use",
        empty: "Please enter a name",
        fetch: "There was an error connecting to the database"
    };

    const checkIfTaken = () => {
        // Realistically won't need this due to the id/SessionKey

        // Make api call
        //  fetchData(`api/isnametaken?name=${sessionName}`)
        //  .then(response => {
        //      const data = response.data;
        //      if (data.isTaken) {
        //          setErrorMsgs( {
        //              err: "taken",
        //              msg: errors.taken
        //          });
        //          return true;
        //      }
        //  });

        return false;
    };

    const navigateToPath = (name, id) => {
        const path = "/Display/" + name + "?=" + id;
        // console.log(path);
        navigate(path);
    }

    const handleSubmit = (event) => {
        // Prevent page reloading
        event.preventDefault();
        // check if empty
        if (sessionName.trim() === "") {
            setErrorMsgs( { err: "empty", msg: errors.empty });
            return;
        }
        // check if taken
        if (checkIfTaken()) { return; }
        // reset error msg
        else { setErrorMsgs( { err: "", msg: "" } ); }

        // get date & time of creation
        const currentDate = new Date();
        let dateTime = currentDate.toISOString();
        dateTime = dateTime.replace('T', '%20').replace('Z', '');

        // Make api call
        setLoading(true);
        const url = `http://localhost:8081/BotKey/create?time=${dateTime}&bugid=${sessionName}`;
        fetchData(url)
        .then( response => {
            const data = response.data;
            console.log(data);
            setTimeout(() => {
                setLoading(response.isLoading);
                if(!response.error) { navigateToPath(data.BugId,data.SessionKey); }
            }, 300);
        })
        .catch( error => {
            setErrorMsgs(errors.fetch);
        });
    };

    const renderErrorMsg = (error) => {
        return ( error === errorMsgs.err &&
            <p className="error_msg">{errorMsgs.msg}</p>
        )
    };

    return (
      <div>
        {isLoading ? (
            <div>Loading...</div>
        ) : (
            <div>
            <h1>Create New Session</h1>
            <form onSubmit={handleSubmit}>
                <label>Session Name:</label>
                <input
                    type="text"
                    placeholder="Enter a name"
                    value={sessionName}
                    onChange={(e) => setSessionName(e.target.value) }
                />
                {renderErrorMsg("empty")}
                {renderErrorMsg("taken")}
                <input type="submit" value="Start Session"/>
            </form>
            <button className = "load_btn">
                <Link to={"/Load"}>Load Previous Session</Link>
            </button>
            <button className = "back_btn">
                <Link to={"/"}>Back to Menu</Link>
            </button>
            </div>
        )}
      </div>
    )
}

export default CreateSession;
