import firebase_admin
from firebase_admin import credentials, db
import datetime
import time
import random

cred = credentials.Certificate('serviceAccountKey.json')
firebase_default_app = firebase_admin.initialize_app(cred, {
    'databaseURL' : 'https://humidtemp-59706.firebaseio.com/'
})

# ref = db.reference('ID')
# snapshot = ref.order_by_child().get()
# for key, val in snapshot.items():
#     print('{0} was {1} meters tall'.format(key, val))

# root = db.reference()
# # Add a new user under /users.
# new_user = root.child('users').push({
#     'name' : 'Mary Anning', 
#     'since' : 1700
# })

# # Update a child attribute of the new user.
# new_user.update({'since' : 1799})

# # Obtain a new reference to the user, and retrieve child data.
# # Result will be made available as a Python dict.
unit = db.reference('ID').get()
print unit
line = db.reference('ID/A01/Current/Line1').get()
print line

