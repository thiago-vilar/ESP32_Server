import socket
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText

def send_email():
    sender_email = "aleatoriorandom608@gmail.com"
    sender_pass = "randomrandom"
    receiver_email = "tcv3@softex.cin.ufpe.br"

    msg = MIMEMultipart()
    msg['From'] = sender_email
    msg['To'] = receiver_email
    msg['Subject'] = "Alert: Security Breach Attempt Detected"
    msg.attach(MIMEText("Multiple failed login attempts detected.", 'plain'))

    server = smtplib.SMTP('smtp.gmail.com', 587)
    server.starttls()
    server.login(sender_email, sender_pass)
    server.sendmail(sender_email, receiver_email, msg.as_string())
    server.quit()
    print("Security alert email sent.")

def main():
    correct_password =str("password=12345678")
    failed_attempts = 0
    max_attempts = 2

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(('', 80))
    sock.listen(5)
    print("Server listening on port 80")
    mysock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    hostname = socket.gethostname()
    local_ip = socket.gethostbyname(hostname)
    print(local_ip)

    while True:
        conn, addr = sock.accept()
        print(f"Connection from {addr}")

        # Recebe os dados da requisição
        data = conn.recv(4096).decode()
        print(f"Full request data:\n{data}")

        # Verifica se o corpo da requisição contém a senha
        if "password=" in data:
            password_received = data.split("password=")[-1].strip()
            print(f"Password extracted: '{password_received}'")

            # Verifica a senha
            if password_received == correct_password:
                conn.sendall("HTTP/1.1 200 OK\n\nAccess granted.".encode())
                print("Access granted.")
            else:
                conn.sendall("HTTP/1.1 401 Unauthorized\n\nInvalid password.".encode())
                print("Failed authentication attempt.")
                failed_attempts += 1
                if failed_attempts >= max_attempts:
                    send_email()
                    failed_attempts = 0
        else:
            print("Password not found in request.")
            conn.sendall("HTTP/1.1 400 Bad Request\n\nPassword not found.".encode())

        conn.close()

if __name__ == '__main__':
    main()