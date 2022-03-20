function currentTime() {
  let date = new Date(); 
  let hh = date.getHours();
  let mm = date.getMinutes();
  let ss = date.getSeconds();
  let session = "AM";
  
  if(hh > 12){
      session = "PM";
   }
   hh = (hh < 10) ? "0" + hh : hh;
   mm = (mm < 10) ? "0" + mm : mm;
   ss = (ss < 10) ? "0" + ss : ss;
   let time = hh + ":" + mm + ":" + ss + " " + session;
  document.getElementById("clock").innerText = time; 
  let t = setTimeout(function(){ currentTime() }, 1000); 
}

currentTime();
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  


function showHome() {
  var x = document.getElementById("zones");
  x.style.display = "none";
  x = document.getElementById("homee");
  x.style.display = "block";
}

function showZone() {
  var x = document.getElementById("zones");
  x.style.display = "block";
  x = document.getElementById("homee");
  x.style.display = "none";
}

  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');

  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
	    //let text = event.data;
	    const myArray = event.data.split(",");
		if (myArray[0] == "Zone Names")
			myArray.forEach(addZones);
	  /*
		var state;
		if (event.data == "1"){
		  state = "ON";
		}
		else if (event.data == "0"){
		  state = "OFF";
		}
		else if (event.data == "2"){
			let btn = document.createElement("button");
			btn.innerHTML = "Click Me";
			document.body.appendChild(btn);
		}
		else
			*/
		{
			document.getElementById(event.data).checked="checked";
		}
   // document.getElementById('state').innerHTML = state;

  }
  
  function addZones(item, index) {
		if (index < 1) return;
	    var myDiv = document.getElementById("ezones");
		let btn = document.createElement("button");
		myDiv.appendChild(btn);
		btn.innerHTML = item;
		btn.classList.add('button2');
		btn.onclick = function () {
			//alert("Button is clicked");
			window.location = "./myzone.html" + "?name=" + item; 
			};
		//btn.onclick = "./myzone.html"
		//document.body.appendChild(btn);
		
		
		//document.getElementById('z1').innerHTML = item;
}
  function onLoad(event) {
	var x = document.getElementById("zones");
	x.style.display = "none";
    initWebSocket();
    initButton();
	mydata = parseURLParams(window.location.href);
	if (mydata['name'] == 'zone') showZone();
  }
  function initButton() {
   // document.getElementById('button').addEventListener('click', toggle);
	//document.getElementById('button2').addEventListener('click', toggle2);
	document.getElementById('off').addEventListener('change', toggle2);
	document.getElementById('heat').addEventListener('change', toggle2);
	document.getElementById('cool').addEventListener('change', toggle2);
	document.getElementById('auto').addEventListener('change', toggle2);
	document.getElementById('on').addEventListener('change', toggle2);
  }
  function toggle(){
    websocket.send('toggle');
  }
    function toggle2(){
    websocket.send(this.id);
	//console.log('sss');
	//alert('Horray! Someone wrote "' + this.id + '"!');
  }
  
  function parseURLParams(url) {
    var queryStart = url.indexOf("?") + 1,
        queryEnd   = url.indexOf("#") + 1 || url.length + 1,
        query = url.slice(queryStart, queryEnd - 1),
        pairs = query.replace(/\+/g, " ").split("&"),
        parms = {}, i, n, v, nv;

    if (query === url || query === "") return;

    for (i = 0; i < pairs.length; i++) {
        nv = pairs[i].split("=", 2);
        n = decodeURIComponent(nv[0]);
        v = decodeURIComponent(nv[1]);

        if (!parms.hasOwnProperty(n)) parms[n] = [];
        parms[n].push(nv.length === 2 ? v : null);
    }
    return parms;
}

