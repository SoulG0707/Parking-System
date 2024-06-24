-- Tạo cơ sở dữ liệu
CREATE DATABASE Parking_System;

-- Sử dụng cơ sở dữ liệu vừa tạo
USE Parking_System;

-- Tạo bảng lightsensor
CREATE TABLE lightsensor (
    id INT PRIMARY KEY AUTO_INCREMENT,
    status VARCHAR(10),
    atTime DATETIME
);

-- Thêm dữ liệu mẫu vào bảng lightsensor
INSERT INTO lightsensor (status, atTime) VALUES ('opened', '2024-12-05');

-- Truy vấn để xem tất cả dữ liệu trong bảng lightsensor
SELECT * FROM lightsensor;

DELETE FROM lightsensor WHERE id > 2;
ALTER TABLE lightsensor AUTO_INCREMENT = 1;

