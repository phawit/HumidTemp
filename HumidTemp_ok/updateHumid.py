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
latitude = [13.6600217, 13.6623712, 13.6624533, 13.6643514]
longtitude = [100.2940463, 100.2963192, 100.3020063, 100.301194]

idx = random.randint(0, 3)

while(1):
    current_time = datetime.datetime.now().strftime("%Y%m%d-%H:%M:%S")
    # idx = random.randint(0, 3)
    print(current_time)
    for idx in range(0,4):
        # Get a database reference to our blog.
        firebase_ref = db.reference('/')
        curr_ref = firebase_ref.child('ID/' + unit_id[idx] + '/Current')
        log_ref = firebase_ref.child('ID/' + unit_id[idx] + '/Log/' + current_time)
        # curr_ref = firebase_ref.child('ID').child(unit_id[idx]).child('Current')
        # log_ref = firebase_ref.child('ID').child(unit_id[idx]).child('Log').child(current_time)

        data = {
                    'DateTime': current_time,
                    'Flag': flag_color[random.randint(0, 4)],
                    'HID': float("{0:.2f}".format(random.uniform(30.0,35.0))),
                    'Humid': float("{0:.2f}".format(random.uniform(40.0,60.0))),
                    'Latitude': latitude[idx],
                    'Longtitude': longtitude[idx],
                    'Temperature': float("{0:.2f}".format(random.uniform(30.0,35.0))),
                    'UnitName': unit_name[idx],
                    'Water': 1
                }

        print(unit_name[idx], data['Flag'])

        curr_ref.update(data)
        log_ref.set(data)

    time.sleep(20)