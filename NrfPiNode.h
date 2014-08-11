struct payload_t
{
  char type;
  uint8_t sensor;
  unsigned char b0;
  unsigned char b1;
  unsigned char b2;
  unsigned char b3;
  float value; 
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
