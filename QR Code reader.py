import cv2
import numpy as np
from pyzbar.pyzbar import decode
import tkinter as tk
import serial
import time

line = ''
roomNumber = 0 #Will identify room we are looking for
myDataPrev = ''
cap = cv2.VideoCapture(0)
cap.set(3,640)
cap.set(4,480)
arduinoOn = False
authorizedUser = False # Waits for verified user
beginCV = False #does not start openCV until this is true
roomRecieved = False
CVRoom = False
#ser = serial.Serial('/dev/ttyACM0',9600) # 9600 is channel arudino will use
#ser.reset_input_buffer()
'''
#variables used in while loops
room_1 = ""
room_2 = ""
chosenRoom = ""

while (arduinoOn == False): #waits to ensure arduino is on before continuing
    print("Checking if arduino is on")
    time.sleep(0.1)
    if ser.in_waiting > 0:
        print("Something is in Serial")
        line = ser.readline().decode('latin-1').rstrip()
        print(line)
    if line == "Arduino On":
        arduinoOn = True
        print("Moving to Authorization Phase")
while (authorizedUser == False): #waits for arduino to confirm authorized user
    time.sleep(1)
    ser.write("Pi On\n".encode('utf-8'))
    if ser.in_waiting > 0:
        line = ser.readline().decode('latin-1').rstrip()
        print(line)
        if line == "User Authorized":
            authorizedUser = True
            print("Moving to Standby Phase")
    
while (beginCV == False): #waits for arduino to begin searching for room anf gets room number
    time.sleep(1)
    ser.write("Auth User Received\n".encode('utf-8'))
    if ser.in_waiting > 0:
        line = ser.readline().decode('latin-1').rstrip()
        ser.write("Looking for Room 1 or 2\n".encode('utf-8'))
        if line == "User Inputting Room 1 or 2":
            while(roomRecieved == False):
                time.sleep(1)
                print("Looking for Room 1 or 2\n")
                if ser.in_waiting > 0:
                    line = ser.readline().decode('latin-1').rstrip()
                    line_no_number = line.rstrip("1234567890")
                    print("Line" + line+"\n")
                    print("Line no number" + line_no_number+"\n")
                    if line_no_number == "Room Number 1 Is:":
                        split_char = ':'
                        res = line.split(split_char,1)
                        room_1 = res[1]
                        print("Room: " + room_1 + "\n")
                        roomRecieved = True
                        for x in range(0,5):
                            ser.write("Pi stored Room 1\n".encode('utf-8'))
                    elif line_no_number == "Room Number 2 is:":
                        split_char = ':'
                        res = line.split(split_char,1)
                        room_2 = res[1]
                        print(room_2)
                        roomRecieved = True
                        for x in range(0,5):
                            ser.write("Pi stored Room 2\n".encode('utf-8'))
                    elif line_no_number == "Back To Menu":
                        for x in range(0,5):
                            ser.write("Back to Menu Recieved\n".encode('utf-8'))
                        roomRecieved = True
        elif line == "User will confirm Room": #Button 3 on Arduino
            while(CVRoom == False):
                time.sleep(1)
                ser.write("Pi waiting for final confirmation\n".encode('utf-8'))
                print("Waiting for confirmation\n")
                if ser.in_waiting > 0:
                    line = ser.readline().decode('latin-1').rstrip()
                    print(line)
                    if line == "Cell 1 Selected":
                        chosenRoom = room_1
                        for x in range(0,5):
                            ser.write("CV will look for Room 1".encode('utf-8'))
                        CVRoom = True
                        print("Cell 1 Selected\n")
                    elif line == "Cell 2 Selected":
                        chosenRoom = room_2
                        for x in range(0,5):
                            ser.write("CV will look for Room 2".encode('utf-8'))
                        CVRoom = True
                    elif line == "Back to Menu":
                        for x in range(0,50):
                            ser.write("Back to Menu Recieved\n".encode('utf-8'))
                        CVRoom = True
        elif line == "Start Open CV":
            beginCV = True
            for x in range(0,5):
                ser.write("Open CV will start\n".encode('utf-8'))
                    
    #Set variables back to false incase User reinputs information
    roomRecieved = False
    CVRoom = False
'''
                        

while True:
    
    success, img = cap.read()
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    for barcode in decode(gray):
        myData = barcode.data.decode('utf-8')
        if (myDataPrev != myData): #ensures that qr code is only printed once
            print(myData)
            myDataPrev = myData
            
        #if(myData == room_1):
         #   while(1):
          #      ser.write("1\n".encode('utf-8'))
           #     if ser.in_waiting > 0:
            #        line = ser.readline().decode('latin-1').rstrip()
       #Code below produces box around qr code 
        pts = np.array([barcode.polygon], np.int32)
        pts = pts.reshape((-1,1,2))
        cv2.polylines(gray,[pts],True,(0,128,128),5)
        
    cv2.imshow('Result', gray)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
