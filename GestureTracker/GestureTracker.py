import mediapipe as mp
from mediapipe.tasks import python
from mediapipe.tasks.python import vision
import cv2
import numpy as np
import time
import argparse
import time
import math
from pythonosc import udp_client
#cv2 used for capture video
vid = cv2.VideoCapture(0)
#media pipe options used to get the hand marker data
BaseOptions = mp.tasks.BaseOptions
HandLandmarker = mp.tasks.vision.HandLandmarker
HandLandmarkerOptions = mp.tasks.vision.HandLandmarkerOptions
HandLandmarkerResult = mp.tasks.vision.HandLandmarkerResult
VisionRunningMode = mp.tasks.vision.RunningMode


# would get the landmarker locations and set that as the glocal variable
def resultData(result: HandLandmarkerResult, output_image: mp.Image, timestamp_ms: int):

    try:
        global handData
        handData = result.hand_landmarks

    except:
        handData = []



    #21 landmarks for each joint. 
    
def handVid(client):
    #set up the settings for the landmarkers from media pipe documentation for livestreaming
    options = HandLandmarkerOptions(
        base_options=BaseOptions(model_asset_path='hand_landmarker.task'),
        running_mode=VisionRunningMode.LIVE_STREAM,
        num_hands = 2,
        result_callback=resultData)
    #set up the flag for octave setting or music setting, and octave for each player
    flag = False
    timeFlag = False
    timeFlag2 = False
    player1 = 1
    player2 = 1

 
    #using the options made 
    with HandLandmarker.create_from_options(options) as landmarker:
        while(True):
            #read the vid, flip the frame, convert the color, get the time, and format the image, after doing this take the resulting image detect the landmarker locations
            temp, still = vid.read()
            still = cv2.flip(still, 1)
            newimg = cv2.cvtColor(still, cv2.COLOR_BGR2RGB)
            millisec = int(time.time() * 1000)
            image= mp.Image(image_format=mp.ImageFormat.SRGB, data=newimg)
            landmarker.detect_async(image, millisec )
            #get the dimensions of the frame
            height, width, color_channels = still.shape

            try:
                #if there is landmarker data
                if(handData):
                    #if only one hand
                    if(len(handData) == 1):
                        #find the position of the hand relative to the screen/frame
                        x, y = int(handData[0][0].x*width), int(handData[0][0].y*height) 
                        #create a circle at palm base
                        cv2.circle(still,(x,y), 7, (0, 255, 0), cv2.FILLED)
                        #if the flag is false(octave mode)
                        if flag == False:
                            #if only the pointer finger is up
                            if((handData[0][8].y <handData[0][5].y) and (handData[0][12].y >=handData[0][9].y) and (handData[0][16].y >handData[0][13].y) and (handData[0][20].y >handData[0][17].y)and (handData[0][4].x >=handData[0][2].x)):
                                #let player know and set octave to 2
                                #if 2 seconds pass then register it
                                if(timeFlag == False):
                                    oldtime = time.time()
                                    timeFlag = True
                                elif(timeFlag == True):
                                    if time.time() - oldtime > 2:
                                        print("player 1 - 1 finger")
                                        player1 = 2
                                        timeFlag=False
                            #if pointer and middle finger are up
                            elif((handData[0][8].y <handData[0][5].y) and (handData[0][12].y <handData[0][9].y) and (handData[0][16].y >handData[0][13].y) and (handData[0][20].y >handData[0][17].y)and (handData[0][4].x >=handData[0][2].x)):
                                #let player know and set octave to 3
                                if(timeFlag == False):
                                    oldtime = time.time()
                                    timeFlag = True
                                elif(timeFlag == True):
                                    if time.time() - oldtime > 2:
                                        print("player 1 - 2 finger")
                                        player1 = 3
                                        timeFlag=False
                            #if 3 fingers are up
                            elif((handData[0][8].y <handData[0][5].y) and (handData[0][12].y <handData[0][9].y) and (handData[0][16].y <handData[0][13].y) and (handData[0][20].y >handData[0][17].y)and (handData[0][4].x >=handData[0][2].x)):
                                #let player know and set octave to 4
                                if(timeFlag == False):
                                    oldtime = time.time()
                                    timeFlag = True
                                elif(timeFlag == True):
                                    if time.time() - oldtime > 2:
                                        cv2.putText(still, "Point up", (70,20), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 0, 0), 2)
                                        print("player 1 - 3 finger")
                                        player1 = 4
                                        timeFlag=False
                            #if hand is in fist format
                            elif((handData[0][8].y >handData[0][5].y) and (handData[0][12].y >handData[0][9].y) and (handData[0][16].y >handData[0][13].y) and (handData[0][20].y >handData[0][17].y)and (handData[0][4].x >handData[0][2].x)):
                                #set flag to true and set to music mode
                                if(timeFlag == False):
                                    oldtime = time.time()
                                    timeFlag = True
                                elif(timeFlag == True):
                                    if time.time() - oldtime > 2:
                                        cv2.putText(still, "Point up", (70,20), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 0, 0), 2)
                                        print("fist")

                                        timeFlag=False
                                        flag = True
                            #send player hand data to juce
                            client.send_message("/juce/gestureData", [player1,1,0.0,1.0, 0.0,0.0, 1.0,0.0])
                        #if in music mode
                        elif flag == True:
                            #if hand is in pinching format
                            if((handData[0][8].y <handData[0][5].y) and (handData[0][12].y >=handData[0][9].y) and (handData[0][16].y >handData[0][13].y) and (handData[0][20].y >handData[0][17].y)and (handData[0][4].y <handData[0][2].y)):
                                #take the position of the thumb tip
                                thumbx, thumby = int(handData[0][4].x*width), int(handData[0][4].y*height)
                                #take the position of index finger tip
                                indexx,indexy = int(handData[0][8].x*width), int(handData[0][8].y*height)
                                #find distance using linear interpolation
                                length = math.sqrt(math.pow(indexx-thumbx,2)+math.pow(indexy-thumby,2))
                                distance = np.interp(x=length,xp=[15,250],fp=[0,1])
                                #let the player know 
                                print(player1,handData[0][0].x, handData[0][0].y, distance)
                            #if hand is in palm shape
                            elif((handData[0][8].y <handData[0][5].y) and (handData[0][12].y <handData[0][9].y) and (handData[0][16].y <handData[0][13].y) and (handData[0][20].y <handData[0][17].y)and (handData[0][4].x <handData[0][2].x)):
                                #move to octave mode
                                print("palm")
                                flag=False
                                
                            #send player data to juce
                            client.send_message("/juce/gestureData", [player1,1,handData[0][0].x, handData[0][0].y, distance,0.0, 1.0,0.0])
                    #if there are 2 hands
                    if(len(handData) == 2):
                        #find the palm position of both and mark it with circle
                        x, y = int(handData[0][0].x*width), int(handData[0][0].y*height) 
                        cv2.circle(still,(x,y), 7, (0, 255, 0), cv2.FILLED)
                        x1, y1 = int(handData[1][0].x*width), int(handData[1][0].y*height) 
                        cv2.circle(still,(x1,y1), 7, (255, 0, 0), cv2.FILLED)
                        #if in octave mode
                        if flag == False:
                            # if player one has only pointer finger is up
                            if((handData[0][8].y <handData[0][5].y) and (handData[0][12].y >=handData[0][9].y) and (handData[0][16].y >handData[0][13].y) and (handData[0][20].y >handData[0][17].y)and (handData[0][4].x <=handData[0][2].x)):
                                #let player know and set octave to 2 for player 1
                                #if 2 seconds pass then register it
                                        print("player 1 - 1 finger")
                                        player1 = 2
                                        
                            # if player one has pointer and middle finger is up
                            if((handData[0][8].y <handData[0][5].y) and (handData[0][12].y <handData[0][9].y) and (handData[0][16].y >handData[0][13].y) and (handData[0][20].y >handData[0][17].y)and (handData[0][4].x <=handData[0][2].x)):
                                #let player know and set octave to 3 for player 1
                                        print("player 1 - 2 finger")
                                        player1 = 3
                                        
                            # if player one has 3 fingers is up
                            if((handData[0][8].y <handData[0][5].y) and (handData[0][12].y <handData[0][9].y) and (handData[0][16].y <handData[0][13].y) and (handData[0][20].y >handData[0][17].y)and (handData[0][4].x <=handData[0][2].x)):
                                #let player know and set octave to 3 for player 1
                                        print("player 1 - 3 finger")
                                        player1 = 4
                                        
                            # if player 2 has only pointer finger is up
                            if((handData[1][8].y <handData[1][5].y) and (handData[1][12].y >=handData[1][9].y) and (handData[1][16].y >handData[1][13].y) and (handData[1][20].y >handData[1][17].y)and (handData[1][4].x >=handData[1][2].x)):
                                #let player know and set octave to 2 for player 2
                                        print("player 2 - 1 finger")
                                        player2 = 2
                                        
                            # if player 2 has pointer and middle finger is up
                            if((handData[1][8].y <handData[1][5].y) and (handData[1][12].y <handData[1][9].y) and (handData[1][16].y >handData[1][13].y) and (handData[1][20].y >handData[1][17].y)and (handData[1][4].x >=handData[1][2].x)):
                                #let player know and set octave to 3 for player 2
                                        print("player 2 - 2 finger")
                                        player2 = 3
                                        
                            # if player 2 has pointer middle and ring finger is up
                            if((handData[1][8].y <handData[1][5].y) and (handData[1][12].y <handData[1][9].y) and (handData[1][16].y <handData[1][13].y) and (handData[1][20].y >handData[1][17].y)and (handData[1][4].x >=handData[1][2].x)):
                                #let player know and set octave to 4 for player 2

                                        print("player 2 - 3 finger")
                                        player2 = 4
                                        
                            # if player 2 and player 1 has fists 
                            if((handData[1][8].y >handData[1][5].y) and (handData[1][12].y >handData[1][9].y) and (handData[1][16].y >handData[1][13].y) and (handData[1][20].y >handData[1][17].y)and (handData[1][4].x >handData[1][2].x) and (handData[0][8].y >handData[0][5].y) and (handData[0][12].y >handData[0][9].y) and (handData[0][16].y >handData[0][13].y) and (handData[0][20].y >handData[0][17].y)and (handData[0][4].x <handData[0][2].x)):
                                #go to music mode
                                        print("fist")
                                        flag = True
                            #send data to juce
                            client.send_message("/juce/gestureData", [player1,player2,0.0,1.0, 0.0,0.0, 1.0,0.0])
                        #if in music mode
                        if flag == True:
                            #for player 1 if hand is in pinching form 
                            if((handData[0][8].y <handData[0][5].y) and (handData[0][12].y >=handData[0][9].y) and (handData[0][16].y >handData[0][13].y) and (handData[0][20].y >handData[0][17].y)and (handData[0][4].y <handData[0][2].y)):
                                #get thumb point and index finger point and get linear interpolation between them
                                thumbx, thumby = int(handData[0][4].x*width), int(handData[0][4].y*height)
                                indexx,indexy = int(handData[0][8].x*width), int(handData[0][8].y*height)
                                length = math.sqrt(math.pow(indexx-thumbx,2)+math.pow(indexy-thumby,2))
                                distance = np.interp(x=length,xp=[15,250],fp=[0,1])
                                #let player know
                                print(player1,player2,handData[1][0].x, handData[1][0].y, distance2,handData[0][0].x, handData[0][0].y, distance)
                            #for player 2 if hand is in pinching form    
                            if((handData[1][8].y <handData[1][5].y) and (handData[1][12].y >=handData[1][9].y) and (handData[1][16].y >handData[1][13].y) and (handData[1][20].y >handData[1][17].y)and (handData[1][4].y <handData[1][2].y)):
                                #get thumb point and index finger point and get linear interpolation between them
                                thumbx1, thumby1 = int(handData[1][4].x*width), int(handData[1][4].y*height)
                                indexx1,indexy1 = int(handData[1][8].x*width), int(handData[1][8].y*height)
                                length1 = math.sqrt(math.pow(indexx1-thumbx1,2)+math.pow(indexy1-thumby1,2))
                                distance2 = np.interp(x=length1,xp=[15,250],fp=[0,1])
                                #let player know
                                print(player1,player2,handData[1][0].x, handData[1][0].y, distance2,handData[0][0].x, handData[0][0].y, distance)
                            #if both players have open palms 
                            if((handData[1][8].y <handData[1][5].y) and (handData[1][12].y <handData[1][9].y) and (handData[1][16].y <handData[1][13].y) and (handData[1][20].y <handData[1][17].y)and (handData[1][4].x <handData[1][2].x)and (handData[0][8].y <handData[0][5].y) and (handData[0][12].y <handData[0][9].y) and (handData[0][16].y <handData[0][13].y) and (handData[0][20].y <handData[0][17].y)and (handData[0][4].x >handData[0][2].x)):
                                #move to octave mode and let player know
                                flag=False
                                print("palm")
                            #send data to juce
                            client.send_message("/juce/gestureData", [player1,player2,handData[1][0].x, handData[1][0].y, distance2,handData[0][0].x, handData[0][0].y, distance])
                                        
                                              
            except:
                pass
            #close the camera with q
            cv2.imshow("frame", still)
            if cv2.waitKey(1) == ord('q'):
                    break
    #removes and closes all windows                       
    vid.release()

    cv2.destroyAllWindows()




#connects to juce server and sends visual data to create music
if __name__ == "__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument("--ip", default="127.0.0.1",
      help="The ip of the OSC server")
  parser.add_argument("--port", type=int, default=9001,
      help="The port the OSC server is listening on")
  args = parser.parse_args()

  client = udp_client.SimpleUDPClient(args.ip, args.port)
handVid(client)
