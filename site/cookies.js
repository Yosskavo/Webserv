function getCookie(name) {                                                                   
	const cookies = document.cookie.split("; ");                                             
																							 
	for (let i = 0; i < cookies.length; i++) {                                               
		const parts = cookies[i].split("=");                                                 
		if (parts[0] === name) {                                                             
			return parts[1];
		}                                                                                    
	}                                                                                        
	return null;
}

function setCookie(name, value) {                                                            
	document.cookie = `${name}=${value}; path=/; max-age=31536000`;                          
}

function trackVisits() {
	const display = document.getElementById("visitCounter");
	if (!display) return;

	let visits = getCookie("visits");

	if (!visits) {
		// 1st time visiting: initialize to 1
		visits = 1;
		display.innerHTML = 'Welcome! This is your <span class="visit-count">1st</span> visit.';
	} else {
		// Returning visitor: parseInt converts string "4" to integer 4, then + 1            
		visits = parseInt(visits) + 1;
		display.innerHTML = `Welcome back! You have visited this server <span class="visit-count">${visits}</span> times.`;
	}

	// Save the new number back to the cookie
	setCookie("visits", visits);
}

function resetVisits() {
	// Setting max-age=0 deletes the cookie
	document.cookie = "visits=; path=/; max-age=0";
	
	const display = document.getElementById("visitCounter");
	if (display) {
		display.innerText = "Cookie deleted! Refresh the page to start from visit #1.";      
	}
}

trackVisits();
