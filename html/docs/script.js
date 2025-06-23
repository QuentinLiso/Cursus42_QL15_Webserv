	const uploadDir = "/uploads/";

	function uploadFile() {
		const fileInput = document.getElementById("putFile");
		const file = fileInput.files[0];
		if (!file) return alert("Choose a file first.");

		const xhr = new XMLHttpRequest();
		xhr.open("PUT", uploadDir + encodeURIComponent(file.name), true);
		xhr.onload = function () {
			alert("PUT response: " + xhr.status + " " + xhr.statusText);
			loadFiles(); // Refresh list after upload
		};
		xhr.onerror = function () {
			alert("PUT failed");
		};
		xhr.send(file);
	}

	function deleteFile(filename) {
		fetch(uploadDir + encodeURIComponent(filename), { method: "DELETE" })
			.then(res => {
				if (!res.ok) throw new Error("Server error " + res.status);
				return res.text();
			})
			.then(() => {
				document.getElementById("status").textContent = `✅ Deleted: ${filename}`;
				loadFiles();
			})
			.catch(err => {
				document.getElementById("status").textContent = `❌ Error deleting ${filename}: ${err}`;
			});
	}

	function loadFiles() {
		fetch(uploadDir)
			.then(res => res.text())
			.then(html => {
				const ul = document.getElementById("fileList");
				ul.innerHTML = "";

				// Parse autoindex HTML: match href="filename"
				const regex = /href="([^"?/][^"]+)"/g;
				let match;
				while ((match = regex.exec(html)) !== null) {
					const name = match[1];
					const li = document.createElement("li");
					li.innerHTML = `
						<a href="${uploadDir}${name}" target="_blank">${name}</a>
						<button onclick="deleteFile('${name}')">Delete</button>
					`;
					ul.appendChild(li);
				}
			})
			.catch(err => {
				document.getElementById("status").textContent = `❌ Failed to load files: ${err}`;
			});
	}

	window.onload = loadFiles;
