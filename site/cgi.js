async function fetchCgiResult(url) {
    const response = await fetch(url);
    const text = await response.text();
    
    if (!response.ok) {
        throw new Error(`[Status ${response.status}] ${text.substring(0, 120)}`);
    }
    try {
        return JSON.parse(text);
    } catch (e) {
        throw new Error(`Invalid JSON from ${url}: ${text.substring(0, 120)}`);
    }
}

async function runDualCgi() {
    const inputElem = document.getElementById("cgiInput");
    const resultsContainer = document.getElementById("cgiResults");
    const text = inputElem.value.trim() || "Hello 42 Webserv!";

    resultsContainer.innerHTML = "<p>Running both Python and PHP CGI concurrently in the background...</p>";

    try {
        // Send requests simultaneously
        const pyPromise = fetchCgiResult(`/cgi/transform.py?text=${encodeURIComponent(text)}`);
        const phpPromise = fetchCgiResult(`/cgi/transform.php?text=${encodeURIComponent(text)}`);

        const [pyData, phpData] = await Promise.all([pyPromise, phpPromise]);

        // Determine winner
        let winnerBanner = "";
        const diff = Math.abs(pyData.time_ms - phpData.time_ms).toFixed(4);
        if (pyData.time_ms < phpData.time_ms) {
            winnerBanner = `<strong>Python was faster</strong> by ${diff} ms!`;
        } else if (phpData.time_ms < pyData.time_ms) {
            winnerBanner = `<strong>PHP was faster</strong> by ${diff} ms!`;
        } else {
            winnerBanner = "It's a tie!";
        }

        // Render Side-by-Side Results
        resultsContainer.innerHTML = `
            <div style="margin: 15px 0; padding: 10px; background: #eef9ee; border: 1px solid #c2e2c2; border-radius: 5px;">
                ${winnerBanner}
            </div>
            <div style="display: flex; gap: 20px; flex-wrap: wrap;">
                <!-- Python Card -->
                <div style="flex: 1; min-width: 300px; border: 1px solid #ccc; border-radius: 8px; padding: 15px; background: #fdfdfd;">
                    <h3>${pyData.language}</h3>
                    <p><strong>Execution Time:</strong> <span style="color: green; font-weight: bold;">${pyData.time_ms} ms</span></p>
                    <hr>
                    <p><strong>SHA-256:</strong><br><code style="word-break: break-all;">${pyData.sha256}</code></p>
                    <p><strong>MD5:</strong><br><code>${pyData.md5}</code></p>
                    <p><strong>Base64:</strong><br><code>${pyData.base64}</code></p>
                    <p><strong>Binary:</strong><br><code style="font-size: 11px; word-break: break-all;">${pyData.binary}</code></p>
                </div>

                <!-- PHP Card -->
                <div style="flex: 1; min-width: 300px; border: 1px solid #ccc; border-radius: 8px; padding: 15px; background: #fdfdfd;">
                    <h3>${phpData.language}</h3>
                    <p><strong>Execution Time:</strong> <span style="color: blue; font-weight: bold;">${phpData.time_ms} ms</span></p>
                    <hr>
                    <p><strong>SHA-256:</strong><br><code style="word-break: break-all;">${phpData.sha256}</code></p>
                    <p><strong>MD5:</strong><br><code>${phpData.md5}</code></p>
                    <p><strong>Base64:</strong><br><code>${phpData.base64}</code></p>
                    <p><strong>Binary:</strong><br><code style="font-size: 11px; word-break: break-all;">${phpData.binary}</code></p>
                </div>
            </div>
        `;
    } catch (err) {
        resultsContainer.innerHTML = `<p style="color: red;">CGI Execution Error: ${err.message}</p>`;
    }
}
