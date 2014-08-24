var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function getWeather(pos) {
  // Construct URL
  var url = "http://api.openweathermap.org/data/2.5/weather?lat=" + pos.coords.latitude + "&lon=" + pos.coords.longitude;

  
  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      // Temperature in Kelvin requires adjustment
      var temperature = Math.round(json.main.temp - 273.15);
      console.log("Temperature is " + temperature);

      // Conditions: json.weather[0].description is more detailed than json.weather[0].main
      var conditions = json.weather[0].description;   
      console.log("Conditions are " + conditions);
      
      // icon: json.weather[0].icon contains a string identifying an icon
      var icon = json.weather[0].icon;      
      console.log("icon is " + icon);

      // location: json.name contains a string identifying weather location
      var location = json.name;      
      console.log("Location is " + location);
      // More accurate location via 
      // https://maps.googleapis.com/maps/api/geocode/json?latlng=45.52,9.18
      if (pos.timestamp === 0) { location = "gps off"; }  //if timestamp=0, location wasn't available
      
      // time: json.dt contains a string identifying the time when weather was updated
      var weather_age = json.dt+7200;  //temp dumb adjustment for my timezone being gmt+2
      console.log("Weather was taken @" + weather_age);
      
      
      
      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_TEMPERATURE": temperature,
        "KEY_CONDITIONS": conditions,
        "KEY_ICON": icon,
        "KEY_LOCATION": location,
        "KEY_EPOCH": weather_age, //pos.timestamp, //
      };
      
      
      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );

    }      
  );
}

function locationSuccess(gpsfix) {
  getWeather(gpsfix);
}

function locationError(err) {
    console.log("Error requesting location!");
	// now we still try to retrieve the weather update for our standard location (Milano) 
		//NOTE: these should be the same as in config.h
		//#define LATITUDE    45.52
		//#define LONGITUDE 9.17
		// timestamp 0 means this is not recent location, but just default data, it will be used as a flag in locationSuccess
	/* uncomment what follows to get standard weather location if no gps available */
	//var fallback_pos = { coords.latitude:"45.52", coords.longitude:"9.17", timestamp:0 };
	//locationSuccess(fallback_pos);
  
  var fail_dictionary = {
        "KEY_TEMPERATURE": -273.15,
        "KEY_CONDITIONS": "n/a",
        "KEY_ICON": "n/a",
        "KEY_LOCATION": "gps off",
        "KEY_EPOCH": "0"
      };
  
  // Send to Pebble
      Pebble.sendAppMessage(fail_dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );


  
}
/*
function getLocationInaccurate() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000, enableHighAccuracy: false} // timeout = 15s, maximumage = 60
  );
}
*/
function getLocation() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError, //getLocationInaccurate,
    {timeout: 15000, maximumAge: 60000, enableHighAccuracy: true} // timeout = 15s, maximumage = 60s
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");

    // Get the initial weather
    getLocation();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getLocation();
  }                     
);

