function ajax_request(url, callback) {
	var xhr;
	if (typeof XMLHttpRequest !== "undefined")
		xhr = new XMLHttpRequest();
	else {
		var versions = ["MSXML2.XmlHttp.5.0","MSXML2.XmlHttp.4.0","MSXML2.XmlHttp.3.0","MSXML2.XmlHttp.2.0","Microsoft.XmlHttp"];
		for (var i = 0, len = versions.length; i < len; i++) {
			try {
				xhr = new ActiveXObject(versions[i]);
				break;
			} catch (e) {}
		}
	}
	
	xhr.onreadystatechange = ensureReadiness;
	
	function ensureReadiness(){
		if (xhr.readyState<4){return;}
		if (xhr.status!==200){return;}
		if (xhr.readyState===4){callback(xhr);}
	}
	
	xhr.open("GET",url,true);
	xhr.send("");
}

function on(element, event, callback) {
	if (element.addEventListener) {
		element.addEventListener(	event,
									function(){callback(element, event);}, 
									false);
	} else {
		if (element.attachEvent) {
			element.attachEvent(event, function() {callback(element, event);});
		}
	}
}

var a=document.getElementsByTagName("a");
for (var i=0; i<a.length;i++) {
	if (a[i].getAttribute("rel")=="gpio") {
		on(	a[i],"click",
			function(obj, ev) {
				var vnew = 1-parseInt(obj.getAttribute("d-val"));
				ajax_request("/gpio?st="+vnew+"&pin="+obj.getAttribute("data-id"),
						     function() {
								obj.setAttribute("d-val",vnew);
								obj.innerHTML="<div class='g_"+vnew +" k fll'>"+obj.getAttribute("data-id")+"</div>";}
							);
			}
		);
	}
}