async function ft_put() {
	const fileInput = document.getElementById("fileInput");
	const status    = document.getElementById("status");

	if (!fileInput.files[0])
	{
		alert("No file :(");
		return ;
	}
	try 
	{
		const respond = await fetch(`/upload/${encodeURIComponent(fileInput.files[0].name)}`, {method: 'PUT', body: fileInput.files[0]});
		if (respond.ok) {
			status.innerText = `The file uploaded :)`;
			fileInput.value = "";
			loadFiles();
		} else {
			status.innerText = `Upload faild :( [The status code : ${respond.status}]`;
		}
	} catch (err) {
		status.innerText = `Error : ${err.message}`;
	}
}
