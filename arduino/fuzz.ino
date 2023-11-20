
const int K0 = 2;
const int K1 = 3;
const int K2 = 4;
const int K3 = A0;
const int K4 = A1;
const int K5 = A2;
const int K6 = A3;
const int K7 = A4;
const int K8 = A5;
const int K9 = 5;
const int K10 = 6;

struct button_path_t {
	int low;
	int high;
	int delay;
};

const button_path_t b_percent	= {K5, K10, 500};
const button_path_t b_sqr	= {K8, K9, 500};
const button_path_t b_off	= {K3, K1, 50};
const button_path_t b_on	= {K3, K2, 130};
const button_path_t b_mrc	= {K7, K10, 500};
const button_path_t b_msub	= {K5, K0, 500};
const button_path_t b_madd	= {K4, K0, 500};
const button_path_t b_div	= {K4, K1, 500};
const button_path_t b_7		= {K6, K10, 100};
const button_path_t b_8		= {K6, K0, 100};
const button_path_t b_9		= {K6, K1, 100};
const button_path_t b_mul	= {K4, K2, 500};
const button_path_t b_4		= {K7, K0, 100};
const button_path_t b_5		= {K7, K1, 100};
const button_path_t b_6		= {K7, K2, 100};
const button_path_t b_sub	= {K6, K2, 500};
const button_path_t b_1		= {K8, K0, 100};
const button_path_t b_2		= {K8, K1, 100};
const button_path_t b_3		= {K8, K2, 100};
const button_path_t b_add	= {K5, K2, 500};
const button_path_t b_0		= {K8, K10, 100};
const button_path_t b_dot	= {K7, K9, 100};
const button_path_t b_eq	= {K4, K10, 500};

// Helper function to map button names to button paths
button_path_t parse_button(String buttonName) {
	buttonName.toLowerCase();  // Convert the button name to lowercase for case-insensitive comparison
	Serial.print("pressing ");
	Serial.println(buttonName);

	// Use a switch-case statement to map button names to button paths
	if (buttonName == "percent") return b_percent;
	else if (buttonName == "sqrt") return b_sqrt;
	else if (buttonName == "off") return b_off;
	else if (buttonName == "on") return b_on;
	else if (buttonName == "mrc") return b_mrc;
	else if (buttonName == "msub") return b_msub;
	else if (buttonName == "madd") return b_madd;
	else if (buttonName == "div") return b_div;
	else if (buttonName == "7") return b_7;
	else if (buttonName == "8") return b_8;
	else if (buttonName == "9") return b_9;
	else if (buttonName == "mul") return b_mul;
	else if (buttonName == "4") return b_4;
	else if (buttonName == "5") return b_5;
	else if (buttonName == "6") return b_6;
	else if (buttonName == "sub") return b_sub;
	else if (buttonName == "1") return b_1;
	else if (buttonName == "2") return b_2;
	else if (buttonName == "3") return b_3;
	else if (buttonName == "add") return b_add;
	else if (buttonName == "0") return b_0;
	else if (buttonName == "dot") return b_dot;
	else if (buttonName == "eq") return b_eq;

	// Default case: return an invalid button path
	return {-1, -1};
}


void press_button(button_path_t b) {
	const int voltageThreshold = 200;
	unsigned long currentTime = millis();

	// hold button for 50ms
	while (millis() - currentTime < 50) {
		// mirror b.low to b.high
		int sensorValue = analogRead(b.low);
		if (sensorValue > voltageThreshold) {
			pinMode(b.high, INPUT_PULLUP); // use pullup resistor instead of digitalWrite
		} else {
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
	for (int i = 0; i < 8; i++)
		pinMode(i, INPUT);
	press_button(b_on);
}

void loop() {
	if (Serial.available() > 0) {
		String userInput = Serial.readStringUntil('\n');

		button_path_t buttonPath = parse_button(userInput);

		press_button(buttonPath);
	}

	// http://blog.presentandcorrect.com/250-words-you-can-spell-with-a-calculator
	press_button(b_on);
	press_button(b_1);
	delay(1000);
	press_button(b_on);
	press_button(b_7);
	press_button(b_7);
	press_button(b_3);
	press_button(b_5);
	delay(1000);
	press_button(b_on);
	press_button(b_5);
	press_button(b_7);
	press_button(b_7);
	press_button(b_3);
	press_button(b_4);
	press_button(b_5);
	delay(1000);
}
