import socket
import sys

# Configuração inicial do socket
mysock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
    mysock.bind(('', 80))  # Bind to all interfaces on port 80
    mysock.listen(5)
    print("Server listening on port 80")
except socket.error as e:
    print(f"Failed to bind: {e}")
    sys.exit()

# Lista de usuários e senhas
users = {
    "user1": "1234",
    "user2": "5678",
    "security": "12345678"
}

def send_email():
    import smtplib
    from email.mime.text import MIMEText
    from email.mime.multipart import MIMEMultipart

    sender_email = "random608@gmail.com"
    sender_pass = "randomrandom"
    receiver_email = "tcv3@softex.cin.ufpe.br"

    msg = MIMEMultipart()
    msg['From'] = sender_email
    msg['To'] = receiver_email
    msg['Subject'] = "Alert: Failed Authentication Attempt"

    body = "A failed login attempt occurred. Please check the system for any unauthorized access."
    msg.attach(MIMEText(body, 'plain'))

    server = smtplib.SMTP('smtp.gmail.com', 587)
    server.starttls()
    server.login(sender_email, sender_pass)
    server.sendmail(sender_email, receiver_email, msg.as_string())
    server.quit()
    print("Email alert sent.")


try:
    while True:
        conn, addr = mysock.accept()
        print(f"Connection from {addr}")

        data = conn.recv(1024).decode().strip()
        if data:
       
            parts = data.split(' ')
            if len(parts) > 1 and parts[0] == 'POST':
               
                password = parts[1]
                user_found = False
                for username, user_pass in users.items():
                    if user_pass == password:
                        conn.sendall("HTTP/1.1 200 OK\n\nUser authenticated successfully.".encode())
                        print(f"User {username} authenticated successfully.")
                        user_found = True
                        break
                if not user_found:
                    conn.sendall("HTTP/1.1 401 Unauthorized\n\nInvalid password.".encode())
                    print("Failed authentication attempt.")
                    send_email() 
        conn.close()
except KeyboardInterrupt:
    print("Server is shutting down.")
    mysock.close()
    sys.exit()
