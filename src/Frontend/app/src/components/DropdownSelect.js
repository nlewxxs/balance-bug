import React, { useState } from "react";
import Select from "react-select";

function DropdownSelect({ bugs, callback }) {
    const [selectedOption, setOption] = useState("");
    const [isHovered, setHover] = useState(false);

    const handleChange = (selectedOption) => {
        // console.log("option:",selectedOption);
        setOption(selectedOption);
        callback(selectedOption.label);
    }

    const handleHover = () => {
        setHover(true);
    };

    const handleLeave = () => {
        setHover(false);
    };

    const styles = {
    control: (styles, state) => ({
            ...styles,
            fontFamily: "Arial",
            backgroundColor: "#1E1F22",
            boxShadow: "none",
            overflow: "auto",
            border: "none",
            borderRadius: "5px",
            cursor: "text",
            color: "balck",
            ":isHovered": {
                color: "#1E1F22",
                backgroundColor: "#1E1F22"
            }
    }),
    valueContainer: (styles) => ({
        ...styles,
        padding: "10px",
    }),
    dropdownIndicator: (styles) => ({
        ...styles,
        backgroundColor: "#1E1F22",
        color: isHovered ? "#80848E" : "#1E1F22",
        transition: "all 0.6s ease"
    }),
    indicatorSeparator: (styles) => ({
        ...styles,
        background: "#1E1F22",
    }),
    input: (styles, isFocused) => ({
        ...styles,
        color: "#DBDEE1",
        padding: 0,
    }),
    placeholder: (styles) => ({
        ...styles,
        color: "#80848E",
    }), 
    menuList: (styles) => ({
        overflow: "auto",
        ...styles,
    }),
    menu: (styles) => ({
        ...styles,
        marginTop: 0,
        backgroundColor: "#1E1F22",
    }),
    noOptionsMessage: (styles) => ({
        ...styles,
        background: "#1E1F22",
        color: "#80848E"
    }),
    option: (styles, { data, isDisabled, isFocused, isSelected }) => {
        return {
            ...styles,
            color:  
                isDisabled
                ? "grey"
                : isFocused
                ? "lightgrey"
                : isSelected
                ? "white"
                : "darkgrey",
            // transition: "all 0.3s ease",
            backgroundColor: 
                isDisabled
                ? "#1E1F22"
                : isSelected
                ? "#404249"
                : isFocused
                ? "#36373D"
                : "#1E1F22",
            cursor: 
                isDisabled
                ? "default"
                : "pointer",
            ":active": {
                ...styles[":active"],
                backgroundColor: 
                !isDisabled
                ? "#404249"
                : isSelected
                ? "green"
                : "red"
            }
        };
    },
    singleValue: (styles) => ({
      ...styles,
      color: "white"
    })
    };
    
    return (
    <div onMouseOver={handleHover} onMouseLeave={handleLeave}>
      <Select
        // menuIsOpen={isHovered}
        placeholder="Select a bug"
        overflow="auto"
        maxMenuHeight={"23vh"}
        options={bugs ? 
                bugs.map((bug) => ({ 
                value: bug.BugId, 
                label: bug.BugName,
            })) : []}
        onChange={handleChange}
        defaultValue=""
        value={selectedOption}
        styles={styles}
      />
    </div>
    );
};

export default DropdownSelect;
