#include <Arduino.h>
#include <lvgl.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "FT6236.h"
#include "esp_dmx.h"
#include "Preferences.h"

const int i2c_touch_addr = TOUCH_I2C_ADD;

Preferences preferences;

lv_obj_t *keyboard;

  #define channel_no 10

  lv_obj_t *panel[channel_no];
  lv_obj_t *label[channel_no];
  lv_obj_t *selector[channel_no];
  lv_obj_t *base_inputfield[channel_no];
  lv_obj_t *amplitude_inputfield[channel_no];
  lv_obj_t *longtitude_inputfield[channel_no];
  int base_value[channel_no];
  int amp_value[channel_no];
  int longt_value[channel_no];

#define LCD_BL 46

#define SDA_FT6236 38
#define SCL_FT6236 39
// FT6236 ts = FT6236();

/* Now we want somewhere to store our DMX data. Since a single packet of DMX
  data can be up to 513 bytes long, we want our array to be at least that long.
  This library knows that the max DMX packet size is 513, so we can fill in the
  array size with `DMX_PACKET_SIZE`. */
byte data[DMX_PACKET_SIZE];

/* This variable will allow us to update our packet and print to the Serial
  Monitor at a regular interval. */
unsigned long lastUpdate = millis();

/* Next, lets decide which DMX port to use. The ESP32 has either 2 or 3 ports.
  Port 0 is typically used to transmit serial data back to your Serial Monitor,
  so we shouldn't use that port. Lets use port 1! */
dmx_port_t dmxPort = 1;

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ILI9488 _panel_instance;
  lgfx::Bus_Parallel16 _bus_instance;

public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();
      cfg.port = 0;
      cfg.freq_write = 80000000;
      cfg.pin_wr = 18;
      cfg.pin_rd = 48;
      cfg.pin_rs = 45;

      cfg.pin_d0 = 47;
      cfg.pin_d1 = 21;
      cfg.pin_d2 = 14;
      cfg.pin_d3 = 13;
      cfg.pin_d4 = 12;
      cfg.pin_d5 = 11;
      cfg.pin_d6 = 10;
      cfg.pin_d7 = 9;
      cfg.pin_d8 = 3;
      cfg.pin_d9 = 8;
      cfg.pin_d10 = 16;
      cfg.pin_d11 = 15;
      cfg.pin_d12 = 7;
      cfg.pin_d13 = 6;
      cfg.pin_d14 = 5;
      cfg.pin_d15 = 4;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();

      cfg.pin_cs = -1;
      cfg.pin_rst = -1;
      cfg.pin_busy = -1;
      cfg.memory_width = 320;
      cfg.memory_height = 480;
      cfg.panel_width = 320;
      cfg.panel_height = 480;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 2;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = true;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.dlen_16bit = true;
      cfg.bus_shared = true;
      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance);
  }
};

LGFX tft;
/*Change to your screen resolution*/
static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 320;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 5];

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  int pos[2] = {0, 0};

  ft6236_pos(pos);
  if (pos[0] > 0 && pos[1] > 0)
  {
    data->state = LV_INDEV_STATE_PR;
    //    data->point.x = tft.width()-pos[1];
    //    data->point.y = pos[0];
    data->point.x = tft.width() - pos[1];
    data->point.y = pos[0];
    Serial.printf("x-%d,y-%d\n", data->point.x, data->point.y);
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }
}

void touch_init()
{
  // I2C init
  Wire.begin(SDA_FT6236, SCL_FT6236);
  byte error, address;

  Wire.beginTransmission(i2c_touch_addr);
  error = Wire.endTransmission();

  if (error == 0)
  {
    Serial.print("I2C device found at address 0x");
    Serial.print(i2c_touch_addr, HEX);
    Serial.println("  !");
  }
  else if (error == 4)
  {
    Serial.print("Unknown error at address 0x");
    Serial.println(i2c_touch_addr, HEX);
  }
}
static void ta_event_cb(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  if (code == LV_EVENT_FOCUSED)
  {
    lv_keyboard_set_textarea(keyboard, ta);
  }

  if (code == LV_EVENT_DEFOCUSED)
  {
    lv_keyboard_set_textarea(keyboard, NULL);
  }

  if (code == LV_EVENT_VALUE_CHANGED)
  {
    Serial.println("text changed");
    for(int i = 0; i < channel_no; i++)
    {
      base_value[i] = atoi(lv_textarea_get_text(base_inputfield[i]));
      amp_value[i] = atoi(lv_textarea_get_text(amplitude_inputfield[i]));
      longt_value[i] = atoi(lv_textarea_get_text(longtitude_inputfield[i]));

      preferences.putBytes("SavedIntegers", (byte*)(&base_value), sizeof(base_value));
      /*
      preferences.putInt("Dings", base_value[i]);
      preferences.putInt("Dings", amp_value[i]);
      preferences.putInt("Dings", longt_value[i]);*/
    }
  }
}

byte calculate_anim(String function_name, int base_value, int amp_value, int long_value){
  if(function_name = "Static")
    if(base_value < 0)
    {
      return 0;
    }
    else if(base_value > 255)
    {
      return 255;
    }
    else
    return base_value;
  else
    return 0;
}

void setup()
{
  preferences.begin("debugNVM", false);
  preferences.getBytes("SavedIntegers", base_value, sizeof(base_value));
  Serial.begin(115200); /* prepare for possible serial debug */

  tft.begin();        /* TFT init */
  tft.setRotation(1); /* Landscape orientation, flipped */
  tft.fillScreen(TFT_BLACK);
  delay(500);
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);
  touch_init();

  //  if (!ts.begin(0, SDA_FT6236, SCL_FT6236)) {
  //    Serial.println("Unable to start the capacitive touch Screen.");
  //  }
  touch_init();

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 5);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  /*Create an object with the new style*/
  lv_obj_t *flexbox = lv_obj_create(lv_scr_act());
  lv_obj_set_size(flexbox, lv_pct(100), 200);
  lv_obj_align(flexbox, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_flex_flow(flexbox, LV_FLEX_FLOW_COLUMN);
  keyboard = lv_keyboard_create(lv_scr_act());
  lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_NUMBER);
  lv_obj_set_size(keyboard, 180, 120);
  lv_obj_align(keyboard, LV_ALIGN_BOTTOM_LEFT, 0, 0);

  // Create Flexbox Elements
  char buffer [10];
  for (int i = 0; i < channel_no; i++)
  {
    panel[i] = lv_obj_create(flexbox);
    lv_obj_set_size(panel[i], lv_pct(100), 80);
    lv_obj_set_flex_flow(panel[i], LV_FLEX_FLOW_ROW);

    label[i] = lv_label_create(panel[i]);
    lv_obj_set_size(label[i], 28, 40);
    itoa(i + 1, buffer, 10);
    lv_label_set_text(label[i], buffer);

    selector[i] = lv_dropdown_create(panel[i]);
    lv_dropdown_set_options(selector[i], "Static");
    lv_obj_set_size(selector[i], 100, 40);

    base_inputfield[i] = lv_textarea_create(panel[i]);
    itoa(base_value[i], buffer, 10);
    lv_textarea_add_text(base_inputfield[i], buffer);
    lv_obj_add_event_cb(base_inputfield[i], ta_event_cb, LV_EVENT_ALL, keyboard);
    lv_textarea_set_one_line(base_inputfield[i], true);
    lv_obj_set_size(base_inputfield[i], 80, 40);
    lv_textarea_set_accepted_chars(base_inputfield[i], "0123456789+-");
    lv_textarea_set_max_length(base_inputfield[i], 4);

    amplitude_inputfield[i] = lv_textarea_create(panel[i]);
    lv_obj_add_event_cb(amplitude_inputfield[i], ta_event_cb, LV_EVENT_ALL, keyboard);
    lv_textarea_set_one_line(amplitude_inputfield[i], true);
    lv_obj_set_size(amplitude_inputfield[i], 80, 40);
    lv_textarea_set_accepted_chars(amplitude_inputfield[i], "0123456789+-");
    lv_textarea_set_max_length(amplitude_inputfield[i], 4);

    longtitude_inputfield[i] = lv_textarea_create(panel[i]);
    lv_obj_add_event_cb(longtitude_inputfield[i], ta_event_cb, LV_EVENT_ALL, keyboard);
    lv_textarea_set_one_line(longtitude_inputfield[i], true);
    lv_obj_set_size(longtitude_inputfield[i], 80, 40);
    lv_textarea_set_accepted_chars(longtitude_inputfield[i], "0123456789+-");
    lv_textarea_set_max_length(longtitude_inputfield[i], 4);
  }

  // First, use the default DMX configuration...
  dmx_config_t config = DMX_CONFIG_DEFAULT;

  // ...declare the driver's DMX personalities...
  const int personality_count = 1;
  dmx_personality_t personalities[] = {
      {1, "Default Personality"}};

  // ...install the DMX driver...
  dmx_driver_install(dmxPort, &config, personalities, personality_count);

  // ...and then set the communication pins!
  const int tx_pin = 0;
  const int rx_pin = 0;
  const int rts_pin = 40;
  dmx_set_pin(dmxPort, tx_pin, rx_pin, rts_pin);
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
  
  for(int i = 0; i < channel_no; i++){
    data[i+1] = calculate_anim("Static", base_value[i], amp_value[i], longt_value[i]);
  }
  
  dmx_send(dmxPort);

  dmx_write(dmxPort, data, DMX_PACKET_SIZE);

  /* Log our changes to the Serial Monitor. */
  Serial.printf("Sending DMX 0x%02X\n", data[1]);

  /* We can do some other work here if we want. */

  /* If we have no more work to do, we will wait until we are done sending our
    DMX packet. */
  dmx_wait_sent(dmxPort, DMX_TIMEOUT_TICK);
  dmx_write(dmxPort, data, DMX_PACKET_SIZE);
  delay(5);
}
