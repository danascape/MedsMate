/*
      FORMAT FOR RECIEVING DATA
      START AND END MARKERS:                  "<" and ">"
      DIVIDERS BETWEEN ITEM_COUNT PAIRS:      "|"
      DIVIDERS BETWEEM ITEM AND COUNT:        "-"

      Example: "<1-3|2-1|5-2|8-3>"
      make sure data sent from the pi arrives in this format.
  */

bool newData = false;

#define IR_SENSOR_PIN1 2
#define IR_SENSOR_PIN2 3
#define IR_SENSOR_PIN3 4
const int maxItems = 6;
int itemcount = 0;

#define MOTOR_ONE 8
#define MOTOR_TWO 9
#define MOTOR_THREE 10
#define MOTOR_FOUR 11
#define MOTOR_FIVE 12
#define MOTOR_SIX 13



const byte numChars = 64;
char rcvdChars[numChars];
char tempChars[numChars];
int items[maxItems][2] = { { 0 } };
int ir = 1;
int cnt = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(IR_SENSOR_PIN1, INPUT);
  pinMode(IR_SENSOR_PIN2, INPUT);
  pinMode(MOTOR_ONE, OUTPUT);
  pinMode(MOTOR_TWO, OUTPUT);
  pinMode(MOTOR_THREE, OUTPUT);
  pinMode(MOTOR_FOUR, OUTPUT);
  pinMode(MOTOR_FIVE, OUTPUT);
  pinMode(MOTOR_SIX, OUTPUT);

  digitalWrite(MOTOR_ONE, HIGH);
  digitalWrite(MOTOR_TWO, HIGH);
  digitalWrite(MOTOR_THREE, HIGH);
  digitalWrite(MOTOR_FOUR, HIGH);
  digitalWrite(MOTOR_FIVE, HIGH);
  digitalWrite(MOTOR_SIX, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  recvDataFromPi();
  dispenseParent();
}

void dispense(int pin, int count) {
  // dispenses (count) items from (pin) index
  // turns the thing for a second, then in half second increments until the item has been dispensed
  for (int i = 1; i <= count; i++) {
    digitalWrite(pin, LOW);
    Serial.println("rotating coil...");
    while (true) {
      int IR1 = digitalRead(IR_SENSOR_PIN1);
      int IR2 = digitalRead(IR_SENSOR_PIN2);
      int IR3 = digitalRead(IR_SENSOR_PIN3);
      int irf = IR1*IR2*IR3;
      ir *= irf;
      if (ir == 0) {
        digitalWrite(pin, HIGH);
        Serial.println("Item Dispensed");
        delay(2000);
        break;
      }
    }
    ir = 1;
  }
  Serial.println("All items from track Dispensed");
}

void dispenseParent() {
  if (newData == true) {
    Serial.println(rcvdChars);
    parseInput(rcvdChars);

    for (int i = 0; i <= itemcount; i++) {
      dispense(items[i][0], items[i][1]);
    }
    newData = false;
  } else {
    cnt++;
    if (cnt == 30000) {
      // Serial.println("no new data");
      cnt = 0;
    }
  }
}

void recvDataFromPi() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  if (Serial.available() == 0) {
    return;
  }

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        rcvdChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      } else {
        rcvdChars[ndx] = '\0';  // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    } else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

void parseInput(String input) {
  int index = 0;
  int startPos = 0;

  while (index <= maxItems) {
    int separatorPos = input.indexOf('|', startPos);

    if (separatorPos == -1) {
      break;
    }

    String item = input.substring(startPos, separatorPos);
    Serial.println(item);
    int dashPos = item.indexOf('-');


    if (dashPos != -1) {
      items[index][0] = item.substring(0, dashPos).toInt();
      Serial.println(items[index][0]);
      Serial.println(index);

      items[index][1] = item.substring(dashPos + 1).toInt();
      Serial.println(items[index][1]);
      index++;
    }

    startPos = separatorPos + 1;
  }
  itemcount = index;
}
