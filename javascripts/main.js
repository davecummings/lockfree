$(document).ready(function() {
	var contentsUL = document.getElementById("contentsUL");
	var headers = document.querySelectorAll("section > H2, H3");
		
	for (var i = 0; i < headers.length; i++) {
		var header = headers[i];
		var label = header.innerText;
		var linkLabel = label.replace(/ /g, "-").replace(/\'/, "-");
		header.id = linkLabel;
		
		var li = document.createElement("li");
		var a = document.createElement("a");
		a.href = "#" + linkLabel;
		
		if (header.tagName == "H3") {
			var small = document.createElement("small");
			small.innerText = label;
			a.appendChild(small);
		} else {
			a.innerText = label;
		}
		
		li.appendChild(a);
		contentsUL.appendChild(li);
	}

	if (window.location && window.location.href) {
		var url = window.location.href;
		var index = url.indexOf("#");
		if (index > 0) {
			var anchor = url.substring(index);
			$('html,body').animate({scrollTop: $(anchor).offset().top},0);
			if ($('body').scrollTop()+20 < $("#main-content").offset().top) {
		 		val = $("#main-content").offset().top-70 - ($('body').scrollTop() / 1.333333333);
		 	} else {
		 		val = 20;
		 	}
	        $("#contentsDiv").animate({'top' : val + "pt"}, 0);
		}
	}

	 $(window).scroll(function () {
	 	var val;
	 	if ($('body').scrollTop()+20 < $("#main-content").offset().top) {
	 		val = $("#main-content").offset().top-70 - ($('body').scrollTop() / 1.333333333);
	 	} else {
	 		val = 20;
	 	}
        $("#contentsDiv").animate({'top' : val + "pt"}, 0);
	 });

});