int pwm_left = 9;
int pwm_right = 10;

int forward_left = 7;
int reverse_left = 6;
int forward_right = 5;
int reverse_right = 4;

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  pinMode(pwm_left, OUTPUT);
  pinMode(pwm_right, OUTPUT);
  pinMode(forward_left, OUTPUT);
  pinMode(reverse_left, OUTPUT);
  pinMode(forward_right, OUTPUT);
  pinMode(reverse_right, OUTPUT);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  forward(255, 2000);
  Serial.println("Forward.");
  stopAndWait(1000);
  Serial.println("Stop.");

  reverse(255, 2000);
  Serial.println("Reverse.");
  stopAndWait(1000);

  left(255, 1000);
  stopAndWait(500);
  
  right(255, 1500);
  stopAndWait(3000);
  
  

}

void forward(int velocity, int duration) {
  digitalWrite(forward_left, HIGH);
  digitalWrite(forward_right, HIGH);
  digitalWrite(reverse_left, LOW);
  digitalWrite(reverse_right, LOW);
  analogWrite(pwm_left, velocity);
  analogWrite(pwm_right, velocity);
  delay(duration);
}

void reverse(int velocity, int duration) {
  digitalWrite(forward_left, LOW);
  digitalWrite(forward_right, LOW);
  digitalWrite(reverse_left, HIGH);
  digitalWrite(reverse_right, HIGH);
  analogWrite(pwm_left, velocity);
  analogWrite(pwm_right, velocity);
  delay(duration);
}

void left(int velocity, int duration) {
  digitalWrite(forward_left, LOW);
  digitalWrite(forward_right, HIGH);
  digitalWrite(reverse_left, HIGH);
  digitalWrite(reverse_right, LOW);
  analogWrite(pwm_left, velocity);
  analogWrite(pwm_right, velocity);
  delay(duration);
}

void right(int velocity, int duration) {
  digitalWrite(forward_left, HIGH);
  digitalWrite(forward_right, LOW);
  digitalWrite(reverse_left, LOW);
  digitalWrite(reverse_right, HIGH);
  analogWrite(pwm_left, velocity);
  analogWrite(pwm_right, velocity);
  delay(duration);
}

void stopAndWait(int duration) {
  
  digitalWrite(forward_left, LOW);
  digitalWrite(forward_right, LOW);
  digitalWrite(reverse_left, LOW);
  digitalWrite(reverse_right, LOW);
  delay(duration);
}
