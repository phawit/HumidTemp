#!/usr/local/bin/python
# -*- coding: utf-8 -*-
# encoding=utf8  
import sys  

reload(sys)  
sys.setdefaultencoding('utf8')

import requests
import pyrebase
import datetime
import time

config = {
  "apiKey": "AIzaSyA8hvIgXiCLMJt0cLKtQI-mwge4WvQbIuo",
  "authDomain": "humidtemp-59706.firebaseapp.com",
  "databaseURL": "https://humidtemp-59706.firebaseio.com",
  "storageBucket": "humidtemp-59706.appspot.com",
  }

firebase = pyrebase.initialize_app(config)
db = firebase.database()

unit_id = ['A01', 'A02', 'A03', 'A04']



once = 0
while(1):
  #check time
  current_time = datetime.datetime.now().strftime("%Y%m%d-%H:%M:%S")
  Hr = int(datetime.datetime.now().strftime("%H"))
  Min = int(datetime.datetime.now().strftime("%M"))
  print(current_time)

  #print(once)
  
  if Min == 8:
    once = 0
  if ((Hr == 8 or Hr == 10 or Hr == 12 or Hr == 14 or Hr == 16 or Hr == 5) and Min == 0) and once==0 :
    print("line notify")

    #get data from firebase
    line1 = []
    line2 = []
    temp = []
    humid = []
    water = []
    train = []
    rest = []
    flag = []
    unit = []
    message = []
    
    for x in unit_id:
      unit.append(db.child("ID/"+x+"/Current/UnitName").get().val())
      line1.append(db.child("ID/"+x+"/Current/Line1").get().val())
      line2.append(db.child("ID/"+x+"/Current/Line2").get().val())
      temp.append(db.child("ID/"+x+"/Current/Temperature").get().val())
      humid.append(db.child("ID/"+x+"/Current/Humid").get().val())
      water.append(db.child("ID/"+x+"/Current/Water").get().val())
      train.append(db.child("ID/"+x+"/Current/Train").get().val())
      rest.append(db.child("ID/"+x+"/Current/Rest").get().val())
      flag.append(db.child("ID/"+x+"/Current/Flag").get().val())

      print(unit)
      print(line1)
      print(line2)
      print(temp)
      print(humid)
      print(water)
      print(flag)

    #line notify
    url = 'https://notify-api.line.me/api/notify'
    for i in range(len(unit_id)):
      
      #line Thai
      if(flag[i]=='Red'):
        flag[i] = '♦️'
      elif(flag[i]=='Yellow'):
        flag[i] = '🔶'
      elif(flag[i]=='Green'):
        flag[i] = '🇸🇦'
      elif(flag[i]=='Black'):
        flag[i] = '🏴'
      else:
        flag[i] = '🏳️'
     
      if(Hr>=6 and Hr <18):
        dayNight = "🔆"
      else: 
        dayNight = "🌜"

      message =  unit[i]+dayNight
      message += "\n⛰Flag: "+flag[i]
      message += "\n⛰Temperature: "+str(temp[i])+" °C"
      message += "\n⛰Humid: "+str(humid[i])+" %"  
      message += "\n⛰Water: "+str(water[i])+" L/hr"
      message += "\n⛰Train: "+str(train[i])+" min"
      message += "\n⛰Rest: "+str(rest[i])+" min"
      message += "\nhttps://humidtemp-59706.firebaseapp.com/"

      

      messageTH =  unit[i]+dayNight
      messageTH += "\n⛰สัญญาณธง: "+flag[i]
      messageTH += "\n⛰อุณหภูมิ: "+str(temp[i])+" °C"
      messageTH += "\n⛰ความซื้นสัมพัทธ์: "+str(humid[i])+" %"  
      messageTH += "\n⛰ควรดื่มน้ำ: "+str(water[i])+" ลิตร/ชม."
      messageTH += "\n⛰แนะนำให้ฝึก: "+str(train[i])+" นาที"
      messageTH += "\n⛰พัก: "+str(rest[i])+" นาที"
      messageTH += "\nhttps://humidtemp-59706.firebaseapp.com/"
          
      token1 = str(line1[i])
      token2 = str(line2[i])
    
      headers = {'content-type':'application/x-www-form-urlencoded','Authorization':'Bearer '+'FGhzDc08PVp90zsNmEej4U2haijC2W9iLckS3sk1rXY'}
      r = requests.post(url, headers=headers , data = {'message':messageTH})

      headers = {'content-type':'application/x-www-form-urlencoded','Authorization':'Bearer '+token1}
      r = requests.post(url, headers=headers , data = {'message':messageTH})
   
      headers = {'content-type':'application/x-www-form-urlencoded','Authorization':'Bearer '+token2}
      r = requests.post(url, headers=headers , data = {'message':messageTH})






      print(r.text)
      once = 1

  time.sleep(10)





 









