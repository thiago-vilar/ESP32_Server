import socket
import sys

# Criação do socket TCP
mysock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    # Vincula o socket ao endereço e porta
    mysock.bind(("", 80))
except socket.error as err:
    print(f"Failed to bind: {err}")
    sys.exit()

mysock.listen(5)  # Configura o socket para ouvir até 5 conexões pendentes

print("Server listening on port 80...")

while True:
    conn, addr = mysock.accept()  # Aceita a conexão de um cliente
    print(f"Connected by {addr}")
    
    data = conn.recv(1024)  # Recebe os dados da conexão
    if not data:
        break

    print(f"Request received:\n{data.decode('utf-8')}")

    # Mensagem de resposta HTTP
    message = "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\nHello World"
    
    conn.sendall(bytes(message, "utf-8"))  # Envia a resposta
    conn.close()  # Fecha a conexão com o cliente

mysock.close()  # Fecha o servidor (embora nunca será executado neste loop)
