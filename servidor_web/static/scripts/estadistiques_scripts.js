/*
Mostra les meter_values de la base de dades a la corresponent taula.
*/
function mostrarDades() {
    fetch("/meter_values")
        .then(response => response.json())
        .then(data => {
            const taula = document.querySelector("table.w3-table-all.dades tbody")
            taula.innerHTML = ""; // esborra el contingut actual
            data.forEach(fila => {
                const row = `
                    <tr>
                        <td>${fila.charger_id}</td>
                        <td>${fila.connector}</td>
                        <td>${fila.transaccio}</td>
                        <td>${fila.hora}</td>
                        <td>${fila.valor}</td>
                        <td>${fila.unit}</td>
                        <td>${fila.measurand}</td>
                        <td>${fila.context}</td>
                    </tr>`;
                taula.innerHTML += row;
            });
        });
}

/*
Mostra els estats de la base de dades a la corresponent taula.
*/
function mostrarEstats() {
    fetch("/estats")
        .then(response => response.json())
        .then(data => {
            const taula = document.querySelector("table.w3-table-all.estats tbody")
            taula.innerHTML = ""; // esborra el contingut actual
            data.forEach(fila => {
                const row = `
                    <tr>
                        <td>${fila.charger_id}</td>
                        <td>${fila.estat}</td>
                        <td>${fila.connector}</td>
                        <td>${fila.hora}</td>
                        <td>${fila.error_code}</td>
                    </tr>`;
                taula.innerHTML += row;
            });
        });
}

/*
Mostra les transaccions de la base de dades a la corresponent taula.
*/
function mostrarTransaccions() {
    fetch("/transaccions")
        .then(response => response.json())
        .then(data => {
            const taula = document.querySelector("table.w3-table-all.transaccions tbody")
            taula.innerHTML = ""; // esborra el contingut actual
            data.forEach(fila => {
                const row = `
                    <tr>
                        <td>${fila.charger_id}</td>
                        <td>${fila.connector}</td>
                        <td>${fila.estat}</td>
                        <td>${fila.hora}</td>
                        <td>${fila.motiu}</td>
                    </tr>`;
                taula.innerHTML += row;
            });
        });
}

/*
Es carreguen les dades en carregar la pàgina.
*/
window.onload = function() {
    mostrarDades();
    mostrarEstats();
    mostrarTransaccions();
}