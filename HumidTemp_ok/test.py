import firebase_admin
from firebase_admin import credentials, db
import datetime
import time
import random

cred = credentials.Certificate('serviceAccountKey.json')
firebase_default_app = firebase_admin.initialize_app(cred, {
    'databaseURL' : 'https://humidtemp-59706.firebaseio.com/'
})

flag_color = ['White', 'Green', 'Yellow', 'Red', 'Black']
unit_id = ['A01', 'A02', 'A03', 'A04']
unit_name = ['Signal101', 'Signal102', 'SignalSchool', 'SBMB']
latitude = [13.659707, 13.662371, 13.662453, 13.664351]
longtitude = [100.292283, 100.296319, 100.302006, 100.30119]

idx = random.randint(0, 3)

while(1):
    current_time = datetime.datetime.now().strftime("%Y%m%d-%H:%M:%S")
    # idx = random.randint(0, 3)
    print(current_time)
    for idx in range(0,4):
        # Get a database reference to our blog.
        firebase_ref = db.reference('/')
        curr_ref = firebase_ref.child('ID/' + 'A01' + '/Current')
        log_ref = firebase_ref.child('ID/' + 'A01' + '/Log/' + current_time)
        # curr_ref = firebase_ref.child('ID').child(unit_id[idx]).child('Current')
        # log_ref = firebase_ref.child('ID').child(unit_id[idx]).child('Log').child(current_time)

        data = {                   
                    'Latitude': latitude[idx],
                    'Longtitude': longtitude[idx],                   
                    'UnitName': unit_name[idx],
                    'Line1': 'dfdfdf',
                    'Line2': 'sdfsdfs',
                    'Pub': 1               
                }

        

        curr_ref.update(data)
        log_ref.set(data)

    time.sleep(300)