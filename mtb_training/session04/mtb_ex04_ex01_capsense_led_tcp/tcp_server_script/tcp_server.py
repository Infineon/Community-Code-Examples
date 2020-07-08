import threading
import socket
import optparse
import time
import sys

data1 = 1


def echo_server(host, port):
    print("==========================")
    print("TCP Server")
    print("==========================")
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
    try:
        s.bind((host, port))
        s.listen(1)
    except socket.error as msg:
        print("ERROR: ", msg)
        s.close()
        s = None

    if s is None:
        sys.exit(1)

    while 1:
        print("Listening on: %s:%d" % (host, port))
        try:
            conn, addr = s.accept()

        except KeyboardInterrupt:
            print("Closing Connection")
            s.close()
            s = None
            sys.exit(1)

        print('Incoming connection accepted: ', addr)

        try:
            while 1:
                global data1
                data1 = str(data1)
                conn.send(data1.encode())
                data = conn.recv(4096)
                if not data: break
                print("Data from TCP Client:", data.decode('utf-8'))
                print("")

        except KeyboardInterrupt:
            print("Closing Connection")
            s.close()
            s = None
            sys.exit(1)

        conn.close()


def data_read():
    global data1
    while True:
        print("Enter your option: '1' to start receiving data, '0' to stop receiving data and Press the 'Enter' key:")
        data1 = input()


if __name__ == "__main__":
    parser = optparse.OptionParser()
    parser.add_option("-p", "--port", dest="port", type="int", default=50007,
                      help="Port to listen on [default: %default].")
    parser.add_option("--hostname", dest="hostname", default="", help="Hostname to listen on.")

    (options, args) = parser.parse_args()

    t1 = threading.Thread(target=echo_server, args=(options.hostname, options.port,))
    t2 = threading.Thread(target=data_read, args=())
    # starting thread 1 
    t1.start()
    # starting thread 2 
    t2.start()

    # wait until thread 1 is completely executed 
    t1.join()
    # wait until thread 2 is completely executed 
    t1.join()

    # both threads completely executed 
    print("Done!")
