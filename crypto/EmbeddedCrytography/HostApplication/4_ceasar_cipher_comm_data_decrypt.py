import serial.tools.list_ports



def decrypt_ceasar_cipher(encryptedext, shift, length):
    decryptedtext=""
    
    #Loop through each character of the encryptedext
    for i in range(length):
        c = encryptedext[i]

        #Check if the character is a lowercase character
        if 'a'  <= c <= 'z':
            #Decrypt lowercase character
            c =  chr(((ord(c) -  ord('a') -  shift + 26) % 26) + ord('a'))

        #Check if the character is a uppercase character
        elif 'A'  <= c <= 'Z':
            #Decrypt uppercase character
            c =  chr(((ord(c) -  ord('A') -  shift + 26) % 26) + ord('A'))
        
        #Append the decrypted to the decryptedtext
        decryptedtext += c
    
    return decryptedtext


def encypt_ceasar_cipher(plaintext, shift, length):
    #Init empty string to store encrypted text
    encyptedtext = ""

    #Loop through each character of the plaintext
    for i in range(length):
        c =  plaintext[i]

        #Check if character is lowercase or uppercase
        if 'a' <= c <= 'z':
            #Encryt lowercase letter
            c =  chr(((ord(c) -  ord('a') + shift) % 26) + ord('a'))
        elif 'A' <= c <= 'Z':
            #Encryt uppercase letter
            c =  chr(((ord(c) -  ord('A') + shift) % 26) + ord('A'))
        encyptedtext += c
    
    return encyptedtext



def get_ports():
    ports =  list(serial.tools.list_ports.comports())
    for port in ports:
        print(port.manufacturer)

        if port.manufacturer.startswith("STM"):
            port_number = port.device
    return port_number


iot_device = serial.Serial(port =  get_ports(), baudrate=115200, timeout=1)

def receive_data():
    try:
        value =  iot_device.readline()
        #print(value)
    except:
        print("Nothing to print")
    return value


while True:
    received_byte = receive_data()
    ciphertext = received_byte.decode('utf-8')
    shift = 3
    length =  len(ciphertext)
    decryptedtext =  decrypt_ceasar_cipher(ciphertext,shift, length)

    print(f"Encrypted text:{ciphertext}")

    print(f"Decrypted text:{decryptedtext}")
