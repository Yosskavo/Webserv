async function loadFiles() {
	const listContainer = document.getElementById("fileList");
	if (!listContainer) return;

	try {
		// 1. Send a GET request to the directory
		const response = await fetch('/upload/');
		const html = await response.text();

		// 2. Parse the HTML directory listing returned by NGINX
		const parser = new DOMParser();
		const doc = parser.parseFromString(html, 'text/html');
		
		// 3. Extract all links (<a> tags) except the parent directory '../'
		const links = Array.from(doc.querySelectorAll('a'))
			.map(a => a.getAttribute('href'))
			.filter(href => href && href !== '../' && href !== '/');

		// 4. Clear the previous list on the webpage
		listContainer.innerHTML = '';

		if (links.length === 0) {
			listContainer.innerHTML = '<li>No files uploaded yet.</li>';
			return;
		}

		// 5. Create a list item for each file
		links.forEach(fileName => {
			const li = document.createElement('li');
			li.innerHTML = `
				<a href="/upload/${fileName}" target="_blank">${fileName}</a>
				<button type="button" onclick="ft_delete('${fileName}')">Delete</button>     
			`;
			listContainer.appendChild(li);
		});

	} catch (err) {
		listContainer.innerHTML = `<li>Error loading files: ${err.message}</li>`;            
	}
}
window.onload = loadFiles;
