var NAPTAN = "37020361";
var STOP_NAME = "West Street";

var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig, null, { autoHandleEvents: false });


var getTimes=require('./timetable.js');

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    NAPTAN = localStorage.getItem('NAPTAN') || NAPTAN;
    STOP_NAME = localStorage.getItem('STOP_NAME') || STOP_NAME;
    console.log('PebbleKit JS ready!');
    Pebble.sendAppMessage({STOPNAME:STOP_NAME});
    getTimes(NAPTAN);
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
  }                     
);

Pebble.addEventListener('showConfiguration', function(e) {
  clay.setSettings('NAPTAN',NAPTAN);
  clay.setSettings('STOPNAME',STOP_NAME);
  Pebble.openURL(clay.generateUrl());
});


Pebble.addEventListener('webviewclosed', function(e) {
  console.log("Web view closed", JSON.stringify(e));
  
  console.log(JSON.stringify(e));
  if (e && !e.response) {
    return;
  }

  var settings = JSON.parse(decodeURIComponent(e.response));
  NAPTAN=settings.NAPTAN.value;
  STOP_NAME = settings.STOPNAME.value;
  getTimes(NAPTAN);
  localStorage.setItem('NAPTAN', NAPTAN);
  localStorage.setItem('STOP_NAME', STOP_NAME);
  console.log("Settings are ", JSON.stringify(settings));

});