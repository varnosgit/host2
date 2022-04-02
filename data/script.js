  	var gateway = `ws://${window.location.hostname}/ws`;
  	var websocket;
  	window.addEventListener('load', onLoad);

  	myURL_param = parseURLParams(window.location.href);

  	function onLoad(event) {
  	    initWebSocket();
  	}

  	function initWebSocket() {
  	    console.log('Trying to open a WebSocket connection...');
  	    websocket = new WebSocket(gateway);
  	    websocket.onopen = onOpen;
  	    websocket.onclose = onClose;
  	    websocket.onmessage = onMessage;
  	}

  	function onClose(event) {
  	    console.log('Connection closed');
  	    setTimeout(initWebSocket, 2000);
  	}

  	function parseURLParams(url) {
  	    var queryStart = url.indexOf("?") + 1,
  	        queryEnd = url.indexOf("#") + 1 || url.length + 1,
  	        query = url.slice(queryStart, queryEnd - 1),
  	        pairs = query.replace(/\+/g, " ").split("&"),
  	        parms = {},
  	        i, n, v, nv;

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