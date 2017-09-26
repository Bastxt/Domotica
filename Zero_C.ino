int ZC = 17;
int Z;
int up = 0;
int Data;
int Z_OUT = 18;
float TRIAC_TIME = 1;
int Time = 0;
int dimmer =0;

String SET = "";
int d;

void setup() {
  Serial.begin(115200);
  pinMode(ZC, INPUT);
  pinMode(Z_OUT, OUTPUT);
}

void loop() {
  if (Serial.available())
  {
    SET = Serial.readStringUntil('\n');
    if (SET.equals("DIMMER")) {
      dimmer = Serial.readStringUntil('\n').toInt();
      d=0;
    }
  }

  Time = (dimmer * 8400) / 100;
  Z = digitalRead(ZC);

  if (Z >= 1 && up == 0) {
    delayMicroseconds(Time);
    digitalWrite(Z_OUT, HIGH);
    delayMicroseconds(TRIAC_TIME);
    digitalWrite(Z_OUT, LOW);
    up = 1;
  }
  if (Z < 1 && up == 1) {
    up = 0;
  }
}
