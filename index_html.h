#ifndef INDEX_HTML_H
#define INDEX_HTML_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Biometric Lock Admin</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 20px; background: #f4f4f4; }
    .container { max-width: 600px; margin: auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }
    h2 { color: #333; }
    .card { background: #eee; padding: 15px; margin: 10px 0; border-radius: 5px; }
    button { background: #007bff; color: white; border: none; padding: 10px 20px; border-radius: 4px; cursor: pointer; font-size: 16px; margin: 5px; }
    button.del { background: #dc3545; }
    button:hover { opacity: 0.8; }
    input { padding: 8px; width: 60%; margin-bottom: 10px; border: 1px solid #ddd; border-radius: 4px; }
    table { width: 100%; border-collapse: collapse; margin-top: 20px; }
    th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
    th { background-color: #007bff; color: white; }
    #status { font-weight: bold; margin: 10px 0; color: #555; }
  </style>
</head>
<body>
  <div class="container">
    <h2>Biometric Lock Admin</h2>
    <div class="card">
      <h3>Enrollment Log</h3>
      <div id="status" style="height: 150px; overflow-y: scroll; background: #fff; border: 1px solid #ccc; padding: 5px; text-align: left; font-family: monospace; font-size: 12px;">Ready...</div>
      <p>Fingerprint Count: <span id="fpCount">0</span></p>
    </div>
    
    <div class="card">
      <h3>Enroll New Finger</h3>
      <input type="text" id="userName" placeholder="Enter User Name">
      <br>
      <button onclick="enrollFinger()">Start Enrollment</button>
    </div>
    
    <div class="card">
      <h3>Motor Diagnostics</h3>
      <button onclick="motorAction('open')">Forward (Open)</button>
      <button onclick="motorAction('close')">Reverse (Close)</button>
      <button onclick="motorAction('stop')" style="background:#555">Stop</button>
    </div>

    <div class="card">
      <h3>Manage Users</h3>
      <table id="userTable">
        <thead>
            <tr><th>ID</th><th>Name</th><th>Action</th></tr>
        </thead>
        <tbody></tbody>
      </table>
      <br>
      <button class="del" onclick="deleteAll()">Factory Reset (Delete All)</button>
    </div>
  </div>

<script>
  function motorAction(action) {
      updateStatus("Motor: " + action + "...");
      fetch('/api/motor?action=' + action)
      .then(res => res.text())
      .then(text => {
          updateStatus(text);
      })
      .catch(err => updateStatus("Error: " + err));
  }
  function updateStatus(msg) { 
      const el = document.getElementById('status');
      el.innerText += msg + "\n"; 
      el.scrollTop = el.scrollHeight;
  }
  function clearStatus() { document.getElementById('status').innerText = ""; }
  
  function fetchUsers() {
    fetch('/api/list')
      .then(response => response.json())
      .then(data => {
        document.getElementById('fpCount').innerText = data.length;
        const tbody = document.querySelector('#userTable tbody');
        tbody.innerHTML = '';
        data.forEach(user => {
          tbody.innerHTML += `<tr><td>${user.id}</td><td>${user.name}</td><td><button class="del" onclick="deleteFinger(${user.id})">Delete</button></td></tr>`;
        });
      });
  }

  function enrollFinger() {
    const name = document.getElementById('userName').value;
    if(!name) { alert("Please enter a name"); return; }
    
    updateStatus("Starting Enrollment... Place Finger");
    fetch(`/api/enroll?name=${encodeURIComponent(name)}`)
      .then(response => {
        const reader = response.body.getReader();
        const decoder = new TextDecoder();
        
        function read() {
          reader.read().then(({ done, value }) => {
            if (done) {
              fetchUsers();
              return;
            }
            const text = decoder.decode(value, { stream: true });
            // Lines might be split across chunks, but for simple status updates it's usually ok.
            // For better UI, we should split by newline.
            const lines = text.split('\n');
            lines.forEach(line => {
                if(line.trim() !== "") updateStatus(line);
            });
            read();
          });
        }
        read();
      })
      .catch(err => updateStatus("Error: " + err));
  }

  function deleteFinger(id) {
    if(!confirm("Delete ID " + id + "?")) return;
    fetch(`/api/delete?id=${id}`)
      .then(res => fetchUsers());
  }

  function deleteAll() {
    if(!confirm("Erase ALL fingerprints? This cannot be undone.")) return;
    fetch('/api/delete?all=true')
      .then(res => fetchUsers());
  }

  // Load initial data
  fetchUsers();
</script>
</body>
</html>
)rawliteral";

#endif
