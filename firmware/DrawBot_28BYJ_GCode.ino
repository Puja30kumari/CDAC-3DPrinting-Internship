/*
  DrawBot_28BYJ_GCode.ino
  Minimal G-code interpreter for 28BYJ-48 steppers via ULN2003 drivers and a pen servo.
  Supports: G0/G1 with X Y Z F, M3 (pen down), M5 (pen up)
  Baud: 115200
*/

#include <Servo.h>

// ---------------- Pins ----------------
const int X_IN1 = 2,  X_IN2 = 3,  X_IN3 = 4,  X_IN4 = 5;
const int Y_IN1 = 6,  Y_IN2 = 7,  Y_IN3 = 8,  Y_IN4 = 9;
const int Z_IN1 = 10, Z_IN2 = 11, Z_IN3 = 12, Z_IN4 = 13;
const int SERVO_PIN = A0; // use analog A0 as digital pin for servo

// ---------------- Config --------------
const float STEPS_PER_MM_X = 40.0f;  // tune for your mechanics
const float STEPS_PER_MM_Y = 40.0f;
const float STEPS_PER_MM_Z = 200.0f;

const int PEN_UP_ANGLE = 30;
const int PEN_DOWN_ANGLE = 70;

const int STEP_DELAY_US_BASE = 1200; // base microseconds between phase steps (lower is faster)

// 28BYJ-48 8-step half-stepping sequence
const int sequence[8][4] = {
  {1,0,0,0},
  {1,1,0,0},
  {0,1,0,0},
  {0,1,1,0},
  {0,0,1,0},
  {0,0,1,1},
  {0,0,0,1},
  {1,0,0,1}
};

// ---------------- State ----------------
Servo pen;
float curX = 0, curY = 0, curZ = 0;
float feedrate_mm_min = 600.0; // default
int phaseX = 0, phaseY = 0, phaseZ = 0;

// ---------------- Helpers -------------
void driveCoils(int in1, int in2, int in3, int in4, int p) {
  digitalWrite(in1, sequence[p][0]);
  digitalWrite(in2, sequence[p][1]);
  digitalWrite(in3, sequence[p][2]);
  digitalWrite(in4, sequence[p][3]);
}

void stepMotorAxis(char axis, int steps, int dir) {
  int *phase;
  int in1, in2, in3, in4;
  if (axis == 'X') { phase=&phaseX; in1=X_IN1; in2=X_IN2; in3=X_IN3; in4=X_IN4; }
  else if (axis == 'Y') { phase=&phaseY; in1=Y_IN1; in2=Y_IN2; in3=Y_IN3; in4=Y_IN4; }
  else { phase=&phaseZ; in1=Z_IN1; in2=Z_IN2; in3=Z_IN3; in4=Z_IN4; }

  for (int i=0; i<steps; i++) {
    *phase = (*phase + (dir>0 ? 1 : -1) + 8) % 8;
    driveCoils(in1, in2, in3, in4, *phase);
    delayMicroseconds(STEP_DELAY_US_BASE);
  }
}

void lineTo(float x1, float y1, float z1) {
  // Bresenham-like naive synchronized stepping
  long dx = lround((x1 - curX) * STEPS_PER_MM_X);
  long dy = lround((y1 - curY) * STEPS_PER_MM_Y);
  long dz = lround((z1 - curZ) * STEPS_PER_MM_Z);

  long ax = abs(dx), ay = abs(dy), az = abs(dz);
  long n = max(ax, max(ay, az));
  if (n == 0) return;

  float stepx = (float)dx / (float)n;
  float stepy = (float)dy / (float)n;
  float stepz = (float)dz / (float)n;

  float ex=0, ey=0, ez=0;
  int sx = (dx>=0)?1:-1;
  int sy = (dy>=0)?1:-1;
  int sz = (dz>=0)?1:-1;

  float mm_per_min = feedrate_mm_min;
  float mm_per_step = 1.0; // simplistic pacing
  int us = (int)(60000000.0 * mm_per_step / max(mm_per_min, 1.0)); // naive

  for (long i=0; i<n; i++) {
    ex += stepx; ey += stepy; ez += stepz;
    if (fabs(ex) >= 1.0) { stepMotorAxis('X', 1, sx); ex -= (ex>0?1:-1); }
    if (fabs(ey) >= 1.0) { stepMotorAxis('Y', 1, sy); ey -= (ey>0?1:-1); }
    if (fabs(ez) >= 1.0) { stepMotorAxis('Z', 1, sz); ez -= (ez>0?1:-1); }
    if (us > 0) delayMicroseconds(us);
  }

  curX = x1; curY = y1; curZ = z1;
}

void penUp()   { pen.write(PEN_UP_ANGLE);   delay(200); }
void penDown() { pen.write(PEN_DOWN_ANGLE); delay(200); }

// ---------------- Parsing --------------
bool hasWord(const String &s, char w) {
  int i = s.indexOf(w);
  if (i < 0) return false;
  if (i+1 >= s.length()) return false;
  return true;
}
float getWord(const String &s, char w, float defV) {
  int i = s.indexOf(w);
  if (i < 0) return defV;
  int j = i+1;
  while (j < s.length() && (s[j]==' ')) j++;
  return s.substring(j).toFloat();
}

void processLine(String line) {
  line.trim();
  line.toUpperCase();
  if (line.length()==0) return;

  if (line.startsWith("G0") || line.startsWith("G1")) {
    float nx = hasWord(line,'X') ? getWord(line,'X',curX) : curX;
    float ny = hasWord(line,'Y') ? getWord(line,'Y',curY) : curY;
    float nz = hasWord(line,'Z') ? getWord(line,'Z',curZ) : curZ;
    if (hasWord(line,'F')) feedrate_mm_min = max(1.0f, getWord(line,'F',feedrate_mm_min));
    lineTo(nx, ny, nz);
    Serial.println("ok");
    return;
  }
  if (line.startsWith("M3")) { penDown(); Serial.println("ok"); return; }
  if (line.startsWith("M5")) { penUp();   Serial.println("ok"); return; }
  if (line.startsWith(";") || line.startsWith("(")) { Serial.println("ok"); return; } // comment

  Serial.println("unknown");
}

void setup() {
  pinMode(X_IN1, OUTPUT); pinMode(X_IN2, OUTPUT); pinMode(X_IN3, OUTPUT); pinMode(X_IN4, OUTPUT);
  pinMode(Y_IN1, OUTPUT); pinMode(Y_IN2, OUTPUT); pinMode(Y_IN3, OUTPUT); pinMode(Y_IN4, OUTPUT);
  pinMode(Z_IN1, OUTPUT); pinMode(Z_IN2, OUTPUT); pinMode(Z_IN3, OUTPUT); pinMode(Z_IN4, OUTPUT);
  pen.attach(SERVO_PIN);
  penUp();
  Serial.begin(115200);
  Serial.println("DrawBot ready");
}

void loop() {
  static String buf="";
  while (Serial.available()) {
    char c = Serial.read();
    if (c=='\n' || c=='\r') {
      if (buf.length()>0) { processLine(buf); buf=""; }
    } else {
      buf += c;
    }
  }
}
