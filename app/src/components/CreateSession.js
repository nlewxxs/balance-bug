import React, { useState } from "react";
import { Link, useNavigate } from "react-router-dom";

function CreateSession() {
    const [sessionName, setSessionName] = useState("");
    const [isLoading, setLoading] = useState(false);
    const [errorMsgs, setErrorMsgs] = useState({ err: "", msg: ""});
    const navigate = useNavigate();
    const errors = {
        taken: "Name is currently in use",
        empty: "Please enter a name"
    };
    
    const checkIfTaken = () => {
        // Make api call
        //  fetch("api/isnametaken?name=${sessionName}")
        //      .then(response => { response.json(); })
        //      .then(data => { 
        //          if (data.isTaken) {
        //              setErrorMsgs( { // move to if statement
        //                  err: "taken",
        //                  msg: errors.taken
        //              });
        //              return true;
        //          }
        //      });
        return false;
    };

    const navigateToPath = (id) => {
        const path = "/Display/" + sessionName;
        console.log(path);
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
        if (checkIfTaken()) {
           return; 
        }
         // reset error msg        
        else {
            setErrorMsgs( { err: "", msg: "" } ); 
        }
        // SHOULD THIS BE DONE HERE OR IN THE BACKEND?
        // const currentDate = new Date();
        // const currentDateTime = currentDate.toISOString();
        // const requestData = {
            // sessionName: sessionName,
            // currentDateTime: currentDateTime
        // };
        setLoading(true);
        // Make api call
        // fetch("api/createnewsession?data=${requestData}")
        // .then get reponse with/generate sessionID and navigate
        setTimeout( () => {
            setLoading(false);
            let id = "123"; 
            navigateToPath(id);
        }, 1000);
        
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
