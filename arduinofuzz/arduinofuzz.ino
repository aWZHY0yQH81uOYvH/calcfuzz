
const int K0 = 6;
const int K1 = 5;
const int K2 = 4;
const int K3 = A0;
const int K4 = A1;
const int K5 = A2;
const int K6 = A3;
const int K7 = A4;
const int K8 = A5;
const int K9 = 3;
const int K10 = 2;

struct button_path_t {
	int low;
	int high;
	int delay;
};

enum BUTTON_ID {
	B_0,
	B_1,
	B_2,
	B_3,
	B_4,
	B_5,
	B_6,
	B_7,
	B_8,
	B_9,
	B_DOT,
	B_EQ,
	B_SUB,
	B_ADD,
	B_MUL,
	B_DIV,
	B_PERCENT,
	B_SQRT,
	B_MRC,
	B_MSUB,
	B_MADD,
	B_OFF,
	B_ON,
	B_MAX
};

const button_path_t buttons[B_MAX] = {
	{K8, K10, 100}, // B_0
	{K8, K0,  100}, // B_1
	{K8, K1,  100}, // B_2
	{K8, K2,  100}, // B_3
	{K7, K0,  100}, // B_4
	{K7, K1,  100}, // B_5
	{K7, K2,  100}, // B_6
	{K6, K10, 100}, // B_7
	{K6, K0,  100}, // B_8
	{K6, K1,  100}, // B_9
	{K7, K9,  100}, // B_DOT
	{K4, K10, 500}, // B_EQ
	{K6, K2,  500}, // B_SUB
	{K5, K2,  500}, // B_ADD
	{K4, K2,  500}, // B_MUL
	{K4, K1,  500}, // B_DIV
	{K5, K10, 500}, // B_PERCENT
	{K8, K9,  500}, // B_SQRT
	{K7, K10, 500}, // B_MRC
	{K5, K0,  500}, // B_MSUB
	{K4, K0,  500}, // B_MADD
	{K3, K1,  50},  // B_OFF
	{K3, K2,  130}  // B_ON
};

void press_button(BUTTON_ID button) {
	const button_path_t &b = buttons[button];

	const int voltageThreshold = 200;
	unsigned long currentTime = millis();

	// hold button for 50ms
	while (millis() - currentTime < 50) {
		// mirror b.low to b.high
		int sensorValue = analogRead(b.low);
		if (sensorValue > voltageThreshold) {
			// Don't connect when button of interest is not selected
			pinMode(b.high, INPUT);
		} else {
			// Pull down when b.low is low to simulate connection of button contacts
			digitalWrite(b.high, LOW);
			pinMode(b.high, OUTPUT);
		}
	}
	// let go of button
	pinMode(b.high, INPUT);

	// wait for operation to finish
	while (millis() - currentTime < b.delay);
}

void setup() {
	Serial.begin(9600);  // Set the baud rate to match your serial monitor
	press_button(B_ON);

	// Say R when ready
	Serial.print("R");
}

void loop() {
	// wait until data recieved
	if (!Serial.available())
		return;

	// remove non-integers
	if (Serial.peek() < '0' || Serial.peek() > '9') {
		Serial.read();
		return;
	}

	// parse button press
	const BUTTON_ID selection = Serial.parseInt();

	// ensure button is valid
	if (selection < 0 || selection >= B_MAX) {
		Serial.print("N");
		return;
	}

	// press button
	press_button(selection);
	Serial.print("K");
}
