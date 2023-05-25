async function fetchData(url) {
  let data = null;
  let isLoading = true;
  let error = null;

  let lol = await fetch(url)
    .then(response => {
      if (!response.ok) {
        throw Error("Unable to fetch that resource");
      } 
      return response.json();
    })
    .then(rdata => {
      isLoading = false;
      data = rdata;
      error = null;
    })
    .catch(err => {
      isLoading = false;
      error = err.message;
    })
    return { data, isLoading, error } ;
};

export default fetchData;
