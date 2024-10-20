// Ramiro Iván Ríos 2018-2024
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>

<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
.borde {
  border: 3px solid black;
  border-radius: 10px;
  display: inline-block;
}

.switch {
  position: relative;
  display: inline-block;
  width: 60px;
  height: 34px;
}

.switch input {display:none;}

.slider {
  position: absolute;
  cursor: pointer;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: #ccc;
  -webkit-transition: .4s;
  transition: .4s;
}

.slider:before {
  position: absolute;
  content: "";
  height: 26px;
  width: 26px;
  left: 4px;
  bottom: 4px;
  background-color: white;
  -webkit-transition: .4s;
  transition: .4s;
}

input:checked + .slider {
  background-color: #2196F3;
}

input:focus + .slider {
  box-shadow: 0 0 1px #2196F3;
}

input:checked + .slider:before {
  -webkit-transform: translateX(26px);
  -ms-transform: translateX(26px);
  transform: translateX(26px);
}

/* Rounded sliders */
.slider.round {
  border-radius: 34px;
}

.slider.round:before {
  border-radius: 50%;
}
</style>
</head>
<body>
<BR> <b> Salida 1 </b>
<label class="switch">
   <input type="checkbox" id="digital_1" onclick="switchOutput(1)">
   <span class="slider"></span>
</label>

<BR> <b> Salida 2 </b>
<label class="switch">
   <input type="checkbox" id="digital_2" onclick="switchOutput(2)">
   <span class="slider"></span>
</label>
<BR>
<div class="borde">
	<b>Salida controlada por horario NTP</b>
  <BR>
	Hora actual: <span id="HORA_ACTUAL"> </span><BR>
  <input type="checkbox" id="set_horas" onclick="setAutoTime()"> Activar <BR>
	<div class="borde">
		
		Encender en hora(s):
		<input type="text" id="hora_on">    
    <input type="text" id="minutos_on">
	</div>
  <BR>
	<div class="borde">
		Apagar en hora(s):
		<input type="text" id="hora_off">		
		<input type="text" id="minutos_off">
			
	</div>
	<label class="switch">
	   <input type="checkbox" id="digital_3" onclick="switchOutput(3)">
	   <span class="slider"></span>
	</label>
</div>

<BR>

<div class="borde">
  Temperatura: <span id="TEMPValue">0</span>
  <BR>
  Encender <select id="limite_temp" onchange="setAutoTemp()">
    <option value="HIGH">Cuando temp. menor a</option>
    <option value="LOW">Cuando temp. mayor a</option>
  </select>
  <input type="text" id="temperatura" text="20" onchange="setAutoTemp()">
  <input type="checkbox" id="check_auto_temp" onclick="setAutoTemp()">
  <BR>
  <b> Estufa/ventilador </b>
  <label class="switch">
     <input type="checkbox" id="digital_4" onclick="switchOutput(4)">
     <span class="slider"></span>
  </label>

</div>

<BR><BR>

<div id="digital_todo" class="borde">
<b>TODOS </b>
  <button type="button" onclick="setAll(0)">ON</button>
  <button type="button" onclick="setAll(1)">OFF</button>
</div>

<BR>

<br>

<div id="informacion">
  
</div>

<script>
function setAutoTemp(){
  var xhttp = new XMLHttpRequest();

  var limite = document.getElementById("limite_temp").selectedIndex;
  var temperatura = document.getElementById("temperatura").value;
  var check_auto = document.getElementById("check_auto_temp").checked;
  
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("informacion").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "setAutoTemp?LIMITE=" + limite +"&TEMPERATURA="+temperatura +"&CHECK_AUTO=" + check_auto, true);
  xhttp.send();
}

function setAutoTime(){
  var xhttp = new XMLHttpRequest();

  var hora_on = document.getElementById("hora_on").value;
  var minutos_on = document.getElementById("minutos_on").value;
  var hora_off = document.getElementById("hora_off").value;
  var minutos_off = document.getElementById("minutos_off").value;
  
  var set_horas = document.getElementById("set_horas").checked;
  
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("informacion").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "setAutoHorario?HORA_ON=" + hora_on +"&HORA_OFF=" + hora_off +"&MINUTOS_ON=" + minutos_on + "&MINUTOS_OFF=" + minutos_off + "&CHECK_HORARIO=" + set_horas, true);
  xhttp.send();
}

function setOutput(led, value) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("informacion").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "setState?OUTPUT=" + led +"&STATE="+value, true);
  xhttp.send();
}

function setAll(value) {
  for (var cont =1; cont <=4; cont++)
    setOutput(cont, value);
  UpdateOutputs();
}

function switchOutput(what_output){
  if(document.getElementById("digital_" + what_output.toString()).checked == true)
    setOutput(what_output, 0);
  else
    setOutput(what_output, 1);
}

setInterval(function() {
  // Llamar a una función para actualizar...
  getData();
  Update();
  
}, 2500); //... y actualizar cada 2,5 segundos

function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("TEMPValue").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "readTEMP", true);
  xhttp.send();
}

function UpdateOutputs(){
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {

      var outputs = this.responseText.split(";");
      for( var count=1; count <= 4; count++){
        var digital_checked = document.getElementById("digital_" + count.toString());
        if (outputs[count-1] == "1")
          digital_checked.checked = false;
        else
          digital_checked.checked = true;
      }
    }
  };
  
  xhttp.open("GET", "getOutputs", true);
  xhttp.send();
}

function UpdateTime(){
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var hora_actual = document.getElementById("HORA_ACTUAL");

      hora_actual.innerHTML = this.responseText;
    }
  }

  xhttp.open("GET", "getHora", true);
  xhttp.send();
}

function UpdateControls(){
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      let hora_auto = document.getElementById("set_horas");
	  let hora_on = document.getElementById("hora_on").value;
	  let minutos_on = document.getElementById("minutos_on").value;
	  let hora_off = document.getElementById("hora_off").value;
	  var minutos_off = document.getElementById("minutos_off").value;
	  var limite_temp = document.getElementById("limite_temp");
	  var valor_temp = document.getElementById("temperatura").value;
	  var temp_auto = document.getElementById("check_auto_temp");
		console.log(this.responseText);
      var result = this.responseText.split(";");
	  
	  if (result[0] == "0")
		hora_auto.checked = false;
	  if (result[0] == "1")
		hora_auto.checked = true;
	  hora_on = result[1];
	  minutos_on = result[2];
	  hora_off = result[3];
	  minutos_off = result[4];
	  if (result[5] == "0"){
		limite_temp.selectedIndex = 0;
	  }else{
		limite_temp.selectedIndex = 0;
	  }
	  valor_temp = result[6];
	  temp_auto.checked = result[7];
    }
  }

  xhttp.open("GET", "getControls", true);
  xhttp.send();
}

function Update(){
	UpdateOutputs();
	UpdateTime();
	//UpdateControls();
}

</script>
</body></html>
)=====";
