<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="refresh" content="5"> <!-- Auto reload setiap 5 detik -->
    <title>Data MQTT</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f4f6f9;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            padding: 20px;
        }
        .container {
            width: 100%;
            max-width: 800px;
            background-color: #fff;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0px 0px 15px rgba(0, 0, 0, 0.1);
        }
        h1, h2 {
            text-align: center;
            color: #333;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 20px 0;
        }
        th, td {
            padding: 12px;
            border: 1px solid #ddd;
            text-align: center;
        }
        th {
            background-color: #f8f9fc;
            color: #555;
        }
        tr:nth-child(even) {
            background-color: #f9fafb;
        }
        .success {
            color: green;
            font-weight: bold;
            text-align: center;
            margin-top: 10px;
        }
        .error {
            color: red;
            font-weight: bold;
            text-align: center;
            margin-top: 10px;
        }
    </style>
</head>
<body>
    <div class="container">
        <?php
        // Pengaturan koneksi database
        $servername = "localhost";
        $username = "root";
        $password = "";
        $dbname = "uts";

        // Membuat koneksi ke database
        $conn = new mysqli($servername, $username, $password, $dbname);

        // Periksa koneksi
        if ($conn->connect_error) {
            die("<p class='error'>Koneksi ke database gagal: " . $conn->connect_error . "</p>");
        }

        // URL API Python Flask
        $api_url = "http://localhost:5000/mqtt_data";

        // Mengambil data dari API JSON
        $response = file_get_contents($api_url);
        $data = json_decode($response, true);

        // Menyimpan data ke dalam database
        if ($data) {
            $timestamp = date("Y-m-d H:i:s");
            $temperature = $data['temperature'] ?? null;
            $humidity = $data['humidity'] ?? null;

            // Query untuk menyimpan data ke dalam tabel "mqtt_data_history"
            $sql = "INSERT INTO utsiot (timestamp, temperature, humidity) VALUES ('$timestamp', '$temperature', '$humidity')";

            if ($conn->query($sql) === TRUE) {
                echo "<p class='success'>Data berhasil disimpan ke database.</p>";
            } else {
                echo "<p class='error'>Error menyimpan data: " . $conn->error . "</p>";
            }

            // Menampilkan data terbaru dalam bentuk tabel
            echo "<h1>Data MQTT Terbaru</h1>";
            echo "<table>";
            echo "<tr><th>Timestamp</th><th>Temperature</th><th>Humidity</th></tr>";
            echo "<tr><td>$timestamp</td><td>$temperature</td><td>$humidity</td></tr>";
            echo "</table>";
        } else {
            echo "<p class='error'>No data available</p>";
        }

        // Menampilkan histori data
        $result = $conn->query("SELECT * FROM utsiot ORDER BY timestamp DESC");
        if ($result->num_rows > 0) {
            echo "<h2>Histori Data</h2>";
            echo "<table>";
            echo "<tr><th>Timestamp</th><th>Temperature</th><th>Humidity</th></tr>";
            while ($row = $result->fetch_assoc()) {
                echo "<tr><td>{$row['timestamp']}</td><td>{$row['temperature']}</td><td>{$row['humidity']}</td></tr>";
            }
            echo "</table>";
        } else {
            echo "<p class='error'>Tidak ada histori data tersedia.</p>";
        }

        // Menutup koneksi
        $conn->close();
        ?>
    </div>
</body>
</html>
