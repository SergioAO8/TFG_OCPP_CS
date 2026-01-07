# TFG_OCPP_CS
Sistema de control OCPP en C, amb interfície d'usuari web (Flaks, HTML, CSS y JavaScript).

# A.2. Execució del sistema localment

En cas que es vulgui executar i utilitzar el sistema de control localment, s’han d’instal·lar les llibreries necessàries i fer una petita modificació en el codi del servidor web. En aquesta secció es detalla com es pot fer.

---

## A.2.1. Execució del nucli del sistema

En primer lloc, s’han d’instal·lar les llibreries necessàries. Per facilitar aquesta feina, s’ha creat un paquet de Debian anomenat **`ocpp-cs-1.0-amd64.deb`**.

Abans d’executar aquest paquet, cal executar el fitxer `pre_install.sh` amb la següent comanda:

```bash
./pre_install.sh
```

Seguidament, s’ha d’executar la comanda següent:

```bash
sudo dpkg -i ocpp-cs-1.0-amd64.deb
```

D’aquesta manera, s’instal·len les llibreries necessàries per compilar i executar el nucli del sistema de control.

Per compilar-lo, cal accedir al directori següent:

```bash
nucli_sistema/ocpp_cs/
```

Un cop dins del directori, s’ha d’executar la comanda:

```bash
make
```

Aquesta acció crearà l’executable **`ocpp_cs`**, el qual es pot executar amb:

```bash
./ocpp_cs
```

En executar-lo, es mostrarà al terminal el següent missatge:

```text
Waiting for incoming connections...
```

---

## A.2.2. Execució del servidor web

En primer lloc, cal anar al directori:

```bash
servidor_web/
```

Un cop aquí, és molt important fer un petit canvi en el fitxer **`app.py`**. El fitxer proporcionat correspon a la versió que s’executa a la màquina virtual d’AWS; per tant, si s’executa localment no funciona correctament, ja que utilitza adreces accessibles únicament des de la màquina virtual.

Si aquest canvi no es fa, el servidor web local es connectarà amb el nucli del sistema que s’està executant a la màquina virtual (accessible des d’Internet), fet que podria afectar al seu funcionament.

### Modificació del fitxer `app.py`

Cal comentar la línia **212** del codi i descomentar la línia immediatament posterior. Les línies originals són les següents:

```python
ws_c = websocket.WebSocketApp("wss://ocpp-cs.duckdns.org/ocpp/",
#ws_c = websocket.WebSocketApp("ws://localhost:8080",
```

Per comentar la primera i descomentar la segona, simplement cal afegir el caràcter `#` a l’inici de la primera línia i eliminar-lo de la segona, de manera que el codi quedi així:

```python
#ws_c = websocket.WebSocketApp("wss://ocpp-cs.duckdns.org/ocpp/",
ws_c = websocket.WebSocketApp("ws://localhost:8080",
```

En cas de no trobar aquestes línies fàcilment, al codi hi ha comentaris al costat d’aquestes que ajuden a localitzar-les.

---

### Instal·lació de llibreries i entorn virtual

A continuació, cal instal·lar diverses llibreries de Python. Abans, però, s’ha de crear un entorn virtual dins del directori **`servidor_web`**.

Totes les comandes que cal executar en aquest directori són les següents:

```bash
python3 -m venv .venv
. .venv/bin/activate
pip install flask
pip install eventlet
pip install flask-login
pip install flask-socketio
pip install websocket-client
```

---

### Execució del servidor web

Un cop finalitzada la instal·lació, el servidor web es pot executar amb la comanda:

```bash
python3 app.py
```

És important assegurar-se que **el nucli del sistema està en execució prèviament**, ja que el servidor web s’hi connecta per funcionar correctament.
