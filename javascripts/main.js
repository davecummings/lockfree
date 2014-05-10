$(document).ready(function() {
	var contentsUL = document.getElementById("contentsUL");
	var headers = document.querySelectorAll("section > H2, H3");
		
	for (var i = 0; i < headers.length; i++) {
		var header = headers[i];
		var label = header.innerText;
		var linkLabel = label.replace(/ /g, "-");
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
		}
	}

	 $(window).scroll(function () {
	 	var top;
	 	if ($('html,body').scrollTop()+20 < $("#blurb").offset().top)
	 		top = $("#blurb").offset().top - $('html,body').scrollTop();
	 	else
	 		top = 20;
        $("#contentsDiv").animate({'top' : top}, 0);
	 });

});