#! /usr/bin/python

#import libraries
import os, time
import pygame
import sys, random

#accepting type of song as a commandline argument
song_type = sys.argv[1]

mp3s = []

#setting all the parameters value
freq = 48000
bitsize = -16
channel = 2
buff_size = 1024

#Creating a playlist and shuffle the song
def create_playlist(mediapath):
    for path, directory, element in os.walk(mediapath, topdown = False):
        temparray = element
        for i in range(0, len(temparray)):
            if(temparray[i][-3:] == "mp3" and temparray[i][:1] != "."):
                mp3s.append(temparray[i])
            else:
                print("Unusable : ",temparray[i])

    random.shuffle(mp3s)

#Playing the song 
def play(mp3s):
    start_time = time.time()
    pygame.mixer.init(freq, bitsize, channel, buff_size)
    pygame.mixer.music.load(mediapath + mp3s.pop())
    
    flag = 0
    while(int(time.time() - start_time) < 30):
        if(flag == 0):
            pygame.mixer.music.play(1, 0)
            flag = 1
    return flag
        
#Main function        
if __name__ == "__main__":
    if(song_type == "rock"):
        mediapath = "../songs/rock/" 
        create_playlist(mediapath)
        play(mp3s)
    elif(song_type == "pop"):
        mediapath = "../songs/pop/" 
        create_playlist(mediapath)
        play(mp3s)
    elif(song_type == "hiphop"):
        mediapath = "../songs/hiphop/" 
        create_playlist(mediapath)
        play(mp3s)
    elif(song_type == "edm"):
        mediapath = "../songs/edm/" 
        create_playlist(mediapath)
        play(mp3s)

