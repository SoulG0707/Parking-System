var express = require("express");
var bodyParser = require("body-parser");
var cors = require("cors");
var mysql = require("mysql2"); //npm install mysql2
var app = express();
app.use(cors());
app.use(bodyParser.json());

var con = mysql.createConnection({
  host: "localhost",
  port: "3306",
  user: "root",
  password: "Ptyn07072004_",
  insecureAuth: true,
  database: "Parking_System",
});

con.connect(function (err) {
  if (err) throw err;
  console.log("Connected!!!");
  var sql = "SELECT * FROM lightsensor";
  con.query(sql, function (err, results) {
    if (err) throw err;
    console.log(results);
  });
});

app.get("/get", function (req, res) {
  const sql = "SELECT * FROM lightsensor";
  con.query(sql, function (err, results) {
    if (err) {
      console.error(err);
      return res.status(500).send("Database error!");
    }
    res.json(results);
  });
});

app.get("/get/:id", function (req, res) {
  const { id } = req.params;
  const sql = "SELECT * FROM lightsensor WHERE id = ?";
  console.log(sql);

  con.query(sql, [id], function (err, results) {
    if (err) {
      console.error(err);
      return res.status(500).send("Database error!");
    }
    if (results.length === 0) {
      return res.status(404).send("Record not found!");
    }
    res.json(results[0]);
  });
});

app.post("/add", function (req, res) {
  const { status, atTime } = req.body;
  console.log("Received data:", req.body);
  const sql = "INSERT INTO lightsensor (status, atTime) VALUES (?, ?)";
  console.log(sql);
  con.query(sql, [status, atTime], function (err, results) {
    if (err) {
      console.error("Error while inserting:", err);
      return res.status(500).send("Database error!");
    }
    res.send("Add success!");
  });
});

app.put("/update/:id", function (req, res) {
  const { id } = req.params;
  const { status, atTime } = req.body;
  const sql = "UPDATE lightsensor SET status = ?, atTime = ? WHERE id = ?";
  console.log(sql);

  con.query(sql, [status, atTime, id], function (err, results) {
    if (err) {
      console.error(err);
      return res.status(500).send("Database error!");
    }
    if (results.affectedRows === 0) {
      return res.status(404).send("Record not found!");
    }
    res.send("Update success!");
  });
});

app.delete("/delete/:id", function (req, res) {
  const { id } = req.params;
  const sql = "DELETE FROM lightsensor WHERE id = ?";
  console.log(sql);

  con.query(sql, [id], function (err, results) {
    if (err) {
      console.error(err);
      return res.status(500).send("Database error!");
    }
    if (results.affectedRows === 0) {
      return res.status(404).send("Record not found!");
    }
    res.send("Delete success!");
  });
});

// Biến toàn cục để lưu trạng thái cửa
let doorStatus = "closed";

// Endpoint để xử lý mở/đóng cửa
app.post("/door", function (req, res) {
  const { action } = req.body;
  if (action === "open" || action === "close") {
    doorStatus = action;
    console.log(`Door ${action}`);
    res.send(`Door ${action}`);
  } else {
    res.status(400).send("Invalid action");
  }
});

// Endpoint để kiểm tra trạng thái cửa
app.get("/door/status", function (req, res) {
  res.json({ action: doorStatus });
});

var server = app.listen(7777, function () {
  var host = server.address().address;
  var port = server.address().port;
  console.log("Server is listening at http://%s:%s", host, port);
});
