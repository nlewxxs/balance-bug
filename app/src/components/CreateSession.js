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
        // Realistically won't need this due to the id/SessionName

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
        const url = `http://localhost:8081/Session/Create?TimeStamp=${dateTime}&BugName=${sessionName}`;
        fetchData(url)
        .then( response => {
            const data = response.data;
            console.log(data);
            setTimeout(() => {
                setLoading(response.isLoading);
                if(!response.error) { navigateToPath(data.BugName,data.SessionId); }
                else { setErrorMsgs( { err: "fetch", msg: errors.fetch } );  }
                console.log(errorMsgs);
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
            <div className="create_page">
            <h1 className="title">Create Session</h1>
            <div className="form_container">
            <form onSubmit={handleSubmit}>
                <label className="input_label">SESSION NAME</label>
                <input className="input_box"
                    type="text"
                    placeholder="Enter a name"
                    value={sessionName}
                    onChange={(e) => setSessionName(e.target.value) } />
                {renderErrorMsg("fetch")}
                {renderErrorMsg("empty")}
                {renderErrorMsg("taken")}
                <input className="input_btn" type="submit" value="Start Session"/>
            </form>
            </div>
            <div className="btn_container">
                <div className="load_btn">
                  <button className="btn">
                    <Link to="/Load" className="btn_link">Load Session</Link>
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

export default CreateSession;
