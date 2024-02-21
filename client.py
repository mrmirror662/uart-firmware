import serial
import time
import sys
# Open serial port (change the port and baudrate according to your setup)
ser = serial.Serial('COM3', 2400, timeout=1)

# Function to send data to Arduino
def send_data(data):
    ser.write(bytes(data,'ascii'))

# Function to receive data from Arduino
def receive_data():
    return ser.readline().decode('ascii').strip()


#open the file containing the sample text
f = open("msg.txt", "r")
data = f.read()+'\r'

#param to modify the live transmission rate
band_width_sample_rate = 64

# Main function
def main():
    try:
        p_start = ''
        #firmware sends program start before actual transmission
        while True:
            c = ser.read(1)
            print(c)
            if(c == b'\r'):
                break;
        
        num_bytes = 0
        counter = 0
        #set current time
        time_current_s = float(time.time_ns())/1000000000.0
        start_time_s = time_current_s
        data_size = 0

        # Send data to Arduino
        print("sending data..\n")
        for c in data:
            #print live transmission
            if(counter > band_width_sample_rate):
                #sample current time
                updated_time_s = float(time.time_ns())/1000000000.0
                bits_s = (num_bytes/(updated_time_s-time_current_s))*8
                print(f'\r{bits_s} bits/s', end='\r', flush=True)
                time_current_s = updated_time_s
                counter = 0
                num_bytes = 0
            counter+=1
            send_data(c)
            num_bytes += 1
            data_size+=1
        data_recieved  = '';
        counter = 0
        time_current_s = float(time.time_ns())/1000000000.0
        end_time_s = time_current_s

        #estimate average time
        print(f"average sending speed = {data_size*8/(end_time_s-start_time_s)}")
        start_time_s = end_time_s

        print("\nrecieving  data..\n")
        data_size = 0
        while True:
            if(counter > band_width_sample_rate):
                #sample current time
                updated_time_s = float(time.time_ns())/1000000000.0
                bits_s = (num_bytes/(updated_time_s-time_current_s))*8
                print(f'\r{bits_s} bits/s', end='\r', flush=True)
                time_current_s = updated_time_s
                counter = 0
                num_bytes = 0
            counter+=1
            num_bytes+=1
            data_size+=1
            c = ser.read(1)
            #end data reception open encountering '\r'
            if(c == b'\r'):
                break;
            #ignore decoding errors
            c = c.decode('ascii','ignore');
            data_recieved += c;
        
        #estimate average time
        end_time_s = float(time.time_ns())/1000000000.0
        print(f"average recieving speed = {data_size*8/(end_time_s-start_time_s)}")

        print("\n~~~~~~~~~~~recieved data~~~~~~~~\n")
        print(data_recieved)
    except KeyboardInterrupt:
        #terminate script if ctrl+c is pressed
        print("\nProgram terminated by user.")
    

if __name__ == "__main__":
    main()