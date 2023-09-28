function getSensorRangeValues() {
  setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        var splittedResponse = this.responseText.split(";");
        document.getElementById("sensor-status-temperature").innerText =splittedResponse[0];
        document.getElementById("sensor-status-humidity").innerText = splittedResponse[1];
        document.getElementById("sensor-status-rain").innerText = splittedResponse[2];
      }
    };
    xhttp.open("GET", "/getCustomizeValues", true);
    xhttp.send();
  }, 1000);
}

function getBlockWindowStatus() {
  setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
		let blockWindowElement = document.getElementById("blockWindowInput");
		if(blockWindowElement)
		{
			if(parseInt(this.responseText) == 1)
			{
				blockWindowElement.value = "Unblock";
				blockWindowElement.classList.add("unblock");
				blockWindowElement.classList.remove("block");
			}
			else 
			{
				blockWindowElement.value = "Block";
				blockWindowElement.classList.add("block");
				blockWindowElement.classList.remove("unblock");

			}
		}
      }
    };
    xhttp.open("GET", "/getBlockWindow", true);
    xhttp.send();
  }, 1000);
}

function readTemperatureStatus() {
  setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        var tihElement = document.getElementById("btnTih");
        if(tihElement)
        {
          if (this.responseText != "nan") {
            tihElement.classList.add("active");
          } else {
            tihElement.classList.remove("active");
          }
        }
        const windowTempElement = document.getElementById("wind-temperature");
        if(windowTempElement)
        {
          windowTempElement.innerText = this.responseText
        }
      }
    };
    xhttp.open("GET", "/temperature", true);
    xhttp.send();
  }, 2000);
}

function readHumidityStatus() {
  setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        var tihElement = document.getElementById("btnTih");
        if(tihElement)
        {
          if (this.responseText != "nan") {
            tihElement.classList.add("active");
          } else {
            tihElement.classList.remove("active");
          }
        }
        const windowHumidityElement = document.getElementById("wind-humidity");
        if(windowHumidityElement)
        {
          windowHumidityElement.innerText = this.responseText
        }
      }
    };
    xhttp.open("GET", "/humidity", true);
    xhttp.send();
  }, 2000);
}

function readRainStatus() {
  setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        var tihElement = document.getElementById("btnRain");
        if(tihElement)
        {
          if (parseInt(this.responseText) > 20) {
            tihElement.classList.add("active");
          } else {
            tihElement.classList.remove("active");
          }
        }
        const windowRainElement = document.getElementById("wind-rain");
        if(windowRainElement)
        {
          let rainValue = parseInt(this.responseText);
          if(rainValue > 600 &&  rainValue < 1000)
          {
            windowRainElement.innerText = "Drizzling Rain"
          }
          else if (rainValue> 399 && rainValue <600)
          {
            windowRainElement.innerText = "Moderate  Rain"

          }
          else if (rainValue < 400)
          {
            windowRainElement.innerText = "Heavy  Rain"

          }
          else{
            windowRainElement.innerText = "No Rain"
          }
        }
      }
    };
    xhttp.open("GET", "/rain", true);
    xhttp.send();
  }, 2000);
}

function readPirStatus() {
  setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        const windowPirElement = document.getElementById("wind-motion");
        if(windowPirElement)
        {
          if (parseInt(this.responseText) == 1) {
            windowPirElement.innerText = "Detected";
          }
          else
          {
            windowPirElement.innerText = "Not detected";
          }
        }
      }
    };
    xhttp.open("GET", "/pir", true);
    xhttp.send();
  }, 2000);
}

function readWindowStatus() {
  setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
      if (document.getElementById("imageContainer").getAttribute("style").indexOf("none") > 0) {
        document.getElementById("imageContainer").style.display = "initial";
      }
      if (this.readyState == 4 && this.status == 200) {
        if (parseInt(this.responseText) == 1) {
          document.getElementById("window-o").style.display = "none";
          document.getElementById("window-c").style.display = "block";
        }
        else {
          document.getElementById("window-c").style.display = "none";
          document.getElementById("window-o").style.display = "block";
        }
      }
    };
    xhttp.open("GET", "/windowstate", true);
    xhttp.send();
  }, 500);
}

function toggleReset() {
  window.setTimeout(function () {
    if (confirm("Are you sure you want to reset?")) {
      var xhttp = new XMLHttpRequest();
      xhttp.open("GET", "/toggleReset?state=1", true);
      xhttp.send();
    } else {
      return false;
    }
  }, 5000);
}