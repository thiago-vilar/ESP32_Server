import socket
import sys
import time

# Para enviar o e-mail
import smtplib
from email.mime.text import MIMEText


# aleatoriorandom608@gmail.com
# Senha: randomrandom
def send_email():
    # Alterar exemplo do prof! mailtrap.io/blog/python-send-email-gmail
    # Seguiu também a atualização de 30 de maio de 2022, ao criar
    # o app password de 16 caracteres à conta com 2-Factor Authentication
    # ativado.
    # https://support.google.com/accounts/answer/185833
    sender = "aleatoriorandom608@gmail.com"
    recipient = "aleatoriorandom608@gmail.com"
    password = "bjaf njtf tpee cvpb"
    # horario = time.ctime(time.time())
    horario_bruto = time.time()
    horario_tupla = time.localtime(horario_bruto)
    horario = time.strftime("%H:%M:%S do dia %d/%m/%Y", horario_tupla)
    body = f""" Intrusão detectada às {horario}.\n Teste OK!!!!!!!!!!!!!"""
    msg = MIMEText(body, "plain")
    msg["Subject"] = "Intrusão"
    msg["From"] = recipient
    msg["To"] = sender
    with smtplib.SMTP_SSL("smtp.gmail.com", 465) as smtp_server:
        smtp_server.login(sender, password)
        smtp_server.sendmail(sender, recipient, msg.as_string())
    print("Message sent!")


# Senhas em um arquivo database.txt
# Estabelecer o servidor
mysock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
hostname = socket.gethostname()
local_ip = socket.gethostbyname(hostname)
print(local_ip)

try:
    mysock.bind(("", 80))  # conecta cliente
except socket.error:
    print("Failed to bind")
    sys.exit()
mysock.listen()  # aguarda cliente

tentativas = 0
senha_correta = False
usuario_logado = ""

while True:
    conn, addr = mysock.accept()
    data = conn.recv(1000)

    time.sleep(3)  # aguarda tempo para envio de mensagem
    if not data:  # Se não tem informação, descartar data.
        break
    print("Got a request!!!")
    data = str(data, "UTF-8")

    #  modificar exemplo a partir daqui
    #  Deve receber mensagem de senha do cliente.
    #  Comparar senha com database.txt
    #  Se match, enviar mensagem de sucesso
    #  e enviar nome do usuário correspondente à senha
    #  Senão, enviar mensagem de erro.

    try:
        with open("database.txt", "r") as file:
            registro = file.readlines()
            for linha in registro:
                user, senha = linha.split()  # Comparar com senha do arquivo
                if data.find("POST") != -1:  # encontra cabeçalho POST
                    indexOption = data.find("/") + 1  # início da senha
                    if data[indexOption:indexOption + 4] == senha:
                        usuario_logado = user  # exibir info de quem logou
                        senha_correta = True
            if not senha_correta:  # falhou ao testar todos os usuarios
                tentativas += 1
                message = "Senha incorreta. Digite novamente."
                print("Senha incorreta. Digite novamente.")
                if tentativas == 2:  # testou 3 usuários 2 vezes
                    print("Senha incorreta novamente.")
                    send_email()
                    tentativas = 0
                    
            else:
                print(f"Usuário {usuario_logado} logado.")
                message = f"Senha Correta. Usuário {usuario_logado} logado."
                tentativas = 0
                usuario_logado = ""  # Reiniciando as variáveis para o próximo ciclo
                senha_correta = False

    except IOError:
        print("Erro! Não foi possível encontrar o arquivo database.txt")

    # if data.find("POST") != -1:  # encontra cabeçalho POST
    #    indexOption = data.find("/") + 1  # início da senha
    #
    #    if data[indexOption:indexOption + 4] == "4321":
    #        print("senha correta")
    #        message = "Senha Correta"
    # content_type = "Content-type:text/html\n\n"
    # content = 'Click <a href="/H">here</a> to turn the LED on.<br>'
    message += "\n"
    conn.sendall(bytes(message, "utf-8"))
    print("Request served.")


conn.close()
mysock.close()
