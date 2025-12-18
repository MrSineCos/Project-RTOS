# ESP32 RTOS Integrated Project (PlatformIO + Arduino)

Project này tích hợp nhiều tác vụ (task) chạy song song trên ESP32 bằng FreeRTOS (đã có sẵn trong Arduino-ESP32):

- Điều khiển LED D3 (blink theo trạng thái)
- Điều khiển NeoPixel (rainbow khi bật)
- Đọc nhiệt độ/độ ẩm từ DHT (DHT11 hoặc DHT20) và hiển thị lên OLED I2C
- Web dashboard (AP mode + chuyển sang STA theo thông tin nhập trên trang Settings)
- TinyML inference (TensorFlow Lite Micro) dựa trên dữ liệu nhiệt độ/độ ẩm

## Kiến trúc RTOS

Các task được tạo trong [src/main.cpp](src/main.cpp#L1):

| Task | Hàm | Stack | Priority | Chu kỳ | Mục đích |
|---|---|---:|---:|---:|---|
| Task LED Blink | `led_blinky()` | 2048 | 2 | 1s | Điều khiển LED D3 theo `led_D3` |
| Task NEO Blink | `neo_blinky()` | 2048 | 2 | ~ liên tục | Hiệu ứng NeoPixel theo `led_NEO` |
| Task TEMP HUMI Monitor | `temp_humi_monitor()` | 2048 | 2 | 5s | Đọc DHT, cập nhật global, hiển thị OLED |
| Task Main Server | `main_server_task()` | 8192 | 2 | 20ms | WebServer + Wi‑Fi mode, cập nhật `led_D3/led_NEO` |
| Tiny ML Task | `tiny_ml_task()` | 2048 | 2 | 5s | Inference TFLM từ `glob_temperature/glob_humidity` |

### Luồng dữ liệu giữa các task

- Sensor task cập nhật `glob_temperature` và `glob_humidity` (global).
- Web task đọc `glob_temperature/glob_humidity` để trả API `/sensors`.
- TinyML task đọc `glob_temperature/glob_humidity` làm input model.
- Web task cập nhật `led_D3` và `led_NEO` dựa trên trạng thái nút bấm trên dashboard.

Các biến dùng chung nằm ở:
- [include/global.h](include/global.h)
- [src/global.cpp](src/global.cpp)

## Web dashboard

Web server chạy ở port `80`.

### AP mode (mặc định khi boot)

- SSID: `MY-ESP32-NETWORK`
- Password: `12345678`
- Truy cập: `http://192.168.4.1/`

Dashboard:
- `/` xem nhiệt độ/độ ẩm và bật/tắt LED1/LED2 (map sang `led_D3` và `led_NEO`).
- `/settings` nhập Wi‑Fi STA để thiết bị kết nối vào router.

### Chuyển về AP mode

Trong [src/mainserver.cpp](src/mainserver.cpp) có xử lý nút `BOOT_PIN` (GPIO0). Khi nhấn giữ, thiết bị sẽ chuyển về AP mode.

## Cảm biến + OLED

### Chọn loại DHT

Chỉnh macro trong [include/temp_humi_monitor.h](include/temp_humi_monitor.h):

```cpp
#define DHT_TYPE DHT_TYPE_DHT11
// hoặc
// #define DHT_TYPE DHT_TYPE_DHT20
```

### OLED I2C

Trong [src/temp_humi_monitor.cpp](src/temp_humi_monitor.cpp):

- OLED SSD1306 địa chỉ `0x3C`
- I2C pins: `Wire.begin(11, 12)` (SDA=11, SCL=12 theo code hiện tại)

## TinyML (TensorFlow Lite Micro)

- Model được nhúng trong [src/dht_anomaly_model.h](src/dht_anomaly_model.h)
- Input hiện tại: `input->data.f[0] = glob_temperature`, `input->data.f[1] = glob_humidity`
- Output in ra Serial mỗi 5 giây

Ghi chú: `kTensorArenaSize = 8*1024` trong [src/tinyml.cpp](src/tinyml.cpp). Nếu sau này thay model nặng hơn và gặp lỗi `AllocateTensors() failed`, hãy tăng kích thước arena.

## Build / Upload / Monitor

Project dùng PlatformIO environment: `env:yolo_uno` (xem [platformio.ini](platformio.ini)).

Chạy các lệnh sau trong thư mục project:

- Build: `pio run`
- Upload: `pio run -t upload`
- Serial monitor: `pio device monitor -b 115200`

## Cấu hình phần cứng (pins)

Các chân đang dùng theo header:

- LED D3: `LED_GPIO = 48` ([include/led_blinky.h](include/led_blinky.h))
- NeoPixel: `NEO_PIN = 45` ([include/neo_blinky.h](include/neo_blinky.h))
- BOOT button: `BOOT_PIN = 0` ([include/mainserver.h](include/mainserver.h))
- DHT11 data: `DHT_PIN = GPIO_NUM_6` ([include/temp_humi_monitor.h](include/temp_humi_monitor.h))

## Thư viện phụ thuộc

Các lib chính được khai báo trong [platformio.ini](platformio.ini):

- `SimpleDHT` (cho DHT11)
- `DHT20` (thư viện nội bộ trong `lib/`)
- `Adafruit SSD1306` + `Adafruit GFX` (OLED)
- `Adafruit NeoPixel`
- `TensorFlowLite_ESP32`

## Troubleshooting nhanh

- Không thấy trang web: kiểm tra đang ở AP `MY-ESP32-NETWORK`, truy cập `http://192.168.4.1/`.
- OLED không lên: kiểm tra địa chỉ I2C `0x3C` và dây SDA/SCL đúng với `Wire.begin(11, 12)`.
- Nhiệt độ/độ ẩm = `-1`: sensor đọc lỗi, kiểm tra loại DHT và chân `DHT_PIN`.
- Reset/WDT: tránh `while(1)` không delay; web task đã có `vTaskDelay(20)`.