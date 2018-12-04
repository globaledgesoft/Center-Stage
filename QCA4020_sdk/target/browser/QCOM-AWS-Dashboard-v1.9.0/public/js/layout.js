
/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/


var socket = io();

$.notify.addStyle("breach", {
	html: "<div>\n<span data-notify-text></span>\n</div>",
	classes: {
		base: {
			"padding": "8px 15px 8px 14px",
			"background-color": "#fcf8e3",
			"white-space": "nowrap",
			"padding-left": "25px",
			"background-repeat": "no-repeat",
			"background-position": "3px 7px"
		},
		alert: {
			"color": "#000000",
			"background-color": "#FFFF87",
			"border": "1px solid #3356b7",
			"border-radius":"5px",
			"background-image": "url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABQAAAAUCAMAAAC6V+0/AAABJlBMVEXr6eb/2oD/wi7/xjr/0mP/ykf/tQD/vBj/3o7/uQ//vyL/twebhgD/4pzX1K3z8e349vK6tHCilCWbiQymn0jGworr6dXQza3HxcKkn1vWvV/5uRfk4dXZ1bD18+/52YebiAmyr5S9mhCzrWq5t6ufjRH54aLs0oS+qD751XqPhAybhwXsujG3sm+Zk0PTwG6Shg+PhhObhwOPgQL4zV2nlyrf27uLfgCPhRHu7OmLgAafkyiWkD3l49ibiAfTs0C+lgCniwD4sgDJxqOilzDWowWFfAH08uebig6qpFHBvH/aw26FfQTQzsvy8OyEfz20r3jAvaKbhgG9q0nc2LbZxXanoUu/u5WSggCtp1anpJKdmFz/zlX/1nGJiYmuq5Dx7+sAAADoPUZSAAAAAXRSTlMAQObYZgAAAAFiS0dEAIgFHUgAAAAJcEhZcwAACxMAAAsTAQCanBgAAAAHdElNRQfdBgUBGhh4aah5AAAAlklEQVQY02NgoBIIE8EUcwn1FkIXM1Tj5dDUQhPU502Mi7XXQxGz5uVIjGOJUUUW81HnYEyMi2HVcUOICQZzMMYmxrEyMylJwgUt5BljWRLjmJm4pI1hYp5SQLGYxDgmLnZOVxuooClIDKgXKMbN5ggV1ACLJcaBxNgcoiGCBiZwdWxOETBDrTyEFey0jYJ4eHjMGWgEAIpRFRCUt08qAAAAAElFTkSuQmCC)"
		}
	}
});

socket.on("connect", function(data) {
	console.log("Connected");
});

socket.on('notify', function(notifications) {
	console.log(notifications);
	for(var index = 0; index < notifications.length; index++) {
		$.notify(notifications[index], {
			autoHideDelay: 5 * 1000,
			style: "breach",
			className: "alert",
			globalPosition:"bottom right"
		});
	}
});
