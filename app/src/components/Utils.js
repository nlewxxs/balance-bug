import React, { useState, useEffect } from 'react';

const usePoll = (callback, delay) => {
  const [isPolling, setIsPolling] = useState(true);

  useEffect(() => {
    const poll = setInterval(() => {
      callback();
    }, delay);

    return () => {
      clearInterval(poll);
    };
  }, [callback, delay]);

  return isPolling;
};


async function fetchData(url) {
    let data = null;
    let isLoading = true;
    let error = null;

    await fetch(url)
    .then(response => {
        //console.log(response);
        if (!response.ok) {
            throw Error("Unable to fetch that resource");
        }
        return response.json();
    })
    .then(rdata => {
        isLoading = false;
        data = rdata;
    })
    .catch(err => {
        isLoading = false;
        error = err.message;
        console.log(err.message);
    })
    return { data, isLoading, error } ;
};

export {fetchData,
        usePoll}; 
