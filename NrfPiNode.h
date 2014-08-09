struct payload_t
{
  char type;
  uint8_t sensor;
  uint8_t value_high;
  uint8_t value_low;
  uint8_t options;
};

struct __attribute__((packed)) input_msg{
    uint16_t nodeid;
    char header_type;
    char payload[77];
};

// OTA Config load payload
struct config_payload_t
{
  uint8_t pos;
  uint8_t data;
};


struct __attribute__((packed)) ws2801_payload_t
{
  uint8_t led;
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t led_h;
};
