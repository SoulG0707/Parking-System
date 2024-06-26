const express = require("express");
const bodyParser = require("body-parser");
const cors = require("cors");
const mysql = require("mysql2");

const app = express();
const PORT = 7777;

// Sử dụng middleware
app.use(cors());
app.use(bodyParser.json());

// Kết nối CSDL MySQL
const con = mysql.createConnection({
  host: "localhost",
  port: "3306",
  user: "root",
  password: "Ptyn07072004_", // Thay đổi thành mật khẩu MySQL của bạn
  database: "Parking_System",
});

// Kiểm tra kết nối tới CSDL
con.connect((err) => {
  if (err) {
    console.error("Lỗi kết nối CSDL:", err.stack);
    return;
  }
  console.log("Đã kết nối tới CSDL MySQL!");

  // Truy vấn ban đầu để kiểm tra kết nối
  const sql = "SELECT * FROM lightsensor";
  con.query(sql, (err, results) => {
    if (err) {
      console.error("Lỗi truy vấn CSDL:", err.stack);
      return;
    }
    console.log("Dữ liệu ban đầu từ bảng lightsensor:", results);
  });
});

// Route lấy tất cả dữ liệu từ bảng lightsensor
app.get("/get", (req, res) => {
  const sql = "SELECT * FROM lightsensor";
  con.query(sql, (err, results) => {
    if (err) {
      console.error("Lỗi truy vấn CSDL:", err.stack);
      return res.status(500).send("Lỗi CSDL!");
    }
    res.json(results);
  });
});

// Route lấy dữ liệu từ lightsensor theo ID
app.get("/get/:id", (req, res) => {
  const { id } = req.params;
  const sql = "SELECT * FROM lightsensor WHERE id = ?";
  con.query(sql, [id], (err, results) => {
    if (err) {
      console.error("Lỗi truy vấn CSDL:", err.stack);
      return res.status(500).send("Lỗi CSDL!");
    }
    if (results.length === 0) {
      return res.status(404).send("Không tìm thấy bản ghi!");
    }
    res.json(results[0]);
  });
});

// Route thêm dữ liệu mới vào lightsensor
app.post("/add", (req, res) => {
  const { status, atTime } = req.body;
  if (!status || !atTime) {
    return res.status(400).send("Thiếu tham số: status hoặc atTime");
  }
  const sql = "INSERT INTO lightsensor (status, atTime) VALUES (?, ?)";
  con.query(sql, [status, atTime], (err, results) => {
    if (err) {
      console.error("Lỗi khi thêm vào CSDL:", err.stack);
      return res.status(500).send("Lỗi CSDL!");
    }
    res.send("Thêm dữ liệu thành công!");
  });
});

// Route cập nhật dữ liệu trong lightsensor theo ID
app.put("/update/:id", (req, res) => {
  const { id } = req.params;
  const { status, atTime } = req.body;
  if (!status || !atTime) {
    return res.status(400).send("Thiếu tham số: status hoặc atTime");
  }
  const sql = "UPDATE lightsensor SET status = ?, atTime = ? WHERE id = ?";
  con.query(sql, [status, atTime, id], (err, results) => {
    if (err) {
      console.error("Lỗi khi cập nhật CSDL:", err.stack);
      return res.status(500).send("Lỗi CSDL!");
    }
    if (results.affectedRows === 0) {
      return res.status(404).send("Không tìm thấy bản ghi!");
    }
    res.send("Cập nhật dữ liệu thành công!");
  });
});

// Route xóa dữ liệu từ lightsensor theo ID
app.delete("/delete/:id", (req, res) => {
  const { id } = req.params;
  const sql = "DELETE FROM lightsensor WHERE id = ?";
  con.query(sql, [id], (err, results) => {
    if (err) {
      console.error("Lỗi khi xóa từ CSDL:", err.stack);
      return res.status(500).send("Lỗi CSDL!");
    }
    if (results.affectedRows === 0) {
      return res.status(404).send("Không tìm thấy bản ghi!");
    }
    res.send("Xóa dữ liệu thành công!");
  });
});

// Endpoint quản lý trạng thái cửa
let doorStatus = "closed";

// Route thay đổi trạng thái cửa
app.post("/door/status", (req, res) => {
  const { action } = req.body;
  if (action === "open" || action === "close") {
    doorStatus = action;
    console.log(`Cửa được ${action}`);
    res.send(`Cửa được ${action}`);
  } else {
    res.status(400).send("Hành động không hợp lệ");
  }
});

// Route lấy trạng thái cửa
app.get("/door/status", (req, res) => {
  res.json({ action: doorStatus });
});

// Khởi động server
app.listen(PORT, () => {
  console.log(`Server đang lắng nghe tại http://localhost:${PORT}`);
});
