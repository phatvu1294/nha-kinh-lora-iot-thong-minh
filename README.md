# Nhà kính LoRa IOT thông minh 
*Nhà kính LoRa IOT thông minh được phát triển dựa trên ESP8266, Arduino, LoRa*  

### Giới thiệu
Hệ thống bao gồm 2 Node có nhiệm vụ thu thập dữ liệu sau đó gửi về Gateway thông qua công nghệ LoRa. Dữ liệu này sẽ được Gateway xử lý và đẩy lên Website thông qua giao thức MQTT. Giao diện Website được thiết kế với Blogger cho phép giám sát các thông số ở bất kỳ nơi đâu mà chúng ta có internet.

![Nhà kính LoRa IOT thông minh](/images/image-01.png)

### Tính năng
- Với IOT chúng ta có thể theo dõi thiết bị ở mọi nơi miễn là có internet
- Với LoRa chúng ta có khoảng cách truyền lên đến hàng kilomet
- Giao diện Website trực quan, dễ nhìn, tên miền blogspot miễn phí
- Phím nhấn cho phép cài đặt WiFi của Gateway 1 cách dễ dàng
- Mạch một lớp giúp dễ dàng làm thủ công, thích hợp với sinh viên nghiên cứu khoa học

### Thông số kỹ thuật
#### Node
- Điện áp vào: 9-12VDC
- Màn hình hiển thị: LCD1602
- Socket cảm biến: DHT11/DHT22/LM35/CB Đất

#### Gateway
- Điện áp vào: 9-12VDC
- Socket hiển thị: LCD I2C
