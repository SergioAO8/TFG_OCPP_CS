/*
Mostra la infromació del carregador corresponent en clicar-lo a la barra de navegació dels carregadors.
*/
function openCharger(idCharger) {
  var i;
  var x = document.getElementsByClassName("charger");
    for (i = 0; i < x.length; i++) {
        x[i].style.display = "none";
    }
    document.getElementById(idCharger).style.display = "block";
    var buttons = document.querySelectorAll(".w3-bar .w3-button");
    buttons.forEach(button => button.classList.remove("active"));
    event.currentTarget.classList.add("active");
}

document.getElementById("default-tab").addEventListener("click", function() {
    openCharger('charger0');
})
document.getElementById("ch1").addEventListener("click", function() {
    openCharger('charger1');
})
document.getElementById("ch2").addEventListener("click", function() {
    openCharger('charger2');
});

document.getElementById("ch3").addEventListener("click", function() {
    openCharger('charger3');
});

// Adreça del servidor WS
const socket = io();

/*
Configura la comunicació WS amb el nucli del sistema.
*/
socket.on('dades_actualitzades', function(data) {
  try {
    charger_id = data.charger;
    charger = (parseInt(charger_id, 10) - 1).toString();
    conn_id_1 = (parseInt(charger, 10) * 2 + 1).toString();
    conn_id_2 = (parseInt(charger, 10) * 2 + 2).toString();
    if (data.type === "bootNotification") {
      switch (data.general) {
        case 0:
            document.getElementById("estatGeneral" + charger).textContent = "ESTAT DEL CARREGADOR " + charger_id + ": Connectat";
            break;
        case 1:
            document.getElementById("estatGeneral" + charger).textContent = "ESTAT DEL CARREGADOR " + charger_id + ": Pendent";
            break;
        case 2:
            document.getElementById("estatGeneral" + charger).textContent = "ESTAT DEL CARREGADOR " + charger_id + ": No connectat";
            break;
        default:
            document.getElementById("estatGeneral" + charger).textContent = "ESTAT DEL CARREGADOR " + charger_id + ": Desconegut";
      }
      
      document.getElementById("vendor" + charger).textContent = "chargePointVendor: " + (data.vendor !== "" ? data.vendor: "Desconegut");
      document.getElementById("model" + charger).textContent = "chargePointModel: " + (data.model !== "" ? data.model: "Desconegut");

    } else {
      
      const conn = {
        0: {alt: "disponible", src: "static/imatges/disponible.png"},
        1: {alt: "carregant", src: "static/imatges/charging.png"},
        2: {alt: "fallada", src: "static/imatges/fallada.png"},
        3: {alt: "finishing", src: "static/imatges/finishing.png"},
        4: {alt: "preparing", src: "static/imatges/preparing.png"},
        5: {alt: "reserved", src: "static/imatges/reserved.png"},
        6: {alt: "suspended_ev", src: "static/imatges/suspended_ev.png"},
        7: {alt: "suspended_evse", src: "static/imatges/suspended_evse.png"},
        8: {alt: "unavailable", src: "static/imatges/no_disponible.png"},
        9: {alt: "unknown", src: "static/imatges/unknown.png"},
      }
      const conn1 = conn[data.connector1];
      const conn2 = conn[data.connector2];
      document.getElementById("connector" + conn_id_1).alt = conn1.alt;
      document.getElementById("connector" + conn_id_1).src = conn1.src;
      document.getElementById("connector" + conn_id_2).alt = conn2.alt;
      document.getElementById("connector" + conn_id_2).src = conn2.src;
      document.getElementById("connector" + conn_id_1 + "Mobile").alt = conn1.alt;
      document.getElementById("connector" + conn_id_1 + "Mobile").src = conn1.src;
      document.getElementById("connector" + conn_id_2 + "Mobile").alt = conn2.alt;
      document.getElementById("connector" + conn_id_2 + "Mobile").src = conn2.src;
      if (data.idTag1 === "no_charging") {
          document.getElementById("idTag" + conn_id_1).textContent = "idTag: (carregador sense cap transacció)";
          document.getElementById("idTagMobile" + conn_id_1).textContent = "idTag: (carregador sense cap transacció)";
      } else {
          document.getElementById("idTag" + conn_id_1).textContent = "idTag: " + (data.idTag1);
          document.getElementById("idTagMobile" + conn_id_1).textContent = "idTag: " + (data.idTag1);
      }
      
      if (data.idTag2 === "no_charging") {
          document.getElementById("idTag" + conn_id_2).textContent = "idTag: (carregador sense cap transacció)";
          document.getElementById("idTagMobile" + conn_id_2).textContent = "idTag: (carregador sense cap transacció)";
      } else {
          document.getElementById("idTag" + conn_id_2).textContent = "idTag: " + (data.idTag2);
          document.getElementById("idTagMobile" + conn_id_2).textContent = "idTag: " + (data.idTag2);          
      }
      
      if (data.transactionId1 === -1) {
          document.getElementById("transactionId" + conn_id_1).textContent = "transactionId: (carregador sense cap transacció)";
          document.getElementById("transactionIdMobile" + conn_id_1).textContent = "transactionId: (carregador sense cap transacció)";
      } else {
          document.getElementById("transactionId" + conn_id_1).textContent = "transactionId: " + (data.transactionId1);
          document.getElementById("transactionIdMobile" + conn_id_1).textContent = "transactionId: " + (data.transactionId1);
      }
      
      if (data.transactionId2 === -1) {
          document.getElementById("transactionId" + conn_id_2).textContent = "transactionId: (carregador sense cap transacció)";
          document.getElementById("transactionIdMobile" + conn_id_2).textContent = "transactionId: (carregador sense cap transacció)";
      } else {
          document.getElementById("transactionId" + conn_id_2).textContent = "transactionId: " + (data.transactionId2);
          document.getElementById("transactionIdMobile" + conn_id_2).textContent = "transactionId: " + (data.transactionId2);
      }
    }
    
  } catch (err) {
    console.log("Missatge rebut");
  }
});

/*
Envia un missatge al nucli del sistema quan un usuari envia una petició.
*/
document.addEventListener("DOMContentLoaded", () => {
  const formularios = document.querySelectorAll(".ws-form");

  formularios.forEach((form) => {
    form.addEventListener("submit", function (event) {
      event.preventDefault();

      const operacion = form.dataset.operation || "Desconeguda";
      const formData = new FormData(form);
      const payload = { };
      const objectesEnters = ["connectorId", "transactionId"];

      let chargerId = "0"; // valor per defecte
      
      selectedKeys = [];
      
      for (const [key, value] of formData.entries()) {
        if (key === "charger") {
          chargerId = value;
          continue;
        }
        
        if (form.dataset.operation === "getConfiguration") {
          selectedKeys.push(value);
        } else if (objectesEnters.includes(key)) {
          payload[key] = parseInt(value, 10);
        } else {
        payload[key] = value;
        }
      }

      if (form.dataset.operation === "getConfiguration") {
        payload["key"] = selectedKeys;
      }

      chargerId = parseInt(chargerId) + 1;

      if (socket && socket.connected) {
        socket.emit("formulari_operacio", "Flask:charger" + chargerId + ":" + operacion + ":" + JSON.stringify(payload));
      } else {
        console.error("WebSocket no disponible");
      }
    });
  });
});

/*
Mostra el carregador 1 per defecte en carregar la pàgina.
*/
window.onload = function() {
    document.getElementById("default-tab").click();
}