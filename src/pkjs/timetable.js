

var NUM_SERVICES = 5;

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function get_next_departures(response){
	
	var extract_row = function(line){
		var chunks = line.replace(/<td [^>]+>/g,'').replace(/&nbsp;/g,'').split("</td>");
		var ret = {};
		ret.service = chunks[0];
		ret.destination = chunks[1];
		ret.time = chunks[2];
		ret.isLiveFeed = chunks[3] == 'LF';
		return ret;
	};
	
	var flat = response.replace(/[\r\n]/g,'');
	var body = flat.match(/<table[^>]+>(.+)<\/table>/)[1];
	var lines = body.split("</tr>");
	var services =  lines.map(extract_row);
  services.length = NUM_SERVICES;
  return services;
}

function getTimes(NAPTAN) {
  var url = 'http://tsy.acislive.com/pip/stop_simulator_table.asp?naptan='+ NAPTAN;
  xhrRequest(url,'get',onTimesAvailable);
}

function logCallback(m){
  console.log("Result:",m);
}

function onTimesAvailable(res){
  var times = get_next_departures(res);
  console.log(times);
  
  Pebble.sendAppMessage({CLEAR:0});
  var i = 0;
  times.forEach(function(time){
    console.log("Sending",JSON.stringify(time));
    time.NEXT = i;
    Pebble.sendAppMessage(time,logCallback,function(er){console.log("error",er);});
    i++;
  });
  
  
}
           


module.exports = getTimes;