var transferInProgress = false;

function transferImageBytes(bytes, chunkSize, successCb, failureCb) {
  var retries = 0;

  var success = function() {
    console.log("Success cb=" + successCb);
    if (successCb !== undefined) {
      successCb();
    }
  };
  var failure = function(e) {
    console.log("Failure cb=" + failureCb);
    if (failureCb !== undefined) {
      failureCb(e);
    }
  };

  // This function sends chunks of data.
  var sendChunk = function(start) {
    var txbuf = bytes.slice(start, start + chunkSize);
    console.log("Sending " + txbuf.length + " bytes - starting at offset " + start);
    Pebble.sendAppMessage({ "NETDL_DATA": txbuf },
      function(e) {
        // If there is more data to send - send it.
        if (bytes.length > start + chunkSize) {
          sendChunk(start + chunkSize);
        }
        // Otherwise we are done sending. Send closing message.
        else {
          Pebble.sendAppMessage({"NETDL_END": "done" }, success, failure);
        }
      },
      // Failed to send message - Retry a few times.
      function (e) {
        if (retries++ < 3) {
          console.log("Got a nack for chunk #" + start + " - Retry...");
          sendChunk(start);
        }
        else {
          failure(e);
        }
      }
    );
  };

  // Let the pebble app know how much data we want to send.
  Pebble.sendAppMessage({"NETDL_BEGIN": bytes.length },
    function (e) {
      // success - start sending
      sendChunk(0);
    }, failure);

}

function downloadBinaryResource(imageURL, callback, errorCallback) {
    var req = new XMLHttpRequest();
    req.open("GET", imageURL,true);
    req.responseType = "arraybuffer";
    req.onload = function(e) {
        console.log("loaded");
        var buf = req.response;
        if(req.status == 200 && buf) {
            var byteArray = new Uint8Array(buf);
            var arr = [];
            for(var i=0; i<byteArray.byteLength; i++) {
                arr.push(byteArray[i]);
            }

            console.log("Downloaded file with " + byteArray.length + " bytes.");
            callback(arr);
        }
        else {
          errorCallback("Request status is " + req.status);
        }
    };
    req.onerror = function(e) {
      errorCallback(e);
    };
    req.send(null);
}

Pebble.addEventListener("appmessage", function(e) {
  console.log("Got message: " + JSON.stringify(e));
  
  console.log(e.payload['NETDL_URL']);
  if ('NETDL_URL' in e.payload && e.payload['NETDL_URL'] != '') {
    if (transferInProgress === false) {
      transferInProgress = true;
      downloadBinaryResource(e.payload['NETDL_URL'], function(bytes) {
        transferImageBytes(bytes, e.payload['NETDL_CHUNK_SIZE'],
          function() { console.log("Done!"); transferInProgress = false; },
          function(e) { console.log("Failed! " + JSON.stringify(e)); transferInProgress = false; }
        );
      },
      function(e) {
        console.log("Download failed: " + e); transferInProgress = false;
      });
    }
    else {
      console.log("Ignoring request to download " + e.payload['NETDL_URL'] + " because another download is in progress.");
    }
  }
});

function HTTPGET(url) {
    var req = new XMLHttpRequest();
    req.open("GET", url, false);
    req.send(null);
    return req.responseText;
}

var getNasa = function() {
  //Get weather info
  //var response = HTTPGET("https://api.nasa.gov/planetary/apod?concept_tags=True&api_key=oFNUigXjqgKAKeVxMyACrcaqeFVoTMDXJ96Q24qS");

  //Convert to JSON
  //var json = JSON.parse(response);

  //Extract the data
  var img = "http://assets.getpebble.com.s3-website-us-east-1.amazonaws.com/pebble-faces/cherie.png"; //json.url;
  
  //Console output to check all is working.
  /*
  console.log("It is " + img);
  downloadBinaryResource(img, function(bytes) {
    transferImageBytes(bytes, 1000,
      function() { console.log("Done!"); transferInProgress = false; },
      function(e) { console.log("Failed! " + e); transferInProgress = false; }
    );
  },
  function(e) {
    console.log("Download failed: " + e); transferInProgress = false;
  });
  */
 
  //Construct a key-value dictionary
  var dict = {"KEY_IMG" : img};

  //Send data to watch for display
  Pebble.sendAppMessage(dict);
};

Pebble.addEventListener("ready",
  function(e) {
    console.log("NetDownload JS Ready");
    getNasa();
  }
);