#include "opensans10b.h"
#include "opensans14b.h"
#include "opensans24b.h"
#include "opensans8b.h"
#include "opensans9b.h"

#define WAVEFORM EPD_BUILTIN_WAVEFORM

#define White 0xFF
#define LightGrey 0xBB
#define Grey 0x88
#define DarkGrey 0x44
#define Black 0x00

#define STATUSX 200
#define STATUSY 528
#define STATUSW 600
#define STATUSH 25
#define STATUSC 50
#define DEBUGIX EPD_WIDTH - 20
#define DEBUGIY STATUSY - 30
#define DEBUGIH 20

#define TITLEY 80

enum alignment { LEFT,
                 RIGHT,
                 CENTER };

EpdiyHighlevelState hl;
EpdRotation orientation = EPD_ROT_LANDSCAPE;
EpdFont currentFont;
int ambient_temp = 22;

uint8_t *fb;
enum EpdDrawError err;
int cursor_x;
int cursor_y;

void drawString(int x, int y, String text, alignment align) {
  char *data = const_cast<char *>(text.c_str());
  EpdFontProperties font_props = epd_font_properties_default();
  if (align == CENTER) font_props.flags = EPD_DRAW_ALIGN_CENTER;
  if (align == LEFT) font_props.flags = EPD_DRAW_ALIGN_LEFT;
  if (align == RIGHT) font_props.flags = EPD_DRAW_ALIGN_RIGHT;
  epd_write_string(&currentFont, data, &x, &y, fb, &font_props);
}

void fillCircle(int x, int y, int r, uint8_t color) {
  epd_fill_circle(x, y, r, color, fb);
}

void drawFastHLine(int16_t x0, int16_t y0, int length, uint16_t color) {
  epd_draw_hline(x0, y0, length, color, fb);
}

void drawFastVLine(int16_t x0, int16_t y0, int length, uint16_t color) {
  epd_draw_vline(x0, y0, length, color, fb);
}

void drawCircle(int x0, int y0, int r, uint8_t color) {
  epd_draw_circle(x0, y0, r, color, fb);
}

void drawQrImage(int x, int y, int size, const uint8_t *data) {
  EpdRect qrrect = {
      .x = x,
      .y = y,
      .width = size,
      .height = size,
  };

  // epd_copy_to_framebuffer(qrrect, data, epd_hl_get_framebuffer(&hl));
  epd_draw_rotated_image(qrrect, data, fb);
}

void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  EpdRect area = {
      .x = x,
      .y = y,
      .width = w,
      .height = h,
  };
  epd_draw_rect(area, color, fb);
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  EpdRect area = {
      .x = x,
      .y = y,
      .width = w,
      .height = h,
  };
  epd_fill_rect(area, color, fb);
}

EpdRect getEdpArea(int x, int y, int w, int h) {
  EpdRect area = {
      .x = x,
      .y = y,
      .width = w,
      .height = h,
  };
  return area;
}

void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                  int16_t x2, int16_t y2, uint16_t color) {
  epd_fill_triangle(x0, y0, x1, y1, x2, y2, color, fb);
}

void drawPixel(int x, int y, uint8_t color) {
  epd_draw_pixel(x, y, color, fb);
}

void setFont(EpdFont const &font) {
  currentFont = font;
}

void epd_update() {
  epd_poweron();
  epd_hl_update_screen(&hl, MODE_GC16, ambient_temp);
  delay(500);
  epd_poweroff();
  delay(600);
}

void clearStatusMsg() {
  fillRect(STATUSX, STATUSY - STATUSH + 1, STATUSW, STATUSH + 3, White);
}

void renderStatusQueue(String msg) {
  setFont(OpenSans8B);
  clearStatusMsg();
  if (msg.length() > STATUSC) msg = msg.substring(0, STATUSC - 1) + "..";
  drawString(EPD_WIDTH / 2, STATUSY, msg, CENTER);
}

void renderStatusMsg(String msg) {
  clearStatusMsg();
  renderStatusQueue(msg);
  epd_update();
}

void renderPost(String title, String summary, String date = "", String author = "", const uint8_t *qr = nullptr, int qr_size = 0) {
  setFont(OpenSans14B);
  drawString(EPD_WIDTH / 2, 285, title, CENTER); 
  setFont(OpenSans10B);
  if (qr != nullptr && qr_size > 0) {
    drawString(60, 370, summary, LEFT);
    drawQrImage(EPD_WIDTH - 60 - qr_size, 330, qr_size, qr);
  }
  else {
    drawString(EPD_WIDTH / 2, 370, summary, CENTER); 
  }
  setFont(OpenSans8B);
  drawString(EPD_WIDTH / 2, 470, date + "  " + author, CENTER);
}

void eInkClear() {
  epd_fullclear(&hl, ambient_temp);
  epd_hl_set_all_white(&hl);
}

void eInkInit() {
  currentFont = OpenSans8B;
  epd_init(EPD_OPTIONS_DEFAULT);
  hl = epd_hl_init(WAVEFORM);
  epd_set_rotation(orientation);
  fb = epd_hl_get_framebuffer(&hl);
}
