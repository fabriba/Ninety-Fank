var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
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
      
      // Conditions: json.weather[0].icon contains a string identifying an icon
      var icon = json.weather[0].icon;      
      console.log("icon is " + icon);
	  
	  // Conditions: json.weather[0].icon contains a string identifying an icon
      var location = json.weather[0].name;      
      console.log("Location is " + location);
	  	if (pos.timestamp = 0) { location = "gps off"; }
      
      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_TEMPERATURE": temperature,
        "KEY_CONDITIONS": conditions,
        "KEY_ICON": icon,
		"KEY_LOCATION": location
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

function locationError(err) {
    console.log("Error requesting location!");
	// now we still try to retrieve the weather update for our standard location (Milano) 
		//NOTE: these should be the same as in config.h
		//#define LATITUDE    45.52
		//#define LONGITUDE 9.17
	var fallback_pos = { coords.latitude:"45.52", coords.longitude:"9.17", timestamp:0 };
	locationSuccess(fallback_pos);
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 300000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");

    // Get the initial weather
    getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getWeather();
  }                     
);

/* Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
  }                     
);
*/