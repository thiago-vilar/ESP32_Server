#include <WiFi.h>
#include <ESP32Servo.h>

const char* ssid     = "Softex_Conv";
const char* password = "Softex2023";
const char* host = "192.168.158.227";
const int httpPort = 80;

WiFiClient client;
Servo servo;

#define PIN_BUZZER 4
#define PIN_LED 23
#define PIN_BOTAO 15
#define PIN_SERVO 2
#define PIN_LDR 34

#define PIN_A 12
#define PIN_B 32
#define PIN_C 33
#define PIN_D 25
#define PIN_E 26
#define PIN_F 27
#define PIN_G 14

#define NOTE_HIGH 1460

enum Estado {
  INICIAL, ATIVACAO, ATIVADO, CHECK, AUTENTICACAO, PEGA_LADRAO
};

Estado estado = INICIAL;  // Variável de estado
unsigned long contadorTempo = 0;  // Contador de tempo
int tentativas = 0;  // Contador de tentativas
const int maxTentativas = 2;  // Máximo de tentativas
const unsigned long tempoSenha = 10000;  // Tempo máximo para entrada de senha

void setupPins() {
  pinMode(PIN_A, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_C, OUTPUT);
  pinMode(PIN_D, OUTPUT);
  pinMode(PIN_E, OUTPUT);
  pinMode(PIN_F, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BOTAO, INPUT_PULLUP);
  pinMode(PIN_SERVO, OUTPUT);
  pinMode(PIN_LDR, INPUT);
  servo.attach(PIN_SERVO);
  servo.write(0);
}

const bool sete_segmentos[10][7] = {
  {1, 1, 1, 1, 1, 1, 0}, {0, 1, 1, 0, 0, 0, 0}, {1, 1, 0, 1, 1, 0, 1},
  {1, 1, 1, 1, 0, 0, 1}, {0, 1, 1, 0, 0, 1, 1}, {1, 0, 1, 1, 0, 1, 1},
  {1, 0, 1, 1, 1, 1, 1}, {1, 1, 1, 0, 0, 0, 0}, {1, 1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 0, 1, 1}
};

void escreverNumero(int numero) {
  const bool* display = sete_segmentos[numero];
  digitalWrite(PIN_A, display[0]);
  digitalWrite(PIN_B, display[1]);
  digitalWrite(PIN_C, display[2]);
  digitalWrite(PIN_D, display[3]);
  digitalWrite(PIN_E, display[4]);
  digitalWrite(PIN_F, display[5]);
  digitalWrite(PIN_G, display[6]);
}

void setup() {
  Serial.begin(115200);
  setupPins();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}

void sendToServer(String path, String data) {
  if (client.connect(host, httpPort)) {
    client.println("POST " + path + " HTTP/1.1");
    client.println("Host: " + String(host));
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Content-Length: " + data.length());
    client.println();
    client.println(data);
    Serial.println("Data sent to server");
  } else {
    Serial.println("Connection failed");
  }
}

String receiveFromServer() {
  String line;
  long startTime = millis();
  while (client.available() == 0) {
    if (millis() - startTime > 5000) {
      Serial.println("Server response timeout");
      return "";
    }
  }
  line = client.readStringUntil('\n');
  return line;
}

void handleServerResponse(String response) {
  if (response.startsWith("OK")) {
    Serial.println("Access granted by server.");
    digitalWrite(PIN_LED, LOW);
    noTone(PIN_BUZZER);
    servo.write(0);
    estado = INICIAL;
  } else {
    Serial.println("Access denied by server.");
    digitalWrite(PIN_LED, HIGH);
    tone(PIN_BUZZER, NOTE_HIGH);
  }
}

void loop() {
  int reading = touchRead(PIN_BOTAO);
  int displayNumber = 0;
  int seconds = 0;
  String senhaVigilancia, response;

  switch (estado) {
    case INICIAL:
      if (reading <= 10) {
        estado = ATIVACAO;
        contadorTempo = millis();
        Serial.println("Alarme sendo ativado. Feche a porta.");
      }
      break;
    case ATIVACAO:
      digitalWrite(PIN_LED, (millis() - contadorTempo) % 1000 < 500);
      seconds = (millis() - contadorTempo) / 1000;
      displayNumber = 9 - seconds;
      escreverNumero(displayNumber);
      if (seconds >= 10) {
        estado = ATIVADO;
        digitalWrite(PIN_LED, LOW);
        servo.write(90);
        Serial.println("Alarme ativado e porta trancada.");
      }
      break;
    case ATIVADO:
      if ((analogRead(PIN_LDR) > 512) || (reading <= 10)) {
        tone(PIN_BUZZER, NOTE_HIGH, 300);
        delay(400);
        tone(PIN_BUZZER, NOTE_HIGH, 300);
        delay(400);
        estado = CHECK;
        contadorTempo = millis();
        tentativas = 0;
        Serial.println("Intrusão detectada! Digite a senha:");
      }
      break;
    case CHECK:
      seconds = 9 - ((millis() - contadorTempo) / 1000);
      escreverNumero(max(0, seconds));  
      if (Serial.available() > 0) {
        String senha = Serial.readStringUntil('\n');
        if (senha == "1234") {
          estado = INICIAL;
          Serial.println("Senha correta. Alarme desativado.");
        } else {
          tentativas++;
          if (tentativas >= maxTentativas) {
            estado = PEGA_LADRAO;
          } else {
            Serial.println("Senha incorreta, tente novamente!");
            contadorTempo = millis(); 
          }
        }
      }
      if (millis() - contadorTempo > tempoSenha) {
        tentativas++;
        if (tentativas >= maxTentativas) {
          estado = PEGA_LADRAO;
        } else {
          Serial.println("Tempo esgotado, tente novamente!");
          contadorTempo = millis(); 
        }
      }
      break;
    case PEGA_LADRAO:
      digitalWrite(PIN_LED, HIGH);
      tone(PIN_BUZZER, NOTE_HIGH, 1000);
      delay(1000);
      noTone(PIN_BUZZER);
      Serial.println("Sistema de segurança ativado. Entre com a senha de vigilância:");

      while (!Serial.available()) {
        delay(1000);
      }

      senhaVigilancia = Serial.readStringUntil('\n');
      senhaVigilancia.trim();
      String postData = "password=" + senhaVigilancia;

      Serial.println("Senha digitada: " + senhaVigilancia);  
      Serial.println("Dados formatados para envio: " + postData); 

      if (client.connect(host, httpPort)) {
        Serial.println("Conexão ao servidor estabelecida.");

      
        client.println("POST /authenticate HTTP/1.1");
        client.println("Host: " + String(host));
        client.println("Content-Type: application/x-www-form-urlencoded");
        client.print("Content-Length: ");
        client.println(postData.length());
        client.println();  
        client.println(postData);  

        Serial.println("Dados enviados ao servidor.");

        unsigned long startTime = millis();
        while (client.available() == 0) {
          if (millis() - startTime > 5000) {
            Serial.println("Tempo de resposta do servidor excedido.");
            break;
          }
        }

        if (client.available()) {
          String response = client.readStringUntil('\n');
          Serial.println("Resposta do servidor: " + response);  

          if (response.indexOf("OK") != -1) {
            Serial.println("Acesso autorizado.");
            digitalWrite(PIN_LED, LOW);
            noTone(PIN_BUZZER);
            servo.write(0);
            estado = INICIAL;
          } else {
            Serial.println("Acesso negado.");
            digitalWrite(PIN_LED, HIGH);
            tone(PIN_BUZZER, NOTE_HIGH);
          }
        } else {
          Serial.println("Nenhuma resposta disponível do servidor.");
        }
      } else {
        Serial.println("Falha na conexão com o servidor.");
        digitalWrite(PIN_LED, HIGH);
        tone(PIN_BUZZER, NOTE_HIGH);
      }
      break;
    }
}