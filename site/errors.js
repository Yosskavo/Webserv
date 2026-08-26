function showErrorModal(title, message) {
	const modal = document.getElementById("errorModal");
	const modalTitle = document.getElementById("modalTitle");
	const modalMessage = document.getElementById("modalMessage");

	if (modal && modalTitle && modalMessage) {
		modalTitle.innerText = title;
		modalMessage.innerHTML = message;
		modal.classList.remove("hidden");
	}
}

function closeErrorModal() {
	const modal = document.getElementById("errorModal");
	if (modal) {
		modal.classList.add("hidden");
	}
}

async function test405() {
	try {
		const response = await fetch('/index.html', {
			method: 'POST',
			body: 'test'
		});
		showErrorModal(
			`${response.status} Method Not Allowed`,
			`The server rejected the <code>POST /index.html</code> request with status code <strong>${response.status}</strong> because only GET is allowed on this route.`
		);
	} catch (err) {
		showErrorModal("Error", `Request failed: ${err.message}`);
	}
}

async function test413() {
	try {
		const largeData = "X".repeat(50 * 1024); // 50 KB payload

		const response = await fetch('/upload/large_test.txt', {
			method: 'POST',
			body: largeData
		});
		showErrorModal(
			`${response.status} Payload Too Large`,
			`The server rejected the <code>50 KB</code> upload with status code <strong>${response.status}</strong> because it exceeds the configured <code>10 KB</code> body size limit.`
		);
	} catch (err) {
		showErrorModal("Error", `Request failed: ${err.message}`);
	}
}
