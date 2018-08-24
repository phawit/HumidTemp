var config = {
  apiKey: "AIzaSyA8hvIgXiCLMJt0cLKtQI-mwge4WvQbIuo",
  authDomain: "humidtemp-59706.firebaseapp.com",
  databaseURL: "https://humidtemp-59706.firebaseio.com",
  projectId: "humidtemp-59706",
  storageBucket: "humidtemp-59706.appspot.com",
  messagingSenderId: "1072796099399"
};
firebase.initializeApp(config);
var database = firebase.database();

var iTemp,iHumid;
var imei, unit_curr, unit_new, line1, line2;

function update(){
  

  // Get the value of the input field with id="numb"
  imei = document.getElementById("imei").value;
  unit_curr = document.getElementById("unit_curr").value;
  unit_new = document.getElementById("unit_new").value;
  latitude = document.getElementById("latitude").value;
  longtitude = document.getElementById("longtitude").value;
  SSID_name = document.getElementById("SSID_name").value;
  SSID_pass = document.getElementById("SSID_pass").value;
  temp = document.getElementById("temp").value;
  humid = document.getElementById("humid").value;
  line1 = document.getElementById("line1").value;
  line2 = document.getElementById("line2").value;

  // 019652455487501 A01 Signal101
  var stat = 'Status: Wrong IMEI';
  document.getElementById("status").innerHTML = stat;
  // document.getElementById("status").innerHTML = stat;
  var ref = database.ref('/IMEI');
  ref.once('value',function (snapshot) {
    snapshot.forEach(function(childSnapshot) {       
      var key = childSnapshot.key;
      var val = childSnapshot.val();

      if (imei == key)
      {
        // stat = val;
        stat = 'Status: Wrong Unit Name';
        document.getElementById("status").innerHTML = stat;
        // val = 'A01';
        var db_id = database.ref('/ID').child(val + '/Current');
        db_id.once('value',function (snapshot_id) {
          // stat = 'OK';
          var data = snapshot_id.val();
          // stat = unit_curr + '...' + data['UnitName'];
          if (unit_curr == data['UnitName']){
            data['UnitName'] = unit_new;
            data['Latitude'] = parseFloat(latitude);
            data['Longtitude'] = parseFloat(longtitude);
            data['SSID_name'] = SSID_name;
            data['SSID_pass'] = SSID_pass;
            data['Line1'] = line1;
            data['Line2'] = line2;
            data['Pub'] = 1;
           // data['Temperature'] = temp;
           // data['Humid'] = humid;
            data['iTemp'] = iTemp;
            data['iHumid'] = iHumid;
            console.log(iTemp);
            console.log(iHumid);
            db_id.set(data);           
            stat = 'Status: Update Success';
            document.getElementById("status").innerHTML = stat;
          }
        });
      }
  });
  });
}

function load(){
  var imei, unit_curr, unit_new;

  // Get the value of the input field with id="numb"
  imei = document.getElementById("imei").value;
  unit_curr = document.getElementById("unit_curr").value;
  unit_new = document.getElementById("unit_new").value;

  // 019652455487501 A01 Signal101
  var stat = 'Status: Wrong IMEI';
  document.getElementById("status").innerHTML = stat;
  // document.getElementById("status").innerHTML = stat;
  var ref = database.ref('/IMEI');
  ref.once('value',function (snapshot) {
    snapshot.forEach(function(childSnapshot) {       
      var key = childSnapshot.key;
      var val = childSnapshot.val();

      if (imei == key)
      {
        stat = 'Status: Wrong Unit Name';
        document.getElementById("status").innerHTML = stat;

        var db_id = database.ref('/ID').child(val + '/Current');
        db_id.once('value',function (snapshot_id) {
          var data = snapshot_id.val();
          if (unit_curr == data['UnitName']){
            document.getElementById("unit_new").value = unit_curr;
            document.getElementById("latitude").value =data['Latitude'];
            document.getElementById("longtitude").value = data['Longtitude'];
            document.getElementById("SSID_name").value = data['SSID_name'];
            document.getElementById("SSID_pass").value = data['SSID_pass'];
            document.getElementById("line1").value = data['Line1'];
            document.getElementById("line2").value = data['Line2'];
            document.getElementById("temp").value = data['Temperature'];
            document.getElementById("humid").value = data['Humid'];

            iTemp = data['iTemp'];
            iHumid = data['iHumid'];
            console.log(iTemp);
            console.log(iHumid);
                  
            stat = 'Status: Load Success';
            document.getElementById("status").innerHTML = stat;
          }
        });
      }
  });
  });
}


var it=0,ih=0;
function tempPlus(){
  it++;
  iTemp = it*0.01;
  temp_x = document.getElementById("temp").value;
  document.getElementById("temp").value = (Number.parseFloat(temp_x)+0.01).toFixed(2);
  console.log(iTemp);
}
function tempMinus(){
  it--;
  iTemp = it*0.01;
  temp_x = document.getElementById("temp").value;
  document.getElementById("temp").value = (Number.parseFloat(temp_x)-0.01).toFixed(2);
  console.log(iTemp);
}
function humidPlus(){
  ih++;
  iHumid = ih*0.01;
  humid_x = document.getElementById("humid").value;
  document.getElementById("humid").value = (Number.parseFloat(humid_x)+0.01).toFixed(2);
  console.log(iHumid);
}
function humidMinus(){
  ih--;
  iHumid = ih*0.01;
  humid_x = document.getElementById("humid").value;
  document.getElementById("humid").value = (Number.parseFloat(humid_x)-0.01).toFixed(2);
  console.log(iHumid);
}