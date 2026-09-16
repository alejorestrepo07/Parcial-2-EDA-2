/**
 * Archivo: AnalizadorSQL.cpp
 * Propósito: Implementación del Analizador Léxico y Sintáctico rudimentario. 
 *            Este archivo procesa los strings ingresados por el usuario,
 *            extrae los tokens (como el ID o los datos a guardar) y llama a 
 *            las funciones del árbol (ArbolBPlus.h).
 */
#include "AnalizadorSQL.h"

// Inicializa el analizador acoplando el puntero del árbol B+ que se pasó en el main
AnalizadorSQL::AnalizadorSQL(ArbolBPlus* base_datos) : bd(base_datos) {}

string AnalizadorSQL::aMayusculas(string cadena) {
    string upper_str = "";
    for (char c : cadena) upper_str += toupper(c);
    return upper_str;
}

void AnalizadorSQL::ejecutarConsulta(string consulta) {
    stringstream string_stream(consulta);
    string command_word;

    if (consulta.empty()) return;
    
    // Extrae la primera palabra (Ej. "INSERT", "SELECT")
    string_stream >> command_word;
    command_word = aMayusculas(command_word); // Normalizamos a mayúsculas para las comparaciones

    // Derivación según familia de instrucciones SQL
    if (command_word == "CREATE" || command_word == "DROP") {
        analizarDDL(consulta, command_word);
    } else if (command_word == "SELECT" || command_word == "INSERT" || command_word == "DELETE") {
        analizarDQL_DML(consulta, command_word);
    } else if (command_word == "HELP") {
        mostrarAyuda();
    } else {
        cout << "Error: Comando SQL no reconocido. Escriba HELP para mas informacion.\n";
    }
}

void AnalizadorSQL::analizarDDL(string consulta, string comando) {
    // [A IMPLEMENTAR EN EL PARCIAL]:
    // Ejemplo: CREATE TABLE usuarios (id INT, nombre STR);
    
    if (comando == "CREATE") {
        if (consulta.find("INDEX") != string::npos) {
            // [A IMPLEMENTAR EN EL PARCIAL]: Lógica para índices secundarios
            // Ejemplo: CREATE INDEX idx_nombre ON usuarios (nombre);
            // Esto implicaría instanciar un SEGUNDO Árbol B+ donde la clave sea el "nombre" 
            // y el valor sea el ID principal.
            cout << "[Ejecutando DDL] -> Analizando creación de ÍNDICE secundario...\n";
        } else {
            // En una BBDD real, aquí se crearía el esquema de la tabla o se reservaría memoria en disco
            cout << "[Ejecutando DDL] -> Analizando creación de TABLA...\n";
        }
    } else if (comando == "DROP") {
        // Al eliminar una tabla, se debe limpiar el archivo en disco y limpiar los nodos RAM
        cout << "[Ejecutando DDL] -> Analizando eliminación de tabla...\n";
    }
}

void AnalizadorSQL::analizarDQL_DML(string consulta, string comando) {
    // [A IMPLEMENTAR EN EL PARCIAL]:
    // Aquí el estudiante debe decidir cómo extraer los datos (id y contenido)
    // del string `consulta`. Puede usar manipulacion de strings (`find`, `substr`) o Expresiones Regulares (regex).
    
    int record_id;
    size_t pos_open_paren;
    size_t pos_comma;
    size_t pos_first_quote;
    size_t pos_last_quote;
    size_t pos_equals;
    string record_data;
    string search_result;
    vector<Registro> all_records;
    size_t i;

    if (comando == "INSERT") {
        // Ejemplo esperado del usuario: INSERT INTO tabla VALUES (10, 'Ejemplo Dato')
        // 1. Extraer el '10' como entero.
        // 2. Extraer el contenido entre las comillas simples "'Ejemplo Dato'" como string.
        // 3. Ejecutar: bd->insertar(10, "'Ejemplo Dato'");
        
        pos_open_paren = consulta.find("(");
        pos_comma = consulta.find(",");
        pos_first_quote = consulta.find("'", pos_comma);
        pos_last_quote = consulta.rfind("'");

        if (pos_open_paren != string::npos && pos_comma != string::npos && pos_first_quote != string::npos && pos_last_quote != string::npos) {
            record_id = stoi(consulta.substr(pos_open_paren + 1, pos_comma - pos_open_paren - 1));
            record_data = consulta.substr(pos_first_quote + 1, pos_last_quote - pos_first_quote - 1);
            bd->insertar(record_id, record_data);
            cout << "Registro insertado con exito.\n";
        }
    } 
    else if (comando == "SELECT") {
        // Ejemplo esperado: SELECT * FROM tabla; o SELECT * FROM tabla WHERE id = 10;
        
        if (consulta.find("WHERE") != string::npos) {
            // Si la consulta contiene la palabra WHERE, significa que es búsqueda por ID.
            // 1. Extraer el ID (el número que está después del "=").
            // 2. Ejecutar: string resultado = bd->buscar(id);
            // 3. Imprimir el resultado en pantalla para que el usuario lo vea.
            
            pos_equals = consulta.find("=");
            if (pos_equals != string::npos) {
                record_id = stoi(consulta.substr(pos_equals + 1));
                search_result = bd->buscar(record_id);
                if (!search_result.empty()) {
                    cout << "ID: " << record_id << " - Dato: " << search_result << "\n";
                } else {
                    cout << "Registro no encontrado.\n";
                }
            }
        } else {
            // Si no tiene WHERE, es un barrido general de toda la base de datos (SELECT * FROM tabla).
            // 1. Ejecutar: vector<Registro> resultados = bd->obtenerTodos();
            // 2. Iterar y pintar todos los registros recuperados de las hojas del árbol.
            
            all_records = bd->obtenerTodos();
            for (i = 0; i < all_records.size(); i++) {
                cout << "ID: " << all_records[i].clave << " - Dato: " << all_records[i].datos << "\n";
            }
        }
    }
    else if (comando == "DELETE") {
        // Ejemplo esperado: DELETE FROM tabla WHERE id = 10;
        // 1. Extraer el ID después del "WHERE id = "
        // 2. Ejecutar: bd->eliminar(id);
        
        pos_equals = consulta.find("=");
        if (pos_equals != string::npos) {
            record_id = stoi(consulta.substr(pos_equals + 1));
            bd->eliminar(record_id);
            cout << "Registro eliminado con exito.\n";
        }
    }
}

void AnalizadorSQL::mostrarAyuda() {
    // Configuración de colores estándar ANSI para terminales modernas
    const string RESET = "\033[0m";
    const string BOLD_YELLOW = "\033[1;33m";
    const string BOLD_CYAN = "\033[1;36m";
    const string BOLD_GREEN = "\033[1;32m";
    const string BOLD_WHITE = "\033[1;37m";

    cout << BOLD_YELLOW << "\n=== Sistema Gestor SQL basado en Árboles B+ ===" << RESET << "\n";
    cout << BOLD_WHITE << "Comandos Soportados (Esqueleto):" << RESET << "\n";
    
    // Categoría DDL
    cout << BOLD_YELLOW << "  [DDL - Lenguaje de Definición de Datos]" << RESET << "\n";
    cout << BOLD_CYAN << "    Sintaxis: CREATE TABLE <nombre> (columnas...)" << RESET << "\n";
    cout << BOLD_GREEN << "    Ejemplo : CREATE TABLE usuarios (id INT, nombre STR)" << RESET << "\n\n";
    
    cout << BOLD_CYAN << "    Sintaxis: CREATE INDEX <nombre> ON <tabla> (columna)" << RESET << "\n";
    cout << BOLD_GREEN << "    Ejemplo : CREATE INDEX idx_nombre ON usuarios (nombre)" << RESET << "\n\n";
    
    cout << BOLD_CYAN << "    Sintaxis: DROP TABLE <nombre>" << RESET << "\n";
    cout << BOLD_GREEN << "    Ejemplo : DROP TABLE usuarios" << RESET << "\n\n";

    // Categoría DML/DQL
    cout << BOLD_YELLOW << "  [DQL / DML - Manipulación y Consulta]" << RESET << "\n";
    cout << BOLD_CYAN << "    Sintaxis: INSERT INTO <nombre> VALUES (<id>, <datos>)" << RESET << "\n";
    cout << BOLD_GREEN << "    Ejemplo : INSERT INTO usuarios VALUES (10, 'Juan Perez')" << RESET << "\n\n";
    
    cout << BOLD_CYAN << "    Sintaxis: SELECT * FROM <nombre>" << RESET << "\n";
    cout << BOLD_GREEN << "    Ejemplo : SELECT * FROM usuarios" << RESET << "\n\n";
    
    cout << BOLD_CYAN << "    Sintaxis: SELECT * FROM <nombre> WHERE id = <id>" << RESET << "\n";
    cout << BOLD_GREEN << "    Ejemplo : SELECT * FROM usuarios WHERE id = 10" << RESET << "\n\n";
    
    cout << BOLD_CYAN << "    Sintaxis: DELETE FROM <nombre> WHERE id = <id>" << RESET << "\n";
    cout << BOLD_GREEN << "    Ejemplo : DELETE FROM usuarios WHERE id = 10" << RESET << "\n\n";
    
    // Controles Base
    cout << BOLD_YELLOW << "  [Otros Comandos]" << RESET << "\n";
    cout << BOLD_CYAN << "    HELP  - Muestra este menu" << RESET << "\n";
    cout << BOLD_CYAN << "    EXIT  - Guarda los datos y sale del programa" << RESET << "\n";
    cout << BOLD_YELLOW << "================================================" << RESET << "\n\n";
}