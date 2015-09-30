function HTTPGET(url) {
    var req = new XMLHttpRequest();
    req.open("GET", url, false);
    req.send(null);
    return req.responseText;
}

var getNasa = function() {
    //Get weather info
    var response = HTTPGET("https://api.nasa.gov/planetary/apod?concept_tags=True&api_key=oFNUigXjqgKAKeVxMyACrcaqeFVoTMDXJ96Q24qS");
 
    //Convert to JSON
    var json = JSON.parse(response);
 
    //Extract the data
    var img = json.url;
 
    //Console output to check all is working.
    console.log("It is " + img);
 
    //Construct a key-value dictionary
    var dict = {"KEY_IMG" : img};
 
    //Send data to watch for display
    Pebble.sendAppMessage(dict);
};

Pebble.addEventListener("ready",
  function(e) {
    getNasa();
  }
);