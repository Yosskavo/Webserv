
window.copyCgiValue = function(btn, text) {
    navigator.clipboard.writeText(text).then(() => {
        const originalText = btn.innerText;
        btn.innerText = "Copied!";
        btn.classList.add("copied");
        setTimeout(() => {
            btn.innerText = originalText;
            btn.classList.remove("copied");
        }, 1500);
    }).catch(err => {
        console.error('Failed to copy: ', err);
    });
};

function createCgiField(label, value, isBinary = false) {
    const binaryClass = isBinary ? " binary" : "";
    return `
        <div class="cgi-field">
            <span class="cgi-field-label">${label}</span>
            <div class="cgi-field-value-wrapper">
                <button type="button" class="cgi-copy-btn" onclick="copyCgiValue(this, '${value}')">Copy</button>
                <div class="cgi-field-value${binaryClass}">${value}</div>
            </div>
        </div>
    `;
}
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
            winnerBanner = `<span style="color: #9ece6a; font-weight: bold;">Python Wins!</span> (Faster by ${diff} ms)`;
        } else if (phpData.time_ms < pyData.time_ms) {
            winnerBanner = `<span style="color: #7aa2f7; font-weight: bold;">PHP Wins!</span> (Faster by ${diff} ms)`;
        } else {
            winnerBanner = `<span style="color: #ff9e64; font-weight: bold;">It's a dead heat tie!</span>`;
        }

        // Render Side-by-Side Results
        resultsContainer.innerHTML = `
            <div class="cgi-results-wrapper">
                <div class="cgi-winner-banner">
                    ${winnerBanner}
                </div>
                
                <div class="cgi-cards-container">
                    <!-- Python Card -->
                    <div class="cgi-card">
                        <div class="cgi-card-header">
                            <h3 class="cgi-card-title">${pyData.language}</h3>
                            <span class="cgi-time-metric" style="color: #9ece6a;">${pyData.time_ms} ms</span>
                        </div>
                        
                        ${createCgiField('SHA-256', pyData.sha256)}
                        ${createCgiField('MD5', pyData.md5)}
                        ${createCgiField('Base64', pyData.base64)}
                        ${createCgiField('Binary Data', pyData.binary, true)}
                    </div>

                    <!-- PHP Card -->
                    <div class="cgi-card">
                        <div class="cgi-card-header">
                            <h3 class="cgi-card-title">${phpData.language}</h3>
                            <span class="cgi-time-metric" style="color: #7aa2f7;">${phpData.time_ms} ms</span>
                        </div>
                        
                        ${createCgiField('SHA-256', phpData.sha256)}
                        ${createCgiField('MD5', phpData.md5)}
                        ${createCgiField('Base64', phpData.base64)}
                        ${createCgiField('Binary Data', phpData.binary, true)}
                    </div>
                </div>
            </div>
        `;
    } catch (err) {
        resultsContainer.innerHTML = `<p class="error-code">Error during CGI execution: ${err.message}</p>`;
    }
}
