
const float q = 5.000 / 1023;
long t, g;
volatile long crank_t = 0, crank_period = 0,
            airFlow_t = 0, airFlow_period = 0;

const float iatDivRes = 2670, 
            literPerRev = 1.25;
            
int mapRaw, iatRaw, prevIatIdx;
float iatOhm, mapKpa, iatC, iatK, rpm = 700, rps = rpm / 60, lps = rps * literPerRev, m;
float iatCal[18][2] = {
  130, 89.3,
  120, 112.7,
  110, 144.2,
  100, 186.6,
  90, 243.2, 
  80, 322.5,
  70, 435.7,
  60, 595.5,
  50, 834,
  40, 1175,
  30, 1707,
  20, 2500,
  10, 3792,
  0, 5896,
  -10, 9397,
  -20, 15462,
  -30, 26114,
  -40, 45313
}; 

float mapCal[2][2] = {
  323.2, 5,
  -6.34, 0
};

const float kpaPerV = (mapCal[0][0] - mapCal[1][0]) / (mapCal[0][1] - mapCal[1][1]), 
            mapOffset = mapCal[0][0] - mapCal[0][1] * kpaPerV; 


void setup() {
  Serial.begin(9600);
  pinMode(8, OUTPUT);  
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  attachInterrupt(0, intCrank, RISING);
  attachInterrupt(1, intAirFlow, RISING);
}

void loop() {
  g = micros();
  mapRaw = analogRead(0);
  iatRaw = analogRead(1);
  iatC = getIat(iatRaw / ((1023 - iatRaw) / iatDivRes));
  iatK = iatC + 273.0;
  mapKpa = (mapRaw * q) * kpaPerV + mapOffset;
  m = (mapKpa * 1000 * (1000000.0 / (crank_period * 3)) * literPerRev * 0.029) / (8.31 * iatK);
  rpm = (1000000.0 / (crank_period * 3.0)) * 60.0;
  tone(8, toHz(m));
  g = micros() - g;
  
  
  Serial.print(m);
  Serial.print(" g/s - ");
  Serial.print(iatC);
  Serial.print(" C - ");
  Serial.print(mapKpa);
  Serial.print(" kPa - ");
  Serial.print(rpm);
  Serial.print(" rpm - ");
  Serial.print(1000000.0 / airFlow_period);
  Serial.print(" Hz - ");
  Serial.println(g);
  
}

float toHz(float m) {
  return m;  
}

float getIat(float ohm) {
  float cellTemp, unit;
  for (int i = prevIatIdx; i < 18; i++) {
    if (iatCal[i][1] > ohm) {
      prevIatIdx = i;
      unit = (iatCal[i][1] - iatCal[i - 1][1]) / 10;  
      return iatCal[i][0] + (iatCal[i][1] - ohm) / unit;     
    }
  }  
  prevIatIdx = 0;
  return getIat(ohm);
}

void intCrank() {
  t = micros();  
  if (t - crank_t < 2000) {
    return; 
  }
  crank_period = t - crank_t;
  crank_t = t; 
}

void intAirFlow() {
  t = micros();  
  if (t - airFlow_t < 200) {
    return; 
  }
  airFlow_period = t - airFlow_t;  
  airFlow_t = t; 
}
