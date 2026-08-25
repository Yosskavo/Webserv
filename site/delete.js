async function ft_delete(Filename) {
	if (!confirm(`Are you sure want to delete ${Filename}`))
		return ;
	const status = document.getElementById("status");

	try {
		const respond = await fetch(`/upload/${encodeURIComponent(Filename)}`, {method : 'DELETE'});
		if (respond.ok)
		{
			status.innerText = `The file '${Filename}' deleted`;
			loadFiles();
		} else {
			status.innerText = `couldn't delete ${Filename} :( [The status code : ${respond.status}]`;
		}
	} catch (err) {
		status.innerText = `Error : ${err.message}`;
	}
}
