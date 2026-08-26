function closeDeleteModal() {
	const modal = document.getElementById("deleteModal");
	if (modal) {
		modal.classList.add("hidden");
	}
}

function ft_delete(Filename) {
	if (!Filename || Filename.startsWith(".")) return;

	const modal = document.getElementById("deleteModal");
	const modalText = document.getElementById("deleteModalText");
	const confirmBtn = document.getElementById("confirmDeleteBtn");

	if (!modal || !modalText || !confirmBtn) return;

	modalText.innerHTML = `Are you sure you want to delete <code style="color: #ff9e64;">${Filename}</code>?`;
	modal.classList.remove("hidden");

	// Attach click action to the Delete button
	confirmBtn.onclick = async function() {
		closeDeleteModal();
		const status = document.getElementById("status");

		try {
			const respond = await fetch(`/upload/${encodeURIComponent(Filename)}`, { method: 'DELETE' });
			if (respond.ok) {
				status.innerText = `The file '${Filename}' was deleted successfully.`;
				loadFiles();
			} else {
				status.innerText = `Couldn't delete '${Filename}' (Status code: ${respond.status})`;
			}
		} catch (err) {
			status.innerText = `Delete Error: ${err.message}`;
		}
	};
}
