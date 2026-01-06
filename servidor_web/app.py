import eventlet
eventlet.monkey_patch()

import flask
import flask_login
import json
import hashlib
import sqlite3
import threading
import websocket
import ssl
from flask_socketio import SocketIO, emit


app = flask.Flask(__name__)
app.secret_key = "9af86b9ec264854b1a6ffb15d959b8d2bdb6356526d633ea5ab728f4214e7f81" # clau secreta per l'inici de sessió dels usuaris
socketio = SocketIO(app,cors_allowed_origins=["https://ocpp-cs.duckdns.org", "http://localhost:5000"]) # dono permís al domini per accedir a la connexió del socket

ws_c = None

DATABASE = 'base_dades/base_dades.db'

login_manager = flask_login.LoginManager()
login_manager.init_app(app)
login_manager.login_view = 'login'

# variables per guardar els últims missatge rebuts per part del carregador per guardar l'estat al recarregar la pàgina
last_boot_notification_messsage_by_charger = {}
last_message_by_charger = {}

class User(flask_login.UserMixin):
    """
    Classe genèrica User pels logins/logouts.
    """
    pass

@app.route("/")
@flask_login.login_required
def index():
    """
    Retorna el contingut de la pàgina principal.
    """
    return flask.render_template('index.html', pagina_actual='index')

@app.route("/estadistiques")
@flask_login.login_required
def estadistiques():
    """
    Retorna el contingut de la pàgina d'estadístiques.
    """
    return flask.render_template('estadistiques.html', pagina_actual='estadistiques')

def get_db():
    """
    Retorna el contingut de la base de dades.
    """
    db = getattr(flask.g, '_database', None)
    if db is None:
        db = flask.g._database = sqlite3.connect(DATABASE)
        db.row_factory = sqlite3.Row
    return db

@app.teardown_appcontext
def close_connection(exception):
    """
    Tanca la connexió amb la base de dades en tancar el servidor web.
    """
    db = getattr(flask.g, '_database', None)
    if db is not None:
        db.close()

def check_user(usuari: str, password: str):
    """
    Comprova les credencials dels usuaris. Retorna True si són correctes o False en cas contrari.
    """
    hashed_pwd = hashlib.sha1(password.encode()).hexdigest()
    # Obtenim l'usuari amb el nom i el hash de contrassenya donats -> si no existeix, dona "None"

    db = get_db()
    user = db.execute(
        'SELECT * FROM usuaris WHERE usuari = ? AND contrasenya = ?',
        (usuari, hashed_pwd)
    ).fetchone()

    return user is not None

@login_manager.user_loader
def load_user(user_id):
    """
    Carrega la informació de l'usuari.
    """
    user = User()
    user.id = user_id
    return user

@app.route('/login', methods=['GET', 'POST'])
def login():
    """
    Amb el mètode GET retorna el contingut de la pàgina de login.
    Amb el mètode PUT inicia la sessió de l'usuari amb les credencials proporcionades, verficant-les prèviament.
    Si les credencials són incorrectes es retorna la pàgina de login.
    Si l'usuari ja s'ha autentificat, es redirigeix a la pàgina principal.
    """
    if not flask_login.current_user.is_anonymous:
        return flask.redirect("/")

    if flask.request.method == 'POST':
        usuari = flask.request.form['username']
        password = flask.request.form['pwd']
        if check_user(usuari, password):
            user = User()
            user.id = usuari
            flask_login.login_user(user)
            return flask.redirect("/")
        else:
            return flask.render_template("login.html")

    return flask.render_template("login.html")

@app.route('/logout', methods=['GET'])
@flask_login.login_required
def logout():
    """
    Tanca la sessió de l'usuari.
    """
    flask_login.logout_user()

    return flask.redirect("/login")

@app.route("/meter_values")
@flask_login.login_required
def meter_values():
    """
    Retorna les dades de la taula meter_values en format JSON.
    """
    db = get_db()
    cursor = db.execute("SELECT * FROM meter_values ORDER BY hora DESC")
    dades = [dict(row) for row in cursor.fetchall()]
    return flask.jsonify(dades)

@app.route("/estats")
@flask_login.login_required
def estats():
    """
    Retorna les dades de la taula estats en format JSON.
    """
    db = get_db()
    cursor = db.execute("SELECT * FROM estats ORDER BY hora DESC")
    dades = [dict(row) for row in cursor.fetchall()]
    return flask.jsonify(dades)

@app.route("/transaccions")
@flask_login.login_required
def transaccions():
    """
    Retorna les dades de la transaccions meter_values en format JSON.
    """
    db = get_db()
    cursor = db.execute("SELECT * FROM transaccions ORDER BY hora DESC")
    dades = [dict(row) for row in cursor.fetchall()]
    return flask.jsonify(dades)

# ho executa un thread que es connecta al servidor WS del sistema de control
def websocket_listener():
    """
    Funcions per la comunicació amb el servidor WS del nucli del sistema de control.
    """
    global ws_c

    def on_message(ws, message):
        """
        S'executa en rebre un missatge.
        """
        try:
            data = json.loads(message)
            print("[ws] Rebut:", data)

            charger_id = data.get("charger")
            message_type = data.get("type")
            if message_type == "bootNotification":
                last_boot_notification_messsage_by_charger[charger_id] = data
            else:
                last_message_by_charger[charger_id] = data

            socketio.emit('dades_actualitzades', data)  # Envia a tots els clients web
        except Exception as e:
            print("Error parsejant:", e)

    def on_error(ws, error):
        """
        S'executa en cas d'error.
        """
        print("WebSocket error:", error)

    def on_close(ws, close_status_code, close_msg):
        """
        S'executa en tancar la connexió.
        """
        print("WebSocket tancat:", close_msg)

    def on_open(ws):
        """
        S'executa en obrir la connexió.
        """
        print("WebSocket connectat al sistema en C")
        ws.send("Flask client")

    # Es connecta a l'adreça del nucli del sistema de control.
    ssl_context = ssl.create_default_context()
    ssl_context.check_hostname = False
    ssl_context.verify_mode = ssl.CERT_NONE
    ws_c = websocket.WebSocketApp("wss://ocpp-cs.duckdns.org/ocpp/", # Comentar aquesta linea per provar amb local
    #ws_c = websocket.WebSocketApp("ws://localhost:8080",              #Descomentar aquesta linea per provar amb local
                                on_open=on_open,
                                on_message=on_message,
                                on_error=on_error,
                                on_close=on_close)
    ws_c.run_forever(ping_interval=30, ping_timeout=10)

@socketio.on("connect")
def on_connect():
    """
    S'executa quan un client web es connecta.
    """
    print("Usuari connectat:", flask.request.sid)
    for charger_data in last_boot_notification_messsage_by_charger.values():
        socketio.emit('dades_actualitzades', charger_data, to=flask.request.sid)
    for charger_data in last_message_by_charger.values():
        socketio.emit('dades_actualitzades', charger_data, to=flask.request.sid)

@socketio.on("formulari_operacio")
def rebre_operacio_des_de_client(data):
    """
    S'executa quan un client web envia una petició.
    """
    try:
        print("Enviant al sistema en C:", data)

        if ws_c and ws_c.sock and ws_c.sock.connected:
            ws_c.send(data)
        else:
            print("WebSocket al sistema C no connectat.")
    except Exception as e:
        print("Error processant operació del formulari:", e)


if __name__ == '__main__':
    # Inicia el thread només un cop (quan Flask arrenca)
    threading.Thread(target=websocket_listener, daemon=True).start()
    socketio.run(app, host="0.0.0.0", port=5000)
