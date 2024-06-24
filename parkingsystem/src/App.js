import React, { useState, useEffect } from "react";
import axios from "axios";
import "./App.css";

function App() {
  const [sensorData, setSensorData] = useState([]);

  useEffect(() => {
    axios
      .get("http://10.106.22.97:7777/get")
      .then((response) => {
        setSensorData(response.data);
      })
      .catch((error) => {
        console.error("Có lỗi xảy ra khi lấy dữ liệu:", error);
      });
  }, []);

  const handleCloseDoor = () => {
    console.log("Đóng cửa");
    axios
      .post("http://10.106.22.97:7777/door", { action: "close" })
      .then((response) => {
        console.log("Đã gửi yêu cầu đóng cửa:", response.data);
      })
      .catch((error) => {
        console.error("Có lỗi xảy ra khi gửi yêu cầu đóng cửa:", error);
      });
  };

  const handleOpenDoor = () => {
    console.log("Mở cửa");
    axios
      .post("http://10.106.22.97:7777/door", { action: "open" })
      .then((response) => {
        console.log("Đã gửi yêu cầu mở cửa:", response.data);
      })
      .catch((error) => {
        console.error("Có lỗi xảy ra khi gửi yêu cầu mở cửa:", error);
      });
  };

  return (
    <div className="App">
      <header className="App-header">
        <h1>Parking System Data</h1>
        <h4>by Pham Thi Yen Ngoc</h4>
        <table>
          <thead>
            <tr>
              <th>ID</th>
              <th>Status</th>
              <th>Time</th>
            </tr>
          </thead>
          <tbody>
            {sensorData.map((item) => (
              <tr key={item.id}>
                <td>{item.id}</td>
                <td>{item.status}</td>
                <td>{item.atTime}</td>
              </tr>
            ))}
          </tbody>
        </table>
        <div className="button-container">
          <button className="open-button" onClick={handleOpenDoor}>MỞ CỬA</button>
          <button className="close-button" onClick={handleCloseDoor}>ĐÓNG CỬA</button>
        </div>
      </header>
    </div>
  );
}

export default App;
