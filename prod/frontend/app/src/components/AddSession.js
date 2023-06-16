import React, { useState, useEffect } from "react";
import { Link, useNavigate } from "react-router-dom";
import { fetchData, usePoll } from "./Utils";
import DropdownSelect from "./DropdownSelect.js";

function AddSession() {
    const [errorMsg, setErrorMsg] = useState({ err: "", msg: ""});
    const [sessionName, setSessionName] = useState("");
    const [isLoading, setLoading] = useState(false);
    const [selectedOption,setOption] = useState("Default"); 
    const [bugs, setBugs] = useState(null);
    const navigate = useNavigate();
    const errors = {
        none:    { err: "",        msg: "" },
        bugs:    { err: "bugs",    msg: "No bugs are currently online" },
        taken:   { err: "taken",   msg: "Name is currently in use" },
        empty:   { err: "empty",   msg: "Please enter a name" },
        fetch:   { err: "fetch",   msg: "Error connecting to the server" },
        select:  { err: "select",  msg: "Please select a bug" },
        invalid: { err: "invalid", msg: "This is not a valid bug" },
    };
   
    useEffect( () => {
        setLoading(true);
        getOnlineBugs();
    }, [] );

    usePoll( () => {
        getOnlineBugs();
    }, 9000); 

    const checkIfTaken = () => {
        // Realistically won't need this due to the id/SessionName
        //
        // Make api call
        //  fetchData(`api/isnametaken?name=${sessionName}`)
        //  .then(response => {
        //      const data = response.data;
        //      if (data.isTaken) {
        //          setErrorMsg(errors.taken);
        //          return true;
        //      }
        //  });
        return false;
    };

    const url = "http://90.196.3.86:8081/BugInformation/Online?Timeout=inf"
    
    const getOnlineBugs = () => {
        fetchData(url)
        .then(response => {
            setBugs(response.data);
            if (response.error ===  "Failed to fetch") { setErrorMsg(errors.fetch); }
            else if (response.error) { setErrorMsg( {err:"fetch", msg: response.error} ); }
            else if (!response.data.length) { setErrorMsg(errors.bugs); }
            else if ( errorMsg.err === "fetch" || errorMsg.err == "bugs" ) { setErrorMsg(errors.none) }; 
            // console.log(errorMsg, errors.fetch);
            // console.log(response.data);
            // console.log("Polling...");
            setLoading(false);
        })
        .catch(error => { setErrorMsg({err: "fetch", msg: error}); })
        // console.log(errorMsg);
    }

    const handleSelect = (selectedOption) => {
        if (selectedOption === "Default") {
            if (errorMsg.err !== "fetch") { setErrorMsg(errors.select); }
            return true;
        } 
        else if (!bugs.some(bug => bug.BugName == selectedOption)){
            if (!errorMsg) { setErrorMsg(errors.invalid); }
            console.log("set:",errorMsg);
            return true;
        }
        else {
            console.log("includes:",
                        (bugs.some(bug => bug.BugName == selectedOption)),
                        "selected:",
                        selectedOption,
                        "bugs",
                        bugs,
                        "values",
                        (bugs.map(bug => bug.BugName)));
            setOption(selectedOption);
            return false;
        }
    }

    const navigateToPath = (name, id) => {
        const path = "/Display/" + name + "?=" + id;
        // console.log(path);
        navigate(path);
    }

    const handleSubmit = (event) => {
        // Prevent page reloading
        event.preventDefault();
        // Priority of error check
        if (errorMsg.err==="fetch") { return; }
        // check if empty
        else if (sessionName.trim() === "") { setErrorMsg(errors.empty); return; }
        // check if taken
        else if (checkIfTaken()) { return; }
        // check if Bug has been selected
        else if (handleSelect(selectedOption)) { return; }
        // check for any other errors
        else if (!errorMsg) { console.log(errorMsg); return; }
        // reset error msg
        else { setErrorMsg(errors.none); }
        // get date & time of creation
        const currentDate = new Date();
        let dateTime = currentDate.toISOString();
        dateTime = dateTime.replace('T', '%20').replace('Z', '');
        // Make api call
        setLoading(true);
        const url = `http://90.196.3.86:8081/Session/Add?TimeStamp=${dateTime}&BugName=${selectedOption}&SessionName=${sessionName}`;
        fetchData(url)
        .then( response => {
            //console.log(response.data);
            const data = response.data;
            setTimeout(() => {
                setLoading(response.isLoading);
                if(!response.error) { navigateToPath(data.BugName,data.SessionId); }
                else { setErrorMsg(errors.fetch);  }
                console.log(errorMsg);
            }, 300);
        })
        .catch( error => { setErrorMsg(errors.fetch); });
    };

    const renderErrorMsg = (error) => {
        if (error === errorMsg.err) {
            //console.log("error:",error,"msg:",errorMsg.msg);
        }
        return ( error === errorMsg.err &&
            <div className="error_msg">{errorMsg.msg}</div>
        )
    };

    return (
      <div>
        {isLoading ? (
            <div>Loading...</div>
        ) : (
            <div className="add_page">
            <h1 className="title">New Session</h1>
            <div className="form_container">
            <form onSubmit={handleSubmit}>
                <label className="input_label">SESSION NAME</label>
                <input className="input_box"
                       type="text"
                       placeholder="Enter a name"
                       value={sessionName}
                       onChange={(e) => setSessionName(e.target.value) } />
                <label className="input_label">SELECT BUG</label>
                <div className="select_container">
                <DropdownSelect bugs={bugs} callback={handleSelect}/>
                </div>
                {renderErrorMsg("fetch")}
                {renderErrorMsg("bugs")}
                {renderErrorMsg("empty")}
                {renderErrorMsg("taken")}
                {renderErrorMsg("select")}
                {renderErrorMsg("invalid")}
                <input 
                    className="input_btn" 
                    type="submit" 
                    value="Start Session"
                    disabled={!errorMsg.error ? false : true} />
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

export default AddSession;
