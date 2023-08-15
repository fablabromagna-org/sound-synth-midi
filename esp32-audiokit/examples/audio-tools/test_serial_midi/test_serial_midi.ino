#define RXD2 21
#define TXD2 22

const int BUFFER_SIZE = 50;
char buf[BUFFER_SIZE];


void setup() {
  Serial.begin(115200);

  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);  
  Serial2.setTimeout(5000); // set the time-out period to 5000 milliseconds (5 seconds)
}

void loop() {
  // wait for incoming data
  while (Serial2.available() == 0) {
    // do nothing
  }

  // read the incoming bytes:
  int rlen = Serial2.readBytes(buf, BUFFER_SIZE);

  // prints the received data
  Serial.print("I received: ");
  for (int i = 0; i < rlen; i++) {
    Serial.print(buf[i]);
  }
}