async function test405() {
	const status = document.getElementById("status");
	try {
		const response = await fetch('/index.html', {
			method: 'POST',
			body: 'test'
		});
		status.innerText = `405 Test Response: ${response.status}`;
	} catch (err) {
		status.innerText = `Error: ${err.message}`;
	}
}

async function test413() {
	const status = document.getElementById("status");
	try {
		const largeData = "X".repeat(50 * 1024);

		const response = await fetch('/upload/large_test.txt', {
			method: 'PUT',
			body: largeData
		});
		status.innerText = `413 Test Response: ${response.status}`;
	} catch (err) {
		status.innerText = `Error: ${err.message}`;
	}
}
