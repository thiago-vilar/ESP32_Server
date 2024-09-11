#include <WiFi.h>
#include <ESP32Servo.h>

#define PIN_BUZZER 4
#define PIN_LED 23
#define PIN_BOTAO 15
#define PIN_SERVO 17
#define PIN_LDR 34

#define PIN_A 12
#define PIN_B 32
#define PIN_C 33
#define PIN_D 25
#define PIN_E 26
#define PIN_F 27
#define PIN_G 14

#define NOTE_HIGH 1460

//Variáveis globais
// Bloco comunicação wifi
const char* ssid     = "Softex_Conv";
const char* password = "Softex2023";

// Bloco de wi-fi client
const char* host = "192.168.155.198";
const int httpPort = 80;

WiFiClient client;
Servo servo;

// variáveis globais para as funções
int reading = 1000;
int displayNumber = 0;
int seconds = 0;
String senhaVigilancia, response;
String senha = "";
String line = "";


unsigned long timeout = 0;
unsigned long contadorTempo = 0;  // Contador de tempo
int tentativas = 0;  // Contador de tentativas
const int maxTentativas = 2;  // Máximo de tentativas
const unsigned long tempoSenha = 10000;  // Tempo máximo para entrada de senha



enum Estado {
  INICIAL, ATIVACAO, ATIVADO, CHECK,  PEGA_LADRAO
};

Estado estado = INICIAL;  // Variável de estado

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
  //servo.setPeriodHertz(50);    // standard 50 hz servo
  //servo.attach(PIN_SERVO, 200, 2500);
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
  Serial.println("\n=====================");
  Serial.println("WiFi connected");
}


void loop() {
  reading = touchRead(PIN_BOTAO); // Esse botão está visível em qualquer estado. 
  
  switch (estado) {
    case INICIAL:
      // servo.write(0);
      // Adicionado para corrigir bug do servo durante a função tone()
      servo.setPeriodHertz(50);    // standard 50 hz servo
      servo.attach(PIN_SERVO, 200, 2500);
      delay(100);
      servo.write(90);
      delay(500);
      servo.detach();
      
      if (reading <= 10) {
        estado = ATIVACAO;
        contadorTempo = millis();
        Serial.println("Alarme sendo ativado. Feche a porta.");
      }
      break;

    case ATIVACAO:
      digitalWrite(PIN_LED, (millis() - contadorTempo) % 1000 < 500);
    //Delay "incorporado" na definição do estado lógico.
    //Se o resto da diferença for menor que 500 milissegundos, valor lógico false( == 0)
    //Senão, se a diferença for maior que 500 milissegundos, valor lógico true( > 0)
      seconds = (millis() - contadorTempo) / 1000;
      displayNumber = 9 - seconds;
      escreverNumero(displayNumber);
      if (seconds >= 10) {
        estado = ATIVADO;
        digitalWrite(PIN_LED, LOW);

        //servo.write(90);
        // Adicionado para corrigir bug do servo durante a função tone()
        servo.setPeriodHertz(50);    // standard 50 hz servo
        servo.attach(PIN_SERVO, 200, 2500);
        delay(100);
        servo.write(180);
        delay(500);
        servo.detach();

        Serial.println("Alarme ativado e porta trancada.");
      }
      break;

    case ATIVADO:
      if ((analogRead(PIN_LDR) > 512) || (reading <= 10)) {
 
        delay(100);
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
      
      if (Serial.available() > 0) { // Serial.available só funciona se recebe bytes do monitor serial!
      // Dessa forma, consegue manter a contagem regressiva sem precisar parar a execução aguardando
      // o usuário digitar a senha.
        String senha = Serial.readStringUntil('\n');

        // Conectar ao servidor, mandar a senha digitada e aguardar resposta
        Serial.print("connecting to ");
        Serial.println(host);

        // Use WiFiClient class to create TCP connections
        if (!client.connect(host, httpPort)) {
            Serial.println("connection failed");
            return; //Volta para o início do loop, desprezando o restante do código
        }
        // Enviar a senha para o servidor, se a conexão for bem sucedida
        client.print(String("POST /") + senha + " \r\n\r\n");  // Writing to server  

        // Recebendo resposta do servidor
        timeout = millis();
        while (client.available() == 0) {
            if (millis() - timeout > 5000) {
                Serial.println(">>> Client Timeout !");
                client.stop();
                return;  //Volta para o início do loop, desprezando o restante do código
            }
        }

        // Read all the lines of the reply from server and print them to Serial
        while(client.available()) { // sai do loop quando todos os caracteres do buffer serial são lidos.
            line = client.readStringUntil('\r');
            Serial.print(line);
        }

        // Analisar a mensagem enviada pelo servidor
        if (line.startsWith("Senha incorreta")){
          // Senha não confere com o banco de senhas. mostrar a mensagem de erro enviada pelo servidor
          tentativas++;
          if (tentativas >= maxTentativas) {
            estado = PEGA_LADRAO;
          } 
          else {
            Serial.println("Senha incorreta, tente novamente!");
            contadorTempo = millis(); 
          }
        }
        else if (line.startsWith("Senha Correta")) {
          // Confirmada senha correta, mostrar a mensagem do servidor com o nome do user logado
          Serial.println("Senha correta. Alarme desativado.");
          estado = INICIAL;
          digitalWrite(PIN_A, LOW);
          digitalWrite(PIN_B, LOW);
          digitalWrite(PIN_C, LOW);
          digitalWrite(PIN_D, LOW);
          digitalWrite(PIN_E, LOW);
          digitalWrite(PIN_F, LOW);
          digitalWrite(PIN_G, LOW);
          break;
        }
        else {
          // leu algum dado não relevante. Mostrar mensagem de erro de comunicação e retomar contagem
          //regressiva
          Serial.println("Erro de comunicação.");
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
      

      if (Serial.available() > 0) {
        // Conectar ao servidor, mandar a senha digitada e aguardar resposta
        senha = Serial.readStringUntil('\n');
        Serial.print("connecting to ");
        Serial.println(host);

        // Use WiFiClient class to create TCP connections
        if (!client.connect(host, httpPort)) {
            Serial.println("connection failed");
            return; //Volta para o início do loop, desprezando o restante do código
        }
        // Enviar a senha para o servidor, se a conexão for bem sucedida
        client.print(String("POST /") + senha + " \r\n\r\n");  // Writing to server  

        // Recebendo resposta do servidor
        timeout = millis();
        while (client.available() == 0) {
            if (millis() - timeout > 5000) {
                Serial.println(">>> Client Timeout !");
                client.stop();
                return;  //Volta para o início do loop, desprezando o restante do código
            }
        }

        if (client.available()) {
          String response = client.readStringUntil('\n');
          Serial.println("Resposta do servidor: " + response);  

          if (response.substring(6,13) == "Correta") {
            Serial.println("Acesso autorizado.");
            digitalWrite(PIN_LED, LOW);
            noTone(PIN_BUZZER);

            // Adicionado para corrigir bug do servo durante a função tone()
            servo.setPeriodHertz(50);    // standard 50 hz servo
            servo.attach(PIN_SERVO,200, 2500);
            delay(100);
            servo.write(90);
            delay(500);
            servo.detach();

            estado = INICIAL;
            digitalWrite(PIN_A, LOW);
            digitalWrite(PIN_B, LOW);
            digitalWrite(PIN_C, LOW);
            digitalWrite(PIN_D, LOW);
            digitalWrite(PIN_E, LOW);
            digitalWrite(PIN_F, LOW);
            digitalWrite(PIN_G, LOW);
          } else {
            Serial.println("Acesso negado.");
            digitalWrite(PIN_LED, HIGH);
            tone(PIN_BUZZER, NOTE_HIGH);
          }
        } else {
          Serial.println("Nenhuma resposta disponível do servidor.");
        }
      }
      break;
  }
  
}
