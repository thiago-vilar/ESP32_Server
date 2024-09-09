#include <WiFi.h>
#include <ESP32Servo.h>


const int LED = 2;

/*
 *  Firmware do NodeMCU ESP32 com o cliente web do sistema de alarme
 *  O cliente envia a senha para o servidor por meio do serial monitor mesmo
 *  Esp32 executa todo o acionamento e monitoramento do hardware, para depois
 *  enviar alguma mensagem para o servidor remoto.

 *  O Usuário ativa o sistema apertando o botão de toque.
 *  Na ativação, led pisca por 10 segundos, avisando ao usuário para fechar
 *  uma porta (hipotética).
 *  Led deve apagar depois desse tempo.
 *  Um servomotor deve girar 90 graus, representando trava de fechadura
 *  Se LDR = True ou botaoToque = true, buzzer emite 2 beeps.
 *  Então habilitar o monitor serial para usuário digitar senha
 *  Mostrar contagem regressiva de 9 a 0 no display 7-segmentos
 *  Se usuário errar a senha por uma vez ou acabar o tempo de 10 segundos,
 *  informar ao usuário e dar mais um tempo de 10 segundos.
 *  Esgotado o segundo tempo, ou errando senha duas vezes, dispara alarme
 *  Acendendo continuamente o LED e ativando continuamente o buzzer.
 *  Se a senha for correta, desligar LED, e voltar Servo para posição
 *  inicial.
 *  
 *  Para autenticar a senha, comunicar com o servidor (script python)
 *  Se senha correta, mostrar o nome de usuário enviado para o ESP32
 *  Se senha errada, mostrar mensagem de erro do servidor.
 *  Se duas senhas erradas, servidor envia e-mail informando data e 
 *  hora do acesso negado.
 */

const char* ssid     = "Softex_Conv";
const char* password = "Softex2023";


//const char* host = "www.google.com";
const char* host = "192.168.15.7";

WiFiClient client;
int value = 0;

void setup()
{
    Serial.begin(115200);
    delay(10);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if (value > 20) {
          Serial.print("10 segundos\n");
        }
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
   
}


const int httpPort = 80;
unsigned long timeout = 0;
String senha = "";
void loop()
{
    //delay(5000);

    // no estado de inserir a senha, copiar o seguinte código:
    senha = "";
    Serial.println("Digite senha:");
    while (senha == "") {
      senha = Serial.readStringUntil('\n');
    // Interromper a contagem regressiva? Pode demorar mais que o
    // tempo restante para receber a resposta do servidor...
    }

    Serial.print("connecting to ");
    Serial.println(host);

    // Use WiFiClient class to create TCP connections

    
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
    }

    client.print(String("POST /") + senha + " \r\n\r\n");  // Writing to server
    timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }

    // Read all the lines of the reply from server and print them to Serial
    while(client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }

  
    client.stop();
    Serial.println();
    Serial.println("closing connection");
    
}



