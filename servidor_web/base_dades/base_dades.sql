-- Taula d'usuaris
CREATE TABLE IF NOT EXISTS usuaris (
    usuari TEXT PRIMARY KEY NOT NULL,
    contrasenya TEXT NOT NULL 
);

-- Taula de meterValues
CREATE TABLE IF NOT EXISTS meter_values (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    charger_id INT NOT NULL,
    connector INT NOT NULL,
    transaccio INT,
    hora TEXT NOT NULL,
    valor FLOAT NOT NULL,
    unit TEXT,
    measurand TEXT,
    context TEXT
);

-- Trigger per limitar les dades de meterValues a un màxim de 30
CREATE TRIGGER max_meter_values BEFORE INSERT ON meter_values
    BEGIN
        DELETE FROM meter_values WHERE
            hora = (SELECT min(hora) FROM meter_values)
            and (SELECT COUNT(*) FROM meter_values) = 30;
    END;

-- Taula de transaccions
CREATE TABLE IF NOT EXISTS transaccions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    charger_id INT NOT NULL,
    estat TEXT NOT NULL,
    connector INT NOT NULL,
    hora TEXT NOT NULL,
    motiu TEXT NOT NULL
);

-- Trigger per limitar les dades de transaccions a un màxim de 30
CREATE TRIGGER max_transaccions BEFORE INSERT ON transaccions
    BEGIN
        DELETE FROM transaccions WHERE
            hora = (SELECT min(hora) FROM transaccions)
            and (SELECT COUNT(*) FROM transaccions) = 30;
    END;

-- Taula d'estats
CREATE TABLE IF NOT EXISTS estats (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    charger_id INT NOT NULL,
    connector INT NOT NULL,
    estat TEXT NOT NULL,
    hora TEXT NOT NULL,
    error_code TEXT NOT NULL
);

-- Trigger per limitar les dades d'estats a un màxim de 30
CREATE TRIGGER max_estats BEFORE INSERT ON estats
    BEGIN
        DELETE FROM estats WHERE
            hora = (SELECT min(hora) FROM estats)
            and (SELECT COUNT(*) FROM estats) = 30;
    END;

-- Insereix dos usuaris
INSERT INTO usuaris (usuari, contrasenya) VALUES
('sergio','7110eda4d09e062aa5e4a390b0a572ac0d2c0220'),
('usuari','7110eda4d09e062aa5e4a390b0a572ac0d2c0220');