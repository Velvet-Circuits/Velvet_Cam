/**
 * On-camera tally / PTT / on-air GPI (USB CDC).
 *
 * Protocol (115200 8N1, line-oriented):
 *   Encoder → board:  T P=<0|1> V=<0|1>\n
 *                     C PTT=<n> PGM=<n> PVW=<n> AIR=<n>\n
 *   Board → Encoder:  P 1\n / P 0\n   (PTT pressed / released)
 *                     H\n             (hello on boot / USB open)
 *
 * Default wiring (until a C command arrives) — must match host defaults:
 *   PTT button: D3 → GND  (internal pull-up; pressed = LOW)
 *   Program LED: D2  (active HIGH)
 *   Preview LED: D8  (active HIGH)
 *   On-air GPI: D4   (open-drain: LOW/to GND when tally program ON, high-Z when off)
 *
 * Supported boards (same sketch):
 *   - Seeeduino XIAO / XIAO SAMD21 (3.3 V — preferred)
 *   - Arduino Leonardo / Pro Micro (ATmega32U4, 5 V)
 *
 * On-air wiring tip: use the pin as a low-side switch to ground (relay coil with
 * flyback diode, optocoupler LED+resistor, or camera GPI expecting contact-to-GND).
 * Do not short the pin directly to a voltage rail without a load. On XIAO stay at 3.3 V.
 */

int pin_ptt = 3;
int pin_program = 2;
int pin_preview = 8;
int pin_on_air = 4;

bool tally_program = false;
bool tally_preview = false;
bool ptt_pressed = false;
String rx_line;

int parseKvInt(const String& line, const char* key, int fallback)
{
  String k = String(key);
  int idx = line.indexOf(k);
  if (idx < 0) {
    return fallback;
  }
  idx += k.length();
  while (idx < (int)line.length() && (line.charAt(idx) == ' ' || line.charAt(idx) == '=')) {
    idx++;
  }
  return line.substring(idx).toInt();
}

void releasePin(int pin)
{
  pinMode(pin, INPUT);
  digitalWrite(pin, LOW);
}

void applyOnAir()
{
  // Open-drain style: closed to GND while on-program, floating otherwise.
  if (tally_program) {
    pinMode(pin_on_air, OUTPUT);
    digitalWrite(pin_on_air, LOW);
  } else {
    pinMode(pin_on_air, INPUT);
  }
}

void applyOutputs()
{
  digitalWrite(pin_program, tally_program ? HIGH : LOW);
  digitalWrite(pin_preview, tally_preview ? HIGH : LOW);
  applyOnAir();
}

void applyPinModes()
{
  pinMode(pin_ptt, INPUT_PULLUP);
  pinMode(pin_program, OUTPUT);
  pinMode(pin_preview, OUTPUT);
  applyOutputs();
}

void sendPtt(bool pressed)
{
  Serial.print(F("P "));
  Serial.println(pressed ? F("1") : F("0"));
}

bool pinsDistinct(int a, int b, int c, int d)
{
  return a != b && a != c && a != d && b != c && b != d && c != d;
}

void handleLine(String line)
{
  line.trim();
  if (line.length() == 0) {
    return;
  }
  if (line.startsWith(F("T "))) {
    int p_idx = line.indexOf(F("P="));
    int v_idx = line.indexOf(F("V="));
    if (p_idx >= 0 && p_idx + 2 < (int)line.length()) {
      tally_program = (line.charAt(p_idx + 2) == '1');
    }
    if (v_idx >= 0 && v_idx + 2 < (int)line.length()) {
      tally_preview = (line.charAt(v_idx + 2) == '1');
    }
    applyOutputs();
    return;
  }
  if (line.startsWith(F("C "))) {
    int next_ptt = parseKvInt(line, "PTT=", pin_ptt);
    int next_pgm = parseKvInt(line, "PGM=", pin_program);
    int next_pvw = parseKvInt(line, "PVW=", pin_preview);
    int next_air = parseKvInt(line, "AIR=", pin_on_air);
    if (next_ptt < 0 || next_ptt > 21 || next_pgm < 0 || next_pgm > 21 || next_pvw < 0
        || next_pvw > 21 || next_air < 0 || next_air > 21) {
      return;
    }
    if (!pinsDistinct(next_ptt, next_pgm, next_pvw, next_air)) {
      return;
    }
    // Release previous output pins (leave floating / input).
    releasePin(pin_program);
    releasePin(pin_preview);
    releasePin(pin_on_air);

    pin_ptt = next_ptt;
    pin_program = next_pgm;
    pin_preview = next_pvw;
    pin_on_air = next_air;
    ptt_pressed = false;
    applyPinModes();
    return;
  }
}

void setup()
{
  applyPinModes();

  Serial.begin(115200);
  unsigned long start = millis();
  while (!Serial && (millis() - start) < 2000) {
    /* wait for USB CDC */
  }
  Serial.println(F("H"));
}

void loop()
{
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n') {
      handleLine(rx_line);
      rx_line = "";
    } else if (c != '\r') {
      if (rx_line.length() < 80) {
        rx_line += c;
      } else {
        rx_line = "";
      }
    }
  }

  const bool now = (digitalRead(pin_ptt) == LOW);
  if (now != ptt_pressed) {
    ptt_pressed = now;
    sendPtt(ptt_pressed);
  }

  delay(1);
}
